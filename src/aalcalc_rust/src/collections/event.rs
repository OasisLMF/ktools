//! Here we define the ```Event``` struct which handles the losses and summary statistics around an event.
use std::collections::HashMap;

use crate::data_access_layer::occurrence::Occurrence;


/// Holds the summary statistics for a collection of losses under an event.
///
/// # Fields
/// * **event_id:** the ID of the event
/// * **maximum_loss:** the maximum loss that the event has endured
/// * **numerical_mean:** the mean of all the losses belonging to the event
/// * **standard_deviation:** The standard deviation for the losses belonging to the event
/// * **sample_size:** the number of samples that houses, it must be noted that the sample size
/// across the entire AAL calculation will be uniform
/// * **period_categories:** a vector of total losses which is the length of the sample size which can
/// be accessed using the period number as the key.
/// * **squared_total_loss:** The sum of squares of each loss belonging to the event
/// * **total_loss:** the total loss of all the losses belonging to the event
/// * **ni_loss:** The total losses for the event where each loss is multiplied by the amount of times
/// the loss occurs in the occurrence data.
/// * **ni_loss_squared:** the sum of each loss multiplied by the occurrence squared
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

    /// Creates a default Event. 
    /// 
    /// # Arguments
    /// * **event_id:** the ID of the event
    /// * **period_categories:** a vector of total losses which is the length of the sample size
    /// 
    /// # Returns
    /// the constructed ```Event``` struct
    pub fn default(event_id: i32, period_categories: HashMap<i32, Vec<f64>>) -> Self {
        Event{
            event_id: event_id,
            maximum_loss: None,
            numerical_mean: 0.0,
            standard_deviation: None,
            sample_size: 0,
            period_categories,
            squared_total_loss: 0.0,
            total_loss: 0.0,
            ni_loss: 0.0,
            ni_loss_squared: 0.0
        }
    }

    /// Adds a loss to the ```self.losses```.
    /// 
    /// # Arguments
    /// * **sidx:** the sample ID of the loss
    /// * **loss:** the total amount of the loss
    /// * **occurrence_vec:** the occurrences of the event
    /// * **vec_capacity:** The vec capacity needed for a ```self.period_categories``` insert which is
    /// usually the number of samples
    /// 
    /// # Returns
    /// ```true```: loss added and should continue to add losses
    /// ```false```: the end of event stream has been reached
    pub fn add_loss(&mut self, sidx_int: i32, loss_float: f64, occurrence_vec: &Vec<Occurrence>, vec_capacity: i32) -> bool {

        // double zeros indicate the end of the stream
        if sidx_int == 0 && loss_float == 0.0 {
            return false
        }
        // minus values indicate meta data around the loss
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
            // positive values are losses
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
