//! This defines the structure that loads period weights from a binary file.
//! Period weights are used to apply weights to the total loss of the period for a summary.
use byteorder::{ByteOrder, LittleEndian};
use std::{fs::File, io::Read};


/// A struct to hold the period weights
///
/// # Fields
/// * **weights** - A vector of f64 values containing the period weights where the index of the
/// vector is the period number.
pub struct PeriodWeights {
    pub weights: Vec<f64>,
}


impl PeriodWeights {

    /// Create a new PeriodWeights struct from a period weights binary file.
    ///
    /// # Arguments
    /// * **path** - A String containing the path to the period weights file
    ///
    /// # Returns
    /// * A PeriodWeights struct
    pub fn new(path: String) -> PeriodWeights {
        // read the data from the file
        let mut file = File::open(path).unwrap();
        let mut data: Vec<u8> = Vec::new();
        file.read_to_end(&mut data).unwrap();

        // define the meta data for looping through the data file
        let end = data.len();
        let total_chunks = (end / 12) as i32;
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
            buffer[(period_number - 1) as usize] = period_weight;
        }
        PeriodWeights {
            weights: buffer
        }
    }

    pub fn as_dummy(self) -> Self {
        let ones = vec![1.0; self.weights.len()];
        return Self {weights: ones}
    }
}



// create tests for PeriodWeights
#[cfg(test)]
mod period_weights_test {
    use super::*;

    #[test]
    fn test_new() {
        let path = "./input/periods.bin".to_string();
        let test = PeriodWeights::new(path);
        assert_eq!(0.0006261203011901157, test.weights[3]);

    }
}
