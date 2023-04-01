//! Defines the tratis for data access for Occurrences.
use std::collections::HashMap;


pub trait IngestOccurrence<T, Y> {

    fn insert_occurrence(map: &mut HashMap<T, Vec<Y>>, occurrence: Y);

}


pub trait ReadOccurrences<T, Y, X, Z> {

    fn get_data(self) -> HashMap<T, Vec<Y>>;

    fn get_meta_data(&mut self) -> Z;

}


