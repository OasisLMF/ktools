use tokio::{fs::File, io::AsyncReadExt};
use byteorder::{ByteOrder, LittleEndian};
use std::cmp::PartialEq;


#[derive(Clone, Debug, PartialOrd, PartialEq, Eq, Ord)]
pub enum DateFormat {
    NewFormat,
    IncludesHoursAndMinutes,
    Depreciated
}


pub struct Occurence {
    pub event_id: i32,
    pub period_num: i32,
    pub occ_date_id: i32
}


// impl Occurence {

//     pub async fn new(buffer: [u8; 12]) -> Self {

//     }

// }


pub struct OccurenceData {
    pub date_format: DateFormat,
    pub period_number: i32,
    pub handler: File,
    pub chunk_size: i32
}


impl OccurenceData {

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
                panic!("occurence bin file only supports 0, 1, and 3 in header for date option type");
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

        return OccurenceData{
            date_format,
            period_number,
            handler: file,
            chunk_size
        }
    }

    pub async fn get_data(&mut self) {
        let mut buf: Vec<u8> = Vec::new();
        let mut buffer = [0; 4];
        self.handler.read_exact(&mut buffer);

        // for chunk in buf.into_iter() {
        //     // Check that the sum of each chunk is 4.
        //     println!("{:?}", chunk);
        // }
        println!("{:?}", buffer);
        self.handler.read_exact(&mut buffer);
        println!("{:?}", buffer);
        println!("the test has finished");
        // return buffer
    }

}


#[cfg(test)]
mod occurrence_data_tests {

    use super::{OccurenceData, DateFormat};
    use tokio;

    use tokio::{fs::File, io::AsyncReadExt};
    use byteorder::{ByteOrder, LittleEndian};
    use std::cmp::PartialEq;

    #[tokio::test]
    async fn test_new() {

        let occ_data = OccurenceData::new(String::from("./input/occurrence.bin")).await;
        assert_eq!(10, occ_data.period_number);
        assert_eq!(12, occ_data.chunk_size);
        assert_eq!(DateFormat::NewFormat, occ_data.date_format);
    }


    #[tokio::test]
    async fn test_get_data() {

        let mut occ_data = OccurenceData::new(String::from("./input/occurrence.bin")).await;
        occ_data.get_data().await;
        // let mut file = File::open("./input/occurrence.bin").await.unwrap();
        // let mut buf: Vec<u8> = Vec::new();
        // let mut bytes = file.read(&mut buf).await.unwrap();

        // for chunk in buf.into_iter() {
        //     // Check that the sum of each chunk is 4.
        //     println!("{:?}", chunk);
        // }
        // println!("{:?}", buf);
        // println!("the test has finished");
    }

}