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
    let occurrences = Arc::new(raw_data);

    let files = [
        String::from("./work/summary_aal/summary_1.bin"), 
        String::from("./work/summary_aal/summary_2.bin")
        ];

    // let mut buffer: Vec<i32> = vec![];
    // let sidx_totals: Arc<Mutex<HashMap<i32, f32>>> = Arc::new(Mutex::new(HashMap::new()));
    let mut sidx_totals: HashMap<i32, f32> = HashMap::new();
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

                    for event in summary.events {

                        for loss in event.losses {
                            let sidx = loss.0;
                            let raw_loss = loss.1;
                            let loss = loss.1 * occ_num as f32;

                            sample_size += occ_num;

                            for _ in 0..occ_num {
                                squared_total_loss += raw_loss * raw_loss;
                                total_loss += raw_loss;
                            }

                            match sidx_totals.get_mut(&sidx) {
                                Some(total) => {
                                    *total += loss
                                }, 
                                None => {
                                    sidx_totals.insert(sidx, loss);
                                }
                            }
 
                        }

                        for _ in 0..occ_num {
                            ni_loss += event.numerical_mean;
                            ni_loss_squared += event.numerical_mean * event.numerical_mean;
                        }
                        // ni_loss += event.numerical_mean * occ_num as f32;
                    }

                    // drop(internal_sidx_totals);
                },
                None =>{}
            }
        }
        return 1
    }).collect::<Vec<i32>>();
    // .collect_into_vec(&mut buffer);

    let mut total_sample_losses = 0.0;
    let mut key_placeholder = 0;
    // let totals = sidx_totals.lock().unwrap();

    for key in sidx_totals.keys() {
        if key > &0 {
            total_sample_losses += sidx_totals.get(key).unwrap();

            if key > &key_placeholder {
                key_placeholder = *key;
            }
        }
    }

    let number_of_years = sidx_totals.keys().len() as i32;

    // standard deviation run experimental
    let alpha = (ni_loss * ni_loss) / number_of_years as f32;
    let beta = (ni_loss_squared - alpha) / (number_of_years as f32 - 1.0);
    let std_dev = f32::sqrt(beta);

    println!("The test standard deviation is: {}", std_dev);


    println!("{:?}", occurrences);

    let type_one_ni = ni_loss / number_of_years as f32;
    let type_two_sample = total_sample_losses / (number_of_years * number_of_years) as f32;

    // calculate the standard deviation 
    // let mean = total_mean / sample_size as f32;
    let total_mean = total_loss / sample_size as f32;
    let mean_squared = total_mean * total_mean;

    let rec_mean_squared = squared_total_loss / sample_size as f32;

    // s1 is the mean of the square - the square of the mean
    let s1 = (rec_mean_squared - mean_squared) / (sample_size * number_of_years) as f32;
    let s2 = s1 / (number_of_years * sample_size -1) as f32;
    let sd_dev = f64::sqrt(s2 as f64);



    // drop(totals);
    println!("standard deviation: {}", sd_dev);
    println!("total sample losses: {:?}", total_sample_losses);
    println!("ni loss: {:?}", ni_loss);
    println!("total loss: {:?}", total_loss);
    println!("type 1 ni: {:?}", type_one_ni);
    println!("type 2 sample: {:?}", type_two_sample);
    println!("highest sidx: {:?}", key_placeholder);
}
