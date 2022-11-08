use std::collections::HashMap;


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

}
