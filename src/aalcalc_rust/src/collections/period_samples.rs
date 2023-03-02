use crate::processes::add_two_vectors;


pub struct PeriodSamples {
    data: Vec<Vec<f64>>
}


impl PeriodSamples {

    pub fn new(number_of_periods: i32, number_of_samples: i32) -> Self {
        PeriodSamples {
            data: vec![vec![0.0; number_of_samples as usize]; number_of_periods as usize]
        }
    }

    pub fn add_sample_loss(&mut self, period_number: &i32, sample_number: &i32, loss_total: f64) {
        let period_index = (period_number - 1) as usize;
        let sample_index = (sample_number - 1) as usize;
        self.data[period_index][sample_index] += loss_total;
    }

    pub fn add_event_losses(&mut self, event_period_samples: Self) {
        for i in 0..self.data.len() as i32 {
            add_two_vectors(
                &mut self.data[i as usize], &event_period_samples.data[i as usize]
            ); 
        }
    }

}


// build tests module 
#[cfg(test)]
mod period_samples_test {
    use super::*;

    #[test]
    fn test_new() {
        let number_of_periods = 3;
        let number_of_samples = 2;
        let period_samples = PeriodSamples::new(number_of_periods, number_of_samples);
        assert_eq!(period_samples.data.len(), 3);
        assert_eq!(period_samples.data[0].len(), 2);
        assert_eq!(period_samples.data[1].len(), 2);
        assert_eq!(period_samples.data[2].len(), 2);
    }

    #[test]
    fn test_add_sample_loss() {
        let number_of_periods = 3;
        let number_of_samples = 2;
        let mut period_samples = PeriodSamples::new(number_of_periods, number_of_samples);
        let period_number = 1;
        let sample_number = 1;
        let loss_total = 1.0;
        period_samples.add_sample_loss(&period_number, &sample_number, loss_total);
        assert_eq!(period_samples.data[0][0], 1.0);
    }

    #[test]
    fn test_add_event_losses() {
        let number_of_periods = 3;
        let number_of_samples = 2;
        let mut period_samples = PeriodSamples::new(number_of_periods, number_of_samples);
        let mut event_period_samples = PeriodSamples::new(number_of_periods, number_of_samples);
        let period_number = 1;
        let sample_number = 1;
        let loss_total = 1.0;
        period_samples.add_sample_loss(&period_number, &sample_number, loss_total);
        event_period_samples.add_sample_loss(&period_number, &sample_number, loss_total);
        period_samples.add_event_losses(event_period_samples);
        println!("{:?}", period_samples.data);
        assert_eq!(period_samples.data[0][0], 2.0);
    }

}