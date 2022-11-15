use std::{fs::File, io::Read};
use byteorder::{ByteOrder, LittleEndian};
use std::cmp::PartialEq;
use std::collections::HashMap;


/// The types of date formats that are supported when reading occurrence binary files.
#[derive(Debug, PartialEq, Eq)]
pub enum DateFormat {
    NewFormat,
    IncludesHoursAndMinutes,
    Depreciated
}


/// An occurrence of an event.
/// 
/// # Fields
/// * event_id: the ID of the event that the occurrence belongs to
/// * period_num: the period bin that the occurrence belongs to
/// * occ_date_id: the ID of the occurrence date
#[derive(Debug)]
pub struct Occurrence {
    pub event_id: i32,
    pub period_num: i32,
    pub occ_date_id: i32
}


impl Occurrence {

    /// Constructs the ```Occurrence``` struct from bytes. 
    /// 
    /// # Fields
    /// * event_id: the ID of the event that the occurrence belongs to
    /// * period_num: the period bin that the occurrence belongs to
    /// * occ_date_id: the ID of the occurrence date
    /// 
    /// # Returns
    /// the constructed ```Occurrence``` struct
    pub fn from_bytes(event_id: &[u8], period_num: &[u8], occ_date_id: &[u8]) -> Self {
        return Occurrence {
            event_id: LittleEndian::read_i32(event_id),
            period_num: LittleEndian::read_i32(period_num),
            occ_date_id: LittleEndian::read_i32(occ_date_id),
        }
    }
}


/// Holds the meta data around an occurrence binary file. 
/// 
/// # Fields 
/// * date_format: the format of the dates in the file
/// * period_number: the period bin that the occurrence belongs to
/// * handler: handles the reading and writing of the binary file
/// * chunk_size: the number of bytes each occurrence takes (subject to change based on date format)
#[derive(Debug)]
pub struct OccurrenceData {
    pub date_format: DateFormat,
    pub period_number: i32,
    pub handler: File,
    pub chunk_size: i32
}


impl OccurrenceData {

    /// The constructor for the ```OccurrenceData``` struct. 
    /// 
    /// # Arguments
    /// * path: the path to the binary file that is going to read
    /// 
    /// # Returns 
    /// The constructed ```OccurrenceData``` struct
    pub fn new(path: String) -> Self {
        let mut file = File::open(path).unwrap();

        let mut date_option_buffer = [0; 4];
        let mut period_number_buffer = [0; 4];

        file.read_exact(&mut date_option_buffer).unwrap();
        file.read_exact(&mut period_number_buffer).unwrap();

        let date_format: DateFormat;
        match LittleEndian::read_i32(&date_option_buffer) {
            1 => {
                date_format = DateFormat::NewFormat;    
            },
            3 => {
                date_format = DateFormat::IncludesHoursAndMinutes;
            },
            0 => {
                date_format = DateFormat::Depreciated;
            },
            _ => {
                panic!("occurrence bin file only supports 0, 1, and 3 in header for date option type");
            }
        }

        let period_number = LittleEndian::read_i32(&period_number_buffer);

        let chunk_size: i32;
        match &date_format {
            &DateFormat::IncludesHoursAndMinutes => {
                chunk_size = 16;
            },
            _ => {
                chunk_size = 12;
            }
        }

        return OccurrenceData{
            date_format,
            period_number,
            handler: file,
            chunk_size
        }
    }

    /// Gets all the occurrences and events belonging to the occurrences in the binary file attached to the ```OccurrenceData``` struct.
    /// 
    /// # Returns
    /// all the occurrences in the binary file attached to the ```OccurrenceData``` struct
    pub fn get_data(&mut self) -> HashMap<i32, Vec<Occurrence>> {
        let mut read_frame = [0; 12];
        let mut data = HashMap::new();

        loop {
            match self.handler.read_exact(&mut read_frame) {
                Ok(_) => {
                    let mut chunked_frame = read_frame.chunks(4).into_iter();
                    let occurrence = Occurrence::from_bytes(
                        chunked_frame.next().unwrap(), 
                        chunked_frame.next().unwrap(), 
                        chunked_frame.next().unwrap()
                    );
                    OccurrenceData::insert_occurrence(&mut data, occurrence);

                },
                Err(error) => {
                    if error.to_string().as_str() != "failed to fill whole buffer" {
                        panic!("{}", error);
                    }
                    break
                }
            }
        }
        return data
    }

    /// Inserts an occurrence into the map. 
    /// 
    /// # Fields
    /// map: the map in which the occurrence is going to be inserted into
    /// occurrence: the occurrence that is going to be inserted into the map
    /// 
    /// # Returns 
    /// the map with the inserted occurrence
    fn insert_occurrence(map: &mut HashMap<i32, Vec<Occurrence>>, occurrence: Occurrence) {
        // get reference to occurence based on and have a hashmap with the key of the period.
        match map.get_mut(&occurrence.event_id) {
            Some(data) => {
                data.push(occurrence);
            }, 
            None => {
                let mut data = vec![];
                let key = occurrence.event_id.clone();
                data.push(occurrence);
                map.insert(key, data);
            }
        }
    }

}


// #[cfg(test)]
// mod occurrence_data_tests {
//
//     use super::{OccurrenceData, DateFormat};
//     use tokio;
//
//     #[tokio::test]
//     async fn test_new() {
//
//         let occ_data = OccurrenceData::new(String::from("./input/occurrence.bin")).await;
//         assert_eq!(10, occ_data.period_number);
//         assert_eq!(12, occ_data.chunk_size);
//         assert_eq!(DateFormat::NewFormat, occ_data.date_format);
//     }
//
//     #[tokio::test]
//     async fn test_get_data() {
//         let mut occ_data = OccurrenceData::new(String::from("./input/occurrence.bin")).await;
//         let data = occ_data.get_data().await;
//         println!("{:?}", data);
//         assert_eq!(2, data.get(&1).unwrap().len());
//         assert_eq!(2, data.get(&2).unwrap().len());
//     }
//
// }