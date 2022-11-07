use std::collections::HashMap;

type TotalMap = HashMap<i32, f32>;


pub fn calculate_standard_deviation(ni_loss_map: &TotalMap, n: i32) -> f32 {
    let mut sum_squared = 0.0;
    let mut sum = 0.0;

    // square and sum period losses to the ni
    for key in ni_loss_map.keys() {
        let loss = ni_loss_map.get(&key).unwrap();
        sum += loss.clone();
        sum_squared += loss * loss;
    }
    sum = sum * sum;

    let alpha = sum_squared - (sum / n as f32);
    let beta = alpha / (n - 1) as f32;

    return f32::sqrt(beta)
}


pub fn calculate_st_deviation_two(periods: &HashMap<i32, Vec<f32>>, n: i32) -> f32 {
    let mut sum_squared = 0.0;
    let mut sum = 0.0;

    for (_, data) in periods {
        // println!("{:?} {:?}", key, data);
        for loss in data {
            sum += loss.clone();
            sum_squared += loss * loss;
        }
    }
    sum = sum * sum;

    let alpha = sum_squared - (sum / n as f32);
    let beta = alpha / (n - 1) as f32;

    return f32::sqrt(beta)
}
