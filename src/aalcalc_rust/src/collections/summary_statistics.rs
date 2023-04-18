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

use super::super::processes::{
    calculate_standard_deviation, 
    calculate_st_deviation_two,
    calculate_weighted_st_deviation_two,
    calculate_weighted_standard_deviation,
    add_two_vectors
};
use crate::data_access_layer::summary::Summary;


/// Houses the summary statistics for a summary from all files.
///
/// # Fields
/// * **summary_id:** the ID of the summary the statistics belong to
/// * **ni_loss:** The total losses for the event where each loss is multiplied by the amount of times
/// the loss occurs in the occurrence data.
/// * **ni_loss_squared:** the sum of each loss multiplied by the occurrence squared
/// * **sample_size:** the number of samples taken (will be consistent throughout whole of AAL)
/// * **total_loss:** the total loss of all events under the summary
/// * **squared_total_loss:** the total loss of all squared losses under all events under the summary
/// * **ni_loss_map:** total losses multiplied by occurrence of events mapped by period number
/// * **period_categories:** a vector of total losses which is the length of the sample size which can
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
    /// * **summary_id:** the ID of the summaries that statistics are going to be collected on
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

    /// Ingests a summary to be added to the summary statistics.
    /// 
    /// # Arguments
    /// * **summary:** the summary to be added to the summary statistics
    pub fn ingest_summary(&mut self, summary: Summary) {
        self.sample_size = summary.sample_size;
        self.squared_total_loss += summary.squared_total_loss;
        self.total_loss += summary.total_loss;
        self.ni_loss_squared += summary.ni_loss_squared;
        self.ni_loss += summary.ni_loss;

        for (key, value) in summary.period_categories {
            match self.period_categories.get_mut(&key) {
                Some(data) => {
                    add_two_vectors(data, &value);
                },
                None => {
                    self.period_categories.insert(key, value);
                }
            }
        }

        for (key, value) in summary.ni_loss_map {
            match self.ni_loss_map.get_mut(&key) {
                Some(data) => {
                    *data += value;
                },
                None => {
                    self.ni_loss_map.insert(key, value);
                }
            }
        }
    }


    /// Calculates the type one statistics for the summary statistics.
    /// 
    /// # Arguments
    /// * **number_of_periods:** the number of periods in the summary
    /// * **weights:** the weights to be used in the calculation of the standard deviation
    /// 
    /// # Returns
    /// * **(f64, f64):** the type one NI loss and the standard deviation
    pub fn calculate_type_one_stats(&self, number_of_periods: i32, weights: Option<&Vec<f64>>) -> (f64, f64) {
        let type_one_ni = self.ni_loss / number_of_periods as f64;

        let standard_deviation: f64;
        let mean: f64;

        match weights {
            Some(weights_vec) => {
                (standard_deviation, mean) = calculate_weighted_standard_deviation(
                    &self.ni_loss_map, number_of_periods, weights_vec
                );
            },
            None => {
                standard_deviation = calculate_standard_deviation(
                    &self.ni_loss_map, number_of_periods
                );
                mean = type_one_ni;
            }
        }
        return (mean, standard_deviation)
    }

    /// Calculates the type two statistics for the summary statistics.
    /// 
    /// # Arguments
    /// * **number_of_periods:** the number of periods in the summary
    /// * **weights:** the weights to be used in the calculation of the standard deviation
    /// 
    /// # Returns
    /// * **(f64, f64):** the type two sample and the standard deviation
    pub fn calculate_type_two_stats(&self, number_of_periods: i32, weights: Option<&Vec<f64>>) -> (f64, f64) {
        let denominator  = self.sample_size * number_of_periods;

        let type_two_sample: f64;
        let standard_deviation_two: f64;
        match weights {
            Some(weights_vec) => {
                (standard_deviation_two, type_two_sample) = calculate_weighted_st_deviation_two(
                    &self.period_categories, denominator, weights_vec
                );
            },
            None => {
                standard_deviation_two = calculate_st_deviation_two(
                    &self.period_categories, denominator
                );
                type_two_sample = self.total_loss / denominator as f64;
            }
        }
        return (type_two_sample, standard_deviation_two)
    }

    /// Prints out type one statistics resulting in an outcome like the following:
    /// ```
    /// summary_id,type,mean,standard_deviation
    /// 1,1,362000,643234.9
    /// 2,1,12750,40319.043
    /// ```
    /// # Arguments
    /// * **number_of_periods:** the ```period_number``` of the constructed ```OccurrenceData```
    pub fn print_type_one_stats(&self, number_of_periods: i32, weights: Option<&Vec<f64>>) {
        let (type_one_ni, standard_deviation) = self.calculate_type_one_stats(number_of_periods, weights);
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
    /// * **number_of_periods:** the ```period_number``` of the constructed ```OccurrenceData```
    pub fn print_type_two_stats(&self, number_of_periods: i32, weights: Option<&Vec<f64>>) {
        let (type_two_sample, standard_deviation_two) = self.calculate_type_two_stats(number_of_periods, weights);
        println!("{},2,{:.6},{:.6}", self.summary_id, type_two_sample, standard_deviation_two);
    }

}



#[cfg(test)]
mod summary_data_tests {

