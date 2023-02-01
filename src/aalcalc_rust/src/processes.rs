//! Isolated funtions to help calculate statistics or alter collections of data.
use std::collections::HashMap;
use glob::glob;

type TotalMap = HashMap<i32, f64>;


/// Calculates the standard deviation for type one statistics.
///
/// # Arguments
/// * ni_loss_map: ni_loss_map: total losses multiplied by occurrence of events mapped by period number
/// * n: number of periods which is the ```period_number``` of the constructed ```OccurrenceData```
///
/// # Returns
/// the calculated standard deviation
pub fn calculate_standard_deviation(ni_loss_map: &TotalMap, n: i32) -> f64 {
    let mut sum_squared = 0.0;
    let mut sum = 0.0;

    // square and sum period losses to the ni
    for key in ni_loss_map.keys() {
        let loss = ni_loss_map.get(&key).unwrap();
        sum += loss.clone();
        sum_squared += loss * loss;
    }
    sum = sum * sum;

    let alpha = sum_squared - (sum / n as f64);
    let beta = alpha / (n - 1) as f64;

    return f64::sqrt(beta)
}


/// Calculates the standard deviation for type two statistics.
///
/// # Arguments
/// * periods: a vector of total losses which is the length of the sample size which can
/// be accessed using the period number as the key.
/// * n: number of periods which is the ```period_number``` of the constructed ```OccurrenceData```
///
/// # Returns
/// the calculated standard deviation
pub fn calculate_st_deviation_two(periods: &HashMap<i32, Vec<f64>>, n: i32) -> f64 {
    let mut sum_squared = 0.0;
    let mut sum = 0.0;

    for (_, data) in periods {
        for loss in data {
            sum += loss.clone();
            sum_squared += loss * loss;
        }
    }
    sum = sum * sum;

    let alpha = sum_squared - (sum / n as f64);
    let beta = alpha / (n - 1) as f64;
    return f64::sqrt(beta)
}


/// Adds two vectors together elementwise.
///
/// # Arguments
/// * one: the vector that will have the elements added to it
/// * two: the vector to supply the elements to be added to ```one```
pub fn add_two_vectors(one: &mut Vec<f64>, two: &Vec<f64>) {
    for i in 0..one.len() {
        one[i as usize] += two[i as usize];
    }
}


/// Gets all the files in a directory based on a file pattern.
///
/// # Arguments
/// * file_pattern: the path to the directory and the file pattern such as
/// ```"./work/summary_aal_two/*.bin"```
///
/// # Returns
/// all the paths to all the files matching the pattern passed into the function
pub fn get_all_binary_file_paths(file_pattern: String) -> Vec<String> {
    let mut files: Vec<String> = Vec::new();
    let paths = glob(file_pattern.as_str()).unwrap();
    for path in paths {
        let inner_path = path.unwrap().into_os_string();
        files.push(inner_path.into_string().unwrap());
    }
    return files
}
