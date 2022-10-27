use tokio::{fs::File, io::AsyncReadExt};
use byteorder::{ByteOrder, LittleEndian};
use std::cmp::PartialEq;
use std::collections::HashMap;


#[derive(Debug, PartialEq, Eq)]
pub enum DateFormat {
    NewFormat,
    IncludesHoursAndMinutes,
    Depreciated
}


#[derive(Debug)]
pub struct Occurrence {
    pub event_id: i32,
    pub period_num: i32,
    pub occ_date_id: i32
}


impl Occurrence {

    pub fn from_bytes(event_id: &[u8], period_num: &[u8], occ_date_id: &[u8]) -> Self {
        return Occurrence {
            event_id: LittleEndian::read_i32(event_id),
            period_num: LittleEndian::read_i32(period_num),
            occ_date_id: LittleEndian::read_i32(occ_date_id),
        }
    }

}


pub struct OccurrenceData {
    pub date_format: DateFormat,
    pub period_number: i32,
    pub handler: File,
    pub chunk_size: i32
}


impl OccurrenceData {

    pub async fn new(path: String) -> Self {
        let mut file = File::open(path).await.unwrap();

        let mut date_option_buffer = [0; 4];
        let mut period_number_buffer = [0; 4];

        file.read_exact(&mut date_option_buffer).await.unwrap();
        file.read_exact(&mut period_number_buffer).await.unwrap();

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

    pub async fn get_data(&mut self) -> HashMap<i32, Vec<Occurrence>> {
        let mut read_frame = [0; 12];
        let mut data = HashMap::new();

        loop {
            match self.handler.read_exact(&mut read_frame).await {
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
                    if error.to_string().as_str() != "early eof" {
                        panic!("{}", error);
                    }
                    break
                }
            }
        }
        return data
    }

    fn insert_occurrence(map: &mut HashMap<i32, Vec<Occurrence>>, occurrence: Occurrence) {
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


#[cfg(test)]
mod occurrence_data_tests {

    use super::{OccurrenceData, DateFormat};
    use tokio;

    #[tokio::test]
    async fn test_new() {

        let occ_data = OccurrenceData::new(String::from("./input/occurrence.bin")).await;
        assert_eq!(10, occ_data.period_number);
        assert_eq!(12, occ_data.chunk_size);
        assert_eq!(DateFormat::NewFormat, occ_data.date_format);
    }


    #[tokio::test]
    async fn test_get_data() {

        let mut occ_data = OccurrenceData::new(String::from("./input/occurrence.bin")).await;
        occ_data.get_data().await;
    }

}