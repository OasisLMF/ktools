//! defines the trait that loads period weights.


pub trait ReadPeriodWeights<T> {

    fn get_data(self) -> Vec<T>;

}


pub trait IntoDummyWeights<T> {

    fn as_dummy(input_vec: T) -> T;

}
