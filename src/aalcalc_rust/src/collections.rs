use std::collections::HashMap;

use super::processes::{calculate_standard_deviation, calculate_st_deviation_two};


#[derive(Debug)]
pub struct SummaryStatistics {
    pub summary_id: i32,
    pub ni_loss: f32,
    pub ni_loss_squared: f32,
    pub sample_size: i32,
    pub total_loss: f32,
    pub squared_total_loss: f32,
    pub ni_loss_map: HashMap<i32, f32>,
    pub period_categories: HashMap<i32, Vec<f32>>
}

impl SummaryStatistics {

    pub fn new(summary_id: i32) -> Self {
        let ni_loss_map: HashMap<i32, f32> = HashMap::new();
        let period_categories: HashMap<i32, Vec<f32>> = HashMap::new();
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

    pub fn print_type_one_stats(&self, number_of_periods: i32) {
        let type_one_ni = self.ni_loss / number_of_periods as f32;
        let standard_deviation = calculate_standard_deviation(&self.ni_loss_map, number_of_periods);
        println!("{},1,{},{}", self.summary_id, type_one_ni, standard_deviation);
    }

    pub fn print_type_two_stats(&self, number_of_periods: i32) {
        // println!("number of total loss: {}", self.total_loss);
        let type_two_sample = self.total_loss / (number_of_periods * number_of_periods) as f32;
        // println!("{} {}", number_of_periods, self.sample_size);
        let standard_deviation_two = calculate_st_deviation_two(&self.period_categories, number_of_periods * self.sample_size);
        // let standard_deviation_two = calculate_st_deviation_two(&self.period_categories, 100);
        println!("{},2,{},{}", self.summary_id, type_two_sample, standard_deviation_two);
    }

}
