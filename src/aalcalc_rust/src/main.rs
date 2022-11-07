mod data_access_layer;
mod processes;

use data_access_layer::occurrence::OccurrenceData;
use data_access_layer::summary::{SummaryData};
use processes::{calculate_standard_deviation, calculate_st_deviation_two};

use std::collections::HashMap;


fn add_two_vectors(one: &mut Vec<f32>, two: &Vec<f32>) {
    for i in 0..one.len() {
        one[i as usize] += two[i as usize];
    }
}


#[tokio::main]
async fn main() {
    let mut occ_data = OccurrenceData::new(String::from("./input/occurrence.bin")).await;
    let raw_data = occ_data.get_data().await;
    let occurrences = raw_data;
    let number_of_periods = occ_data.period_number;

    let files = [
        String::from("./work/summary_aal/summary_1.bin"), 
        String::from("./work/summary_aal/summary_2.bin")
        ];

    let mut ni_loss_map: HashMap<i32, f32> = HashMap::new();
    let mut period_categories: HashMap<i32, Vec<f32>> = HashMap::new();

    let mut ni_loss = 0.0;
    let mut ni_loss_squared = 0.0;

    let mut sample_size = 0;
    let mut total_loss = 0.0;
    let mut squared_total_loss = 0.0;

    // run the processes in parallel
    let _ = files.iter().map(|i| {
        let mut sum_data = SummaryData::new(i.clone());
        let vec_capacity = sum_data.no_of_samples;
        let summaries = sum_data.get_data(&occurrences, vec_capacity);

        for summary in summaries {
            let occurrences_vec = &occurrences.get(&summary.event_id).unwrap();

            // obtaining the number of times an event occurs
            let occ_num = occurrences_vec.len() as i32;

            // this is the add_loss function with occ_num argument
            for event in summary.events {

                // this loops through the losses. look into calculating the total losses
                // as the events are read from the file
                sample_size += event.sample_size;
                squared_total_loss += event.squared_total_loss;
                total_loss += event.total_loss;

                for (key, value) in event.period_categories {
                    match period_categories.get_mut(&key) {
                        Some(data) => {
                            add_two_vectors(data, &value);
                        },
                        None => {
                            period_categories.insert(key, value);
                        }
                    }
                }

                // add occ_num argument to the add_loss function
                let mut cache = 0.0;
                for _ in 0..occ_num {
                    // add ni_loss field to event
                    ni_loss += event.numerical_mean;
                    cache += event.numerical_mean;
                }
                // add ni_loss_squared to for event
                ni_loss_squared += cache * cache;

                for occurrence in *occurrences_vec {
                    let occ_period = occurrence.period_num;

                    match ni_loss_map.get_mut(&occ_period) {
                        Some(data) => {
                            *data += event.numerical_mean
                        },
                        None => {
                            ni_loss_map.insert(occ_period, event.numerical_mean);
                        }
                    }
                }
            }
        }
        return 1
    }).collect::<Vec<i32>>();
    // .collect_into_vec(&mut buffer);

    let type_one_ni = ni_loss / number_of_periods as f32;
    let type_two_sample = total_loss / (number_of_periods * number_of_periods) as f32;
    let standard_deviation = calculate_standard_deviation(&ni_loss_map, number_of_periods);
    let standard_deviation_two = calculate_st_deviation_two(&period_categories, number_of_periods * 10);

    println!("standard deviation: {:?}", standard_deviation);
    println!("standard deviation two: {:?}", standard_deviation_two);
    println!("total loss: {:?}", total_loss);
    println!("type 1 ni: {:?}", type_one_ni);
    println!("type 2 sample: {:?}", type_two_sample);
}