    use crate::data_access_layer::summary::get_summaries_from_data;
    use crate::data_access_layer::occurrence::OccurrenceFileHandle;
    use crate::data_access_layer::occurrence::Occurrence;
    use crate::data_access_layer::traits::load_occurrence::ReadOccurrences;
    use crate::data_access_layer::summary_loader::SummaryLoaderHandle;
    use crate::data_access_layer::summary::Summary;
    use crate::collections::summary_statistics::SummaryStatistics;
    use crate::data_access_layer::period_weights::{PeriodWeights, PeriodWeightsHandle};
    use crate::data_access_layer::traits::load_period_weights::ReadPeriodWeights;
    use std::collections::HashMap;


    /// Gets the occurrence data from the tes_data files.
    /// 
    /// # Returns
    /// * **(HashMap<i32, Vec<Occurrence>>, i32):** the occurrence data and the number of periods
    fn get_occ_data() -> (HashMap<i32, Vec<Occurrence>>, i32) {
        let occurrence_path = String::from("./test_data/input/occurrence.bin");
        let mut occ_data_handle = OccurrenceFileHandle::new(occurrence_path).unwrap();
        let number_of_periods = &occ_data_handle.get_meta_data().period_number;
        let occ_data = occ_data_handle.get_data();
        return (occ_data, number_of_periods.clone())
    }

    /// Gets the summary data from the test_data files.
    /// 
    /// # Arguments
    /// * **occ_data:** the occurrence data
    /// 
    /// # Returns
    /// * **Vec<Summary>:** the summary data
    fn get_summary_data(occ_data: &HashMap<i32, Vec<Occurrence>>) -> Vec<Summary> {
        let path = String::from("./test_data/work/test/summarycalc.bin");
        let mut handle = SummaryLoaderHandle{};
        let summaries = get_summaries_from_data(path, &mut handle, &occ_data).unwrap();
        return summaries
    }

    /// Gets the period weights from the test_data files.
    /// 
    /// # Returns
    /// * **PeriodWeights:** the period weights
    fn get_period_weights() -> PeriodWeights {
        let period_weights_path = String::from("./test_data/input/periods.bin");
        let period_handle = PeriodWeightsHandle::new(period_weights_path).unwrap();
        PeriodWeights{weights: period_handle.get_data()}
    }

    #[test]
    fn test_type_one_stats_no_weights() {
        let occ_data_tuple = get_occ_data();
        let summaries = get_summary_data(&occ_data_tuple.0);

        let mut summary_statistics = SummaryStatistics::new(1);
        
        for summary in summaries {
            summary_statistics.ingest_summary(summary);
        }

        let (type_one_ni, standard_deviation) = summary_statistics.calculate_type_one_stats(
            occ_data_tuple.1, None
        );
        assert_eq!(383.6407539, (standard_deviation * 1e7).round() / 1e7);
        assert_eq!(809.2949982, (type_one_ni * 1e7).round() / 1e7)
    }

    #[test]
    fn test_type_two_stats_no_weights() {
        let occ_data_tuple = get_occ_data();
        let summaries = get_summary_data(&occ_data_tuple.0);

        let mut summary_statistics = SummaryStatistics::new(1);
        
        for summary in summaries {
            summary_statistics.ingest_summary(summary);
        }

        let (type_two_sample, standard_deviation_two) = summary_statistics.calculate_type_two_stats(
            occ_data_tuple.1, None
        );
        assert_eq!(809.2909963, (type_two_sample * 1e7).round() / 1e7);
        assert_eq!(431.9057573, (standard_deviation_two * 1e7).round() / 1e7);
    }


    #[test]
    fn test_type_one_stats_with_weights() {
        let occ_data_tuple = get_occ_data();
        let summaries = get_summary_data(&occ_data_tuple.0);

        let mut summary_statistics = SummaryStatistics::new(1);

        for summary in summaries {
            summary_statistics.ingest_summary(summary);
        }

        let period_weights_reference = get_period_weights();

        let (type_one_ni, standard_deviation) = summary_statistics.calculate_type_one_stats(
            occ_data_tuple.1, Some(&period_weights_reference.weights)
        );

        // Type	Mean	Standard deviation
        // 1	917.805	351.6125866
        // 2	917.8006	426.9855306

        assert_eq!(917.8049896, (type_one_ni * 1e7).round() / 1e7);
        assert_eq!(351.6125589, (standard_deviation * 1e7).round() / 1e7);
    }

    #[test]
    fn test_type_two_stats_with_weights() {
        let occ_data_tuple = get_occ_data();
        let summaries = get_summary_data(&occ_data_tuple.0);

        let mut summary_statistics = SummaryStatistics::new(1);

        for summary in summaries {
            summary_statistics.ingest_summary(summary);
        }

        let period_weights_reference = get_period_weights();

        let (type_one_ni, standard_deviation) = summary_statistics.calculate_type_two_stats(
            occ_data_tuple.1, Some(&period_weights_reference.weights)
        );

        assert_eq!(917.8005939, (type_one_ni * 1e7).round() / 1e7);
        assert_eq!(426.9855213, (standard_deviation * 1e7).round() / 1e7);
    }

}