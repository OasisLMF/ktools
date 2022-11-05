// use rayon::prelude::*;

mod data_access_layer;
use data_access_layer::occurrence::OccurrenceData;
use data_access_layer::summary::{SummaryData};
use std::sync::Arc;
use std::collections::HashMap;


#[tokio::main]
async fn main() {
    let mut occ_data = OccurrenceData::new(String::from("./input/occurrence.bin")).await;
    let raw_data = occ_data.get_data().await;
    let occurrences = raw_data;

    let files = [
        String::from("./work/summary_aal/summary_1.bin"), 
        String::from("./work/summary_aal/summary_2.bin")
        ];

    // let mut buffer: Vec<i32> = vec![];
    // let sidx_totals: Arc<Mutex<HashMap<i32, f32>>> = Arc::new(Mutex::new(HashMap::new()));
    let mut sidx_totals: HashMap<i32, f32> = HashMap::new();
    let mut ni_loss_map: HashMap<i32, f32> = HashMap::new();

    let mut ni_loss = 0.0;
    let mut ni_loss_squared = 0.0;

    let mut sample_size = 0;
    let mut total_loss = 0.0;
    let mut squared_total_loss = 0.0;

    // run the processes in parallel
    let _ = files.iter().map(|i| {
        let mut sum_data = SummaryData::new(i.clone());
        let summaries = sum_data.get_data();

        for summary in summaries {
            match &occurrences.get(&summary.event_id) {
                Some(occurrences_vec) => {
                    // obtaining the number of times an event occurs
                    let occ_num = occurrences_vec.len() as i32;

                    // let mut internal_sidx_totals = sidx_totals.lock().unwrap();

                    // this is the add_loss function with occ_num argument
                    for event in summary.events {

                        // this loops through the losses. look into calculating the total losses
                        // as the events are read from the file
                        for loss in event.losses {
                            let sidx = loss.0;
                            let raw_loss = loss.1;
                            let loss = loss.1 * occ_num as f32;

                            // add sample_size field for event
                            sample_size += occ_num;

                            for _ in 0..occ_num {
                                // add squared_total_loss field for event
                                squared_total_loss += raw_loss * raw_loss;

                                // add total_loss field for event
                                total_loss += raw_loss;
                            }

                            // add sidx_totals as field for event
                            match sidx_totals.get_mut(&sidx) {
                                Some(total) => {
                                    *total += loss
                                }, 
                                None => {
                                    sidx_totals.insert(sidx, loss);
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
                    // drop(internal_sidx_totals);
                },
                None =>{}
            }
        }
        return 1
    }).collect::<Vec<i32>>();
    // .collect_into_vec(&mut buffer);

    // calculating the standard deviation starts
    let mut sum_squared = 0.0;
    let mut sum = 0.0;
    // square and sum period losses to the ni
    for key in ni_loss_map.keys() {
        let loss = ni_loss_map.get(&key).unwrap();
        sum += loss.clone();
        sum_squared += loss * loss;
    }
    sum = sum * sum;
    let number_of_years = sidx_totals.keys().len() as i32;

    let alpha = sum_squared - (sum / number_of_years as f32);
    let beta = alpha / (number_of_years - 1) as f32;
    // calculating the standard deviation ends

    let type_one_ni = ni_loss / number_of_years as f32;
    let type_two_sample = total_loss / (number_of_years * number_of_years) as f32;
    let standard_deviation = f32::sqrt(beta);

    // drop(totals);
    println!("standard deviation: {:?}", standard_deviation);
    // println!("ni loss squared: {:?}", ni_loss_squared);
    // println!("ni loss: {:?}", ni_loss);
    println!("total loss: {:?}", total_loss);
    println!("type 1 ni: {:?}", type_one_ni);
    println!("type 2 sample: {:?}", type_two_sample);
}
