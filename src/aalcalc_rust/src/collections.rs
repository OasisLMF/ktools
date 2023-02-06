//!The collections needed to organise the data for multiple summaries. If we were to move to
//! multithreading or processing these collectikons will be used to orchestrate data
//! across all threads and processes.
//!
//! # summary statistics
//! This houses all the statistics for a particular summary. The ```Summary``` struct processes
//!the statistics per file. There are then added to the ```SummaryStatistics``` struct which
//!can be wrapped in a ```Mutex``` guard if we make the progress to multithreading/processing. To
//! construct and update a summary statistics struct we implement the following code:
//! ```rust
//! use collections::SummaryStatistics;
//! use processes::add_two_vectors;
//!
//! let statistics = SummaryStatistics::new(*&summary.summary_id.clone());
//!
//! // update the summary statistics with the data from the summary
//! summary_statistics.sample_size = summary.sample_size;
//! summary_statistics.squared_total_loss += summary.squared_total_loss;
//! summary_statistics.total_loss += summary.total_loss;
//! summary_statistics.ni_loss_squared += summary.ni_loss_squared;
//! summary_statistics.ni_loss += summary.ni_loss;
//!
//! for (key, value) in summary.period_categories {
//!     match summary_statistics.period_categories.get_mut(&key) {
//!         Some(data) => {
//!             add_two_vectors(data, &value);
//!         },
//!         None => {
//!             summary_statistics.period_categories.insert(key, value);
//!         }
//!     }
//! }
//! for (key, value) in summary.ni_loss_map {
//!     match summary_statistics.ni_loss_map.get_mut(&key) {
//!         Some(data) => {
//!             *data += value;
//!         },
//!         None => {
//!             summary_statistics.ni_loss_map.insert(key, value);
//!         }
//!     }
//! }
//! ```
use std::collections::HashMap;

use super::processes::{calculate_standard_deviation, calculate_st_deviation_two};


/// Houses the summary statistics for a summary from all files.
///
/// # Fields
/// * summary_id: the ID of the summary the statistics belong to
/// * ni_loss: The total losses for the event where each loss is multiplied by the amount of times
/// the loss occurs in the occurrence data.
/// * ni_loss_squared: the sum of each loss multiplied by the occurrence squared
/// * sample_size: the number of samples taken (will be consistent throughout whole of AAL)
/// * total_loss: the total loss of all events under the summary
/// * squared_total_loss: the total loss of all squared losses under all events under the summary
/// * ni_loss_map: total losses multiplied by occurrence of events mapped by period number
/// * period_categories: a vector of total losses which is the length of the sample size which can
/// be accessed using the period number as the key.
#[derive(Debug)]
pub struct SummaryStatistics {
    pub summary_id: i32,
    pub ni_loss: f64,
    pub ni_loss_squared: f64,
    pub sample_size: i32,
    pub total_loss: f64,
    pub squared_total_loss: f64,
    pub ni_loss_map: HashMap<i32, f64>,
    pub period_categories: HashMap<i32, Vec<f64>>
}

impl SummaryStatistics {

    /// The constructor for the ```SummaryStatistics``` struct.
    ///
    /// # Arguments
    /// * summary_id: the ID of the summaries that statistics are going to be collected on
    pub fn new(summary_id: i32) -> Self {
        let ni_loss_map: HashMap<i32, f64> = HashMap::new();
        let period_categories: HashMap<i32, Vec<f64>> = HashMap::new();
        return SummaryStatistics{
            summary_id,
            ni_loss: 0.0,
            ni_loss_squared: 0.0,
            sample_size: 0,
            total_loss: 0.0,
            squared_total_loss: 0.0,
            ni_loss_map,
            period_categories
        }
    }

    /// Prints out type one statistics resulting in an outcome like the following:
    /// ```
    /// summary_id,type,mean,standard_deviation
    /// 1,1,362000,643234.9
    /// 2,1,12750,40319.043
    /// ```
    /// # Arguments
    /// * number_of_periods: the ```period_number``` of the constructed ```OccurrenceData```
    pub fn print_type_one_stats(&self, number_of_periods: i32) {
        let type_one_ni = self.ni_loss / number_of_periods as f64;
        let standard_deviation = calculate_standard_deviation(&self.ni_loss_map, number_of_periods);
        println!("{},1,{:.6},{:.6}", self.summary_id, type_one_ni, standard_deviation);
    }

    /// Prints out the type two statistics resulting in an outcome like the following:
    /// ```
    /// summary_id,type,mean,standard_deviation
    /// 1,2,355150.75,681288.75
    /// 2,2,12683.684,48889.297
    /// ```
    ///
    /// # Arguments
    /// * number_of_periods: the ```period_number``` of the constructed ```OccurrenceData```
    pub fn print_type_two_stats(&self, number_of_periods: i32) {
        let denominator  = self.sample_size * number_of_periods;
        let type_two_sample = self.total_loss / denominator as f64;
        let standard_deviation_two = calculate_st_deviation_two(&self.period_categories, denominator);
        println!("{},2,{:.6},{:.6}", self.summary_id, type_two_sample, standard_deviation_two);
    }

}
