//!Summary data is a general summary of the events belonging to the summary and losses belonging to
//! the events.
//! # Loading
//! Summary data is generally houses in a series of ```bin``` files, therefore it is advised to
//!collect all ```bin``` files in the directory using ```glob```:
//!```rust
//!use processes::get_all_binary_file_paths;
//!
//!let file_pattern = String::from("./work/summary_aal_two/*.bin");
//!let files = get_all_binary_file_paths(file_pattern);
//!```
//!We can then loop through the file paths denoting each file path as ```i``` loading the summaries
//!with the following code:
//!```rust
//!let mut sum_data: SummaryData = SummaryData::new(i.clone());
//!let vec_capacity: i32 = sum_data.no_of_samples;
//!let summaries: Vec<Summary> = sum_data.get_data(&occurrences, vec_capacity);
//!```
//! # Notes
//! It should be noted that individual losses and events are not housed in the summary data but.
//! Instead events and losses are merely processed when loaded from the file and then dropped. This
//! is because AAL is summary data such as totals and means etc. There is no need at this current
//! time to keep all the events and losses in memory once the data of that loss and event has been
//! added to the tally.
use std::collections::HashMap;

use super::occurrence::Occurrence;
use crate::processes::add_two_vectors;
use crate::collections::event::Event;
use crate::data_access_layer::summary_loader::SummaryLoader;
use crate::data_access_layer::traits::load_summary_data::{
    ReadSummaryData,
    ExtractSummaryData
};


/// Houses the data around a summary. 
/// 
/// # Fields
/// * **event_id:** the ID of the event that the summary belongs to
/// * **summary_id:** the ID of the summary
/// * **exposure_value:** the total insured value of the summary
/// * **sample_size:** the number of samples taken (will be consistent throughout whole of AAL)
/// * **total_loss:** the total loss of all events under the summary
/// * **squared_total_loss:** the total loss of all squared losses under all events under the summary
/// * **ni_loss_map:** total losses multiplied by occurrence of events mapped by period number
/// * **period_categories:** a vector of total losses which is the length of the sample size which can
/// be accessed using the period number as the key.
/// * **ni_loss:** The total losses for the event where each loss is multiplied by the amount of times
/// the loss occurs in the occurrence data.
/// * **ni_loss_squared:** the sum of each loss multiplied by the occurrence squared
/// * **numerical_mean:** the mean of all the events belonging to the summary
#[derive(Debug, Clone)]
pub struct Summary {
    pub event_id: i32,
    pub summary_id: i32,
    pub exposure_value: i32,
    // below are statistics needed for the event
    pub sample_size: i32,
    pub total_loss: f64,
    pub squared_total_loss: f64,
    pub ni_loss_map: HashMap<i32, f64>,
    pub period_categories: HashMap<i32, Vec<f64>>,
    pub ni_loss_squared: f64,
    pub ni_loss: f64,
    pub numerical_mean: f64
}

impl Summary {

    /// Constructs the ```Summary``` struct using bytes
    /// 
    /// # Arguments
    /// * **event_id:** the ID of the event that the summary belongs to
    /// * **summary_id:** the ID of the summary
    /// * **exposure_value:** don't know will need to be filled in
    pub fn from_values(event_id: i32, summary_id: i32, exposure_value: i32) -> Self {
        // let events: Vec<Event> = vec![];
        let ni_loss_map: HashMap<i32, f64> = HashMap::new();
        let period_categories: HashMap<i32, Vec<f64>> = HashMap::new();

        return Summary { 
            event_id, 
            summary_id, 
            exposure_value, 
            // events: events,
            sample_size: 0,
            total_loss: 0.0,
            squared_total_loss: 0.0,
            ni_loss_map,
            period_categories,
            ni_loss_squared: 0.0,
            ni_loss: 0.0,
            numerical_mean: 0.0
        }
    }

    /// Updates the statistics by consuming an event and adding this to the statistics. 
    /// 
    /// # Arguments
    /// * **event:** the event to be added to the statistics
    /// * **occurrences_vec:** the vector of occurrences for the event
    pub fn ingest_event(&mut self, event: Event, occurrences_vec: &Vec<Occurrence>) {
        self.sample_size = event.sample_size;
        self.squared_total_loss += event.squared_total_loss;
        self.total_loss += event.total_loss;
        self.ni_loss_squared += event.ni_loss_squared;
        self.ni_loss += event.ni_loss;

        for (key, value) in &event.period_categories {
            match self.period_categories.get_mut(&key) {
                Some(data) => {
                    add_two_vectors(data, &value);
                },
                None => {
                    self.period_categories.insert(key.clone(), value.clone());
                }
            }
        }
        for occurrence in occurrences_vec {
            let occ_period = occurrence.period_num;

            match self.ni_loss_map.get_mut(&occ_period) {
                Some(data) => {
                    *data += &event.numerical_mean;
                },
                None => {
                    self.ni_loss_map.insert(occ_period, event.numerical_mean.clone());
                }
            }
        }
    }
}


/// Houses meta data around the loading and handling of summary data. 
/// 
/// # Fields 
/// * **stream_id:** the ID of the stream for the summary 
/// * **no_of_samples:** The number of samples per summary (consistent for each summary)
/// * **summary_set:** don't know will need to be filled in
#[derive(Debug)]
pub struct SummaryData {
    pub stream_id: i32,
    pub no_of_samples: i32,
    pub summary_set: i32,
}


/// Loads the summary data from the file and returns a vector of summaries.
/// 
/// # Arguments
/// * **path:** the path to the summary file
/// * **handle:** the handle to the summary file which implements the ```ReadSummaryData``` trait
/// * **reference_data:** the reference data which is a map of event IDs to a vector of occurrences
/// 
/// # Returns
/// * **Result<Vec<Summary>, std::io::Error>** a vector of summaries or an error
pub fn get_summaries_from_data(path: String, handle: &mut dyn ReadSummaryData, reference_data: &HashMap<i32, Vec<Occurrence>>) -> Result<Vec<Summary>, std::io::Error>
{
    let mut buffer = vec![];
    let mut loader = SummaryLoader::new(path, handle)?;
    let file_header = loader.extract_file_header();

    let summary_data = SummaryData {
        stream_id: file_header.0,
        no_of_samples: file_header.1,
        summary_set: file_header.2
    };

    while loader.finish <= loader.end {
        let header = loader.extract_summary_header();
        let mut summary = Summary::from_values(header.0, header.1, header.2);

        let occurrences_vec: &Vec<Occurrence>;
        let placeholder: Vec<Occurrence> = Vec::new();
        match reference_data.get(&summary.event_id) {
            Some(data) => {
                occurrences_vec = data;
            },
            None => {
                occurrences_vec = &placeholder;
            }
        }

        let period_categories = HashMap::new();
        let mut event = Event::default(summary.event_id, period_categories);

        // extract the losses for the event
        while loader.finish <= loader.end {
            let loss = loader.extract_loss();
            let pushed = event.add_loss(loss.0, loss.1, occurrences_vec, summary_data.no_of_samples);

            if pushed == false {
                summary.ingest_event(event, &occurrences_vec);
                break
            }
        }
        buffer.push(summary); 
    }
    return Ok(buffer)
}


// #[cfg(test)]
// mod summary_data_tests {

//     use super::{SummaryData};

//     #[test]
//     fn test_new() {
//         let sum_data = SummaryData::new(String::from("./work/summary_aal/summary_1.bin"));
//         assert_eq!(50331649, sum_data.stream_id);
//         assert_eq!(10, sum_data.no_of_samples);
//         assert_eq!(1, sum_data.summary_set);
//     }
// }
