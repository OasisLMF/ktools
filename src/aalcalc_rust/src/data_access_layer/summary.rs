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
use byteorder::{ByteOrder, LittleEndian};
use std::{fs::File, io::Read};
use std::collections::HashMap;

use super::occurrence::Occurrence;
use crate::processes::add_two_vectors;


/// Holds the summary statistics for a collection of losses under an event.
///
/// # Fields
/// * event_id: the ID of the event
/// * maximum_loss: the maximum loss that the event has endured
/// * numerical_mean: the mean of all the losses belonging to the event
/// * standard_deviation: The standard deviation for the losses belonging to the event
/// * sample_size: the number of samples that houses, it must be noted that the sample size
/// across the entire AAL calculation will be uniform
/// * period_categories: a vector of total losses which is the length of the sample size which can
/// be accessed using the period number as the key.
/// * squared_total_loss: The sum of squares of each loss belonging to the event
/// * total_loss: the total loss of all the losses belonging to the event
/// * ni_loss: The total losses for the event where each loss is multiplied by the amount of times
/// the loss occurs in the occurrence data.
/// * ni_loss_squared: the sum of each loss multiplied by the occurrence squared
#[derive(Debug, Clone)]
pub struct Event {
    pub event_id: i32,
    pub maximum_loss: Option<f64>,
    pub numerical_mean: f64,
    pub standard_deviation: Option<f64>,
    pub sample_size: i32,
    pub period_categories: HashMap<i32, Vec<f64>>,
    pub squared_total_loss: f64,
    pub total_loss: f64,
    pub ni_loss: f64,
    pub ni_loss_squared: f64
}

impl Event {

    /// Adds a loss to the ```self.losses```.
    /// 
    /// # Arguments
    /// * sidx: the sample ID of the loss
    /// * loss: the total amount of the loss
    /// * occurrence_vec: the occurrences of the event
    /// * vec_capacity: The vec capacity needed for a ```self.period_categories``` insert which is
    /// usually the number of samples
    /// 
    /// # Returns
    /// ```true```: loss added and should continue to add losses
    /// ```false```: the end of event stream has been reached
    pub fn add_loss(&mut self, sidx: &[u8], loss: &[u8], occurrence_vec: &Vec<Occurrence>, vec_capacity: i32) -> bool {
        
        // read the raw data 
        let sidx_int = LittleEndian::read_i32(sidx);
        let loss_float = LittleEndian::read_f32(loss) as f64;


        if sidx_int == 0 && loss_float == 0.0 {
            return false
        }

        match sidx_int {
            -1 => {
                self.numerical_mean += loss_float;
                let occ_num = occurrence_vec.len() as i32;
                
                let mut cache = 0.0;
                for _ in 0..occ_num {
                    self.ni_loss += loss_float;
                    cache += loss_float;
                }
                self.ni_loss_squared += cache * cache;

            },
            -2 => {
                self.standard_deviation = Some(loss_float);
            },
            -5 => {
                self.maximum_loss = Some(loss_float);
            },
            _ => {
                self.sample_size += 1;

                for occ in occurrence_vec {
                    let index = sidx_int - 1;

                    match self.period_categories.get_mut(&occ.period_num) {
                        Some(total_array) => {
                            total_array[index as usize] += loss_float;
                        },
                        None => {
                            let mut buffer = vec![0.0; vec_capacity as usize];
                            buffer[index as usize] += loss_float;
                            self.period_categories.insert(occ.period_num, buffer);
                        }
                    }
                    // add squared_total_loss field for event
                    self.squared_total_loss += loss_float * loss_float;

                    // add total_loss field for event
                    self.total_loss += loss_float;
                }
            }
        }
        return true
    }

}


