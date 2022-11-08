mod data_access_layer;
mod processes;
mod collections;

use data_access_layer::occurrence::OccurrenceData;
use data_access_layer::summary::{SummaryData};
use processes::{calculate_standard_deviation, calculate_st_deviation_two};
use collections::SummaryStatistics;

use std::collections::HashMap;


fn add_two_vectors(one: &mut Vec<f32>, two: &Vec<f32>) {
    for i in 0..one.len() {
        one[i as usize] += two[i as usize];
    }
}


#[tokio::main]
async fn main() {
    // get data around the occurrences
    let mut occ_data = OccurrenceData::new(String::from("./input/occurrence.bin")).await;
    let raw_data = occ_data.get_data().await;
    let occurrences = raw_data;
    let number_of_periods = occ_data.period_number;

    // define map for summary statistics
    let mut summary_map: HashMap<i32, SummaryStatistics> = HashMap::new();

    let files = [
        String::from("./work/summary_aal/summary_1.bin"), 
        String::from("./work/summary_aal/summary_2.bin")
        ];

    // run the processes in parallel
    let _ = files.iter().map(|i| {
        let mut sum_data = SummaryData::new(i.clone());
        let vec_capacity = sum_data.no_of_samples;
        let summaries = sum_data.get_data(&occurrences, vec_capacity);

        for summary in summaries {
            let occurrences_vec = &occurrences.get(&summary.event_id).unwrap();

            let mut summary_statistics: &mut SummaryStatistics;
            match summary_map.get_mut(&summary.summary_id) {
                Some(statistics) => {
                    summary_statistics = statistics;
                },
                None => {
                    let statistics = SummaryStatistics::new(*&summary.summary_id.clone());
                    summary_map.insert(summary.summary_id.clone(), statistics);
                    summary_statistics = summary_map.get_mut(&summary.summary_id).unwrap();
                }
            }

            for event in summary.events {
                summary_statistics.sample_size += event.sample_size;
                summary_statistics.squared_total_loss += event.squared_total_loss;
                summary_statistics.total_loss += event.total_loss;
                summary_statistics.ni_loss_squared += event.ni_loss_squared;
                summary_statistics.ni_loss += event.ni_loss;

                for (key, value) in event.period_categories {
                    match summary_statistics.period_categories.get_mut(&key) {
                        Some(data) => {
                            add_two_vectors(data, &value);
                        },
                        None => {
                            summary_statistics.period_categories.insert(key, value);
                        }
                    }
                }

                for occurrence in *occurrences_vec {
                    let occ_period = occurrence.period_num;

                    match summary_statistics.ni_loss_map.get_mut(&occ_period) {
                        Some(data) => {
                            *data += event.numerical_mean
                        },
                        None => {
                            summary_statistics.ni_loss_map.insert(occ_period, event.numerical_mean);
                        }
                    }
                }
            }
        }
        return 1
    }).collect::<Vec<i32>>();
    // .collect_into_vec(&mut buffer);

    let summary_statistics = summary_map.get_mut(&1).unwrap();

    // calculate the standard deviation and mean for the summary ID
    let type_one_ni = summary_statistics.ni_loss / number_of_periods as f32;
    let type_two_sample = summary_statistics.total_loss / (number_of_periods * number_of_periods) as f32;
    let standard_deviation = calculate_standard_deviation(&summary_statistics.ni_loss_map, number_of_periods);
    let standard_deviation_two = calculate_st_deviation_two(&summary_statistics.period_categories, number_of_periods * 10);

    // printout the statistics
    println!("standard deviation: {:?}", standard_deviation);
    println!("standard deviation two: {:?}", standard_deviation_two);
    println!("total loss: {:?}", summary_statistics.total_loss);
    println!("type 1 ni: {:?}", type_one_ni);
    println!("type 2 sample: {:?}", type_two_sample);
}
