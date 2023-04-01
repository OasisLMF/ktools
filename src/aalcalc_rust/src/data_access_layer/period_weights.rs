//! This defines the structure that loads period weights from a binary file.
//! Period weights are used to apply weights to the total loss of the period for a summary.
use byteorder::{ByteOrder, LittleEndian};
use std::{fs::File, io::Read};

use crate::data_access_layer::traits::load_period_weights::{
    ReadPeriodWeights,
    IntoDummyWeights
};



pub struct PeriodWeights {
    pub weights: Vec<f64>,
}


/// A struct to hold the period weights
///
/// # Fields
/// * **weights** - A vector of f64 values containing the period weights where the index of the
/// vector is the period number.
pub struct PeriodWeightsHandle {
    pub file: File
}

impl PeriodWeightsHandle {

    pub fn new(path: String) -> Result<Self, String> {
        match File::open(path) {
            Ok(file) => Ok(Self {file}),
            Err(e) => Err(e.to_string())
        }
    }
}


impl ReadPeriodWeights<f64> for PeriodWeightsHandle {

    /// Create a new PeriodWeights struct from a period weights binary file.
    ///
    /// # Arguments
    /// * **path** - A String containing the path to the period weights file
    ///
    /// # Returns
    /// * A PeriodWeights struct
    fn get_data(self) -> Vec<f64> {
        // read the data from the file
        let mut file = self.file;
        let mut data: Vec<u8> = Vec::new();
        file.read_to_end(&mut data).unwrap();

        // define the meta data for looping through the data file
        let end = data.len();
        let total_chunks = (end / 12) as i32;
        let total_number_weight = total_chunks as f64;
        let mut start = 0;
        let mut finish = 4;
        let mut buffer = vec![0.0; total_chunks as usize];

        // loop through the file data to
        while finish <= end {
            let period_number = LittleEndian::read_i32(&data[start..finish]);
            start += 4;
            finish += 8;
            let period_weight = LittleEndian::read_f64(&data[start..finish]);
            start += 8;
            finish += 4;
            buffer[(period_number - 1) as usize] = period_weight * total_number_weight;
        }
        return buffer
    }
}

impl IntoDummyWeights<Vec<f64>> for PeriodWeightsHandle {

    /// Converts all the weights in the vector to 1.0.
    ///
    /// # Arguments
    /// * **input_vec** - A vector of f64 weights to be converted to 1.0
    ///
    /// # Returns
    /// * A vector of f64 weights with all values set to 1.0
    fn as_dummy(input_vec: Vec<f64>) -> Vec<f64> {
        let ones = vec![1.0; input_vec.len()];
        return ones
    }
}



// create tests for PeriodWeights
#[cfg(test)]
mod period_weights_test {
    use super::*;
    use mockall::predicate::*;
    use mockall::mock;

    #[test]
    fn test_get_weights() {

        mock! {
            PeriodWeightsHandle {}
            impl ReadPeriodWeights<f64> for PeriodWeightsHandle {
                fn get_data(self) -> Vec<f64>;
            }
        }

        let mut mock = MockPeriodWeightsHandle::new();
        mock.expect_get_data().returning(|| {vec![0.1, 0.3, 0.4, 0.5]});

        let data = mock.get_data();
        assert_eq!(0.1, data[0]);
        assert_eq!(0.3, data[1]);
        assert_eq!(0.4, data[2]);
        assert_eq!(0.5, data[3]);

        let ones = PeriodWeightsHandle::as_dummy(data);

        assert_eq!(1.0, ones[0]);
        assert_eq!(1.0, ones[1]);
        assert_eq!(1.0, ones[2]);
        assert_eq!(1.0, ones[3]);
    }
}