/// Houses the data around a summary. 
/// 
/// # Fields
/// * event_id: the ID of the event that the summary belongs to
/// * summary_id: the ID of the summary
/// * exposure_value: the total insured value of the summary
/// * sample_size: the number of samples taken (will be consistent throughout whole of AAL)
/// * total_loss: the total loss of all events under the summary
/// * squared_total_loss: the total loss of all squared losses under all events under the summary
/// * ni_loss_map: total losses multiplied by occurrence of events mapped by period number
/// * period_categories: a vector of total losses which is the length of the sample size which can
/// be accessed using the period number as the key.
/// * ni_loss: The total losses for the event where each loss is multiplied by the amount of times
/// the loss occurs in the occurrence data.
/// * ni_loss_squared: the sum of each loss multiplied by the occurrence squared
/// * numerical_mean: the mean of all the events belonging to the summary
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
    /// * event_id: the ID of the event that the summary belongs to
    /// * summary_id: the ID of the summary
    /// * exposure_value: don't know will need to be filled in
    pub fn from_bytes(event_id: &[u8], summary_id: &[u8], exposure_value: &[u8]) -> Self {
        // let events: Vec<Event> = vec![];
        let ni_loss_map: HashMap<i32, f64> = HashMap::new();
        let period_categories: HashMap<i32, Vec<f64>> = HashMap::new();

        return Summary { 
            event_id: LittleEndian::read_i32(event_id), 
            summary_id: LittleEndian::read_i32(summary_id), 
            exposure_value: LittleEndian::read_i32(exposure_value), 
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
}


/// Houses meta data around the loading and handling of summary data. 
/// 
/// # Fields 
/// * handler: handles the reading and writing of a binary file
/// * stream_id: the ID of the stream for the summary 
/// * no_of_samples: The number of samples per summary (consistent for each summary)
/// * summary_set: don't know will need to be filled in
/// * data: raw bytes loaded from the binary file to be processed
#[derive(Debug)]
pub struct SummaryData {
    pub handler: File,
    pub stream_id: i32,
    pub no_of_samples: i32,
    pub summary_set: i32,
    pub data: Vec<u8>
}

impl SummaryData {

    /// The constructor for the SummaryData struct.
    /// 
    /// # Fields
    /// * path: the path to the binary file that is going to read
    /// 
    /// # Returns 
    /// The constructed ```SummaryData``` struct
    pub fn new(path: String) -> Self {
        let mut file = File::open(path).unwrap();
        let mut num_buffer = [0; 4];

        file.read_exact(&mut num_buffer).unwrap();
        let stream_id = LittleEndian::read_i32(&num_buffer);
        file.read_exact(&mut num_buffer).unwrap();
        let no_of_samples = LittleEndian::read_i32(&num_buffer);
        file.read_exact(&mut num_buffer).unwrap();
        let summary_set = LittleEndian::read_i32(&num_buffer);

        let mut data: Vec<u8> = Vec::new();
        file.read_to_end(&mut data).unwrap();

        return SummaryData {
            stream_id,
            no_of_samples,
            summary_set,
            handler: file,
            data
        }
    }

    /// Gets all the summaries and events belonging to the summary in the binary file attached
    /// to the ```SummaryData``` struct. Whilst the data is being loaded from the file, summary
    /// statistics are calculated on the fly and added to each summary in-turn dropping all events
    /// and losses from memory as soon as the data is added to the summary statistics.
    ///
    /// # Arugments
    /// * reference_data: vector of occurrences mapped by event_id
    /// * vec_capacity: number of samples that need to be stored
    /// 
    /// # Returns
    /// all the summaries in the binary file attached to the ```SummaryData``` struct
    pub fn get_data(&mut self, reference_data: &HashMap<i32, Vec<Occurrence>>, vec_capacity: i32) -> Vec<Summary> {
        let mut buffer = vec![];

        let end = self.data.len();
        let mut start = 0;
        let mut finish = 4;

        while finish <= end {

            // extract the header data
            let event_id = &self.data[start..finish];
            start += 4;
            finish += 4;
            let summary_id = &self.data[start..finish];
            start += 4;
            finish += 4;
            let exposure_value = &self.data[start..finish];
            start += 4;
            finish += 4;

            let mut summary = Summary::from_bytes(
                event_id,
                summary_id,
                exposure_value
            );
            // println!("{:?}", reference_data);
            // println!("{}", &summary.event_id);
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
            // let occurrences_vec = reference_data.get(&summary.event_id).unwrap();
            let period_categories = HashMap::new();

            let mut event = Event{
                event_id: summary.event_id.clone(),
                maximum_loss: None,
                numerical_mean: 0.0,
                standard_deviation: None,
                sample_size: 0,
                period_categories,
                squared_total_loss: 0.0,
                total_loss: 0.0,
                ni_loss: 0.0,
                ni_loss_squared: 0.0
            };

            // extract the losses for the event
            while finish <= end {
                let sidx = &self.data[start..finish];
                start += 4;
                finish += 4;
                let loss = &self.data[start..finish];
                start += 4;
                finish += 4;
                let pushed = event.add_loss(sidx, loss, occurrences_vec, vec_capacity);

                // if false this is the end of the event stream for the summary.
                if pushed == false {
                    summary.sample_size = event.sample_size;
                    summary.squared_total_loss += event.squared_total_loss;
                    summary.total_loss += event.total_loss;
                    summary.ni_loss_squared += event.ni_loss_squared;
                    summary.ni_loss += event.ni_loss;

                    for (key, value) in &event.period_categories {
                        match summary.period_categories.get_mut(&key) {
                            Some(data) => {
                                add_two_vectors(data, &value);
                            },
                            None => {
                                summary.period_categories.insert(key.clone(), value.clone());
                            }
                        }
                    }
                    for occurrence in occurrences_vec {
                        let occ_period = occurrence.period_num;

                        match summary.ni_loss_map.get_mut(&occ_period) {
                            Some(data) => {
                                *data += &event.numerical_mean;
                            },
                            None => {
                                summary.ni_loss_map.insert(occ_period, event.numerical_mean.clone());
                            }
                        }
                    }
                    // summary.events.push(event);
                    break
                }
            }
            buffer.push(summary);
        }
        return buffer
    }
}


#[cfg(test)]
mod summary_data_tests {

    use super::{SummaryData};

    #[test]
    fn test_new() {
        let sum_data = SummaryData::new(String::from("./work/summary_aal/summary_1.bin"));
        assert_eq!(50331649, sum_data.stream_id);
        assert_eq!(10, sum_data.no_of_samples);
        assert_eq!(1, sum_data.summary_set);
    }
}
