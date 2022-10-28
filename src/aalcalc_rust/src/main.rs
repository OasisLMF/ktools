use rayon::prelude::*;

mod data_access_layer;
use data_access_layer::occurrence::OccurrenceData;
use data_access_layer::summary::{SummaryData};
use std::sync::{Arc, Mutex};
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

    let mut buffer: Vec<i32> = vec![];
    let sidx_totals: Arc<Mutex<HashMap<i32, f32>>> = Arc::new(Mutex::new(HashMap::new()));

    // run the processes in parallel
    let _ = files.par_iter().map(|i| {
        let mut sum_data = SummaryData::new(i.clone());
        let summaries = sum_data.get_data();

        for summary in summaries {
            match &occurrences.get(&summary.event_id) {
                Some(occurrences_vec) => {
                    let occ_num = occurrences_vec.len() as i32;

                    let mut internal_sidx_totals = sidx_totals.lock().unwrap();

                    for event in summary.events {
                        for loss in event.losses {
                            let sidx = loss.0;
                            let loss = loss.1 * occ_num as f32;

                            if sidx > -2 {
                                match internal_sidx_totals.get_mut(&sidx) {
                                    Some(total) => {
                                        *total += loss
                                    }, 
                                    None => {
                                        internal_sidx_totals.insert(sidx, loss);
                                    }
                                }
                            }
                        }
                    }
                    drop(internal_sidx_totals);
                },
                None =>{}
            }
        }
        return 1
    }).collect_into_vec(&mut buffer);

    let mut total_sample_losses = 0.0;
    let mut key_placeholder = 0;
    let totals = sidx_totals.lock().unwrap();

    for key in totals.keys() {
        if key > &0 {
            total_sample_losses += totals.get(key).unwrap();

            if key > &key_placeholder {
                key_placeholder = *key;
            }
        }
    }
    let ni_loss = totals.get(&-1).unwrap().clone();
    let number_of_years = 10;
    let type_one_ni = ni_loss / number_of_years as f32;
    let type_two_sample = total_sample_losses / (number_of_years * number_of_years) as f32;

    drop(totals);
    println!("total sample losses: {:?}", total_sample_losses);
    println!("ni loss: {:?}", ni_loss);
    println!("type 1 ni: {:?}", type_one_ni);
    println!("type 2 sample: {:?}", type_two_sample);
    println!("highest sidx: {:?}", key_placeholder);
}
