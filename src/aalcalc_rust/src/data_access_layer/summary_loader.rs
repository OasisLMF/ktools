use std::{fs::File, io::Read};
use byteorder::{ByteOrder, LittleEndian};

use crate::data_access_layer::traits::load_summary_data::{
    ReadSummaryData,
    ExtractSummaryData
};


pub struct SummaryLoader {
    pub start: usize,
    pub finish: usize,
    pub end: usize,
    pub data: Vec<u8>
}


impl SummaryLoader {

    pub fn new(path: String, handle: &mut dyn ReadSummaryData) -> Result<Self, std::io::Error> {
        let data = handle.get_data(path)?;
        Ok(SummaryLoader {
            start: 0,
            finish: 4,
            end: data.len(),
            data
        })
    }

}


pub struct SummaryLoaderHandle{}


impl ReadSummaryData for SummaryLoaderHandle {

    fn get_data(&self, path: String) -> Result<Vec<u8>, std::io::Error> {
        let mut file = File::open(path.to_string())?;
        let mut data: Vec<u8> = Vec::new();
        file.read_to_end(&mut data)?;
        Ok(data)
    }

}



impl ExtractSummaryData for SummaryLoader {

    fn extract_file_header(&mut self) -> (i32, i32, i32) {
        let raw_stream_id = &self.data[self.start..self.finish];
        self.start += 4;
        self.finish += 4;
        let raw_no_of_samples = &self.data[self.start..self.finish];
        self.start += 4;
        self.finish += 4;
        let raw_summary_set = &self.data[self.start..self.finish];
        self.start += 4;
        self.finish += 4;
        (
            LittleEndian::read_i32(raw_stream_id),
            LittleEndian::read_i32(raw_no_of_samples),
            LittleEndian::read_i32(raw_summary_set)
        )
    }

    fn extract_summary_header(&mut self) -> (i32, i32, i32) {
        let event_id = &self.data[self.start..self.finish];
        self.start += 4;
        self.finish += 4;
        let summary_id = &self.data[self.start..self.finish];
        self.start += 4;
        self.finish += 4;
        let exposure_value = &self.data[self.start..self.finish];
        self.start += 4;
        self.finish += 4;
        (
            LittleEndian::read_i32(event_id),
            LittleEndian::read_i32(summary_id),
            LittleEndian::read_i32(exposure_value)
        )
    }

    fn extract_loss(&mut self) -> (i32, f64) {
        let sidx = &self.data[self.start..self.finish];
        self.start += 4;
        self.finish += 4;
        let loss = &self.data[self.start..self.finish];
        self.start += 4;
        self.finish += 4;
        (
            LittleEndian::read_i32(sidx),
            LittleEndian::read_f32(loss) as f64
        )
    }
}


#[cfg(test)]
mod summary_loader_tests {

    use super::*;
    use mockall::predicate::*;
    use mockall::mock;
    use std::io::Error;


    fn get_fake_data() -> Vec<u8> {
        let data: Vec<u8> = vec![
            0x01, 0x00, 0x00, 0x00, // stream id = 1
            0x0A, 0x00, 0x00, 0x00, // number of samples = 10
            0x02, 0x00, 0x00, 0x00, // summary set = 2
            0x00, 0x01, 0x02, 0x03, // additional data
        ];
        return data
    } 


    #[test]
    fn test_summary_loader_new() {
        mock! {
            SummaryLoaderHandle{}
            impl ReadSummaryData for SummaryLoaderHandle {
                fn get_data(&self, path: String) -> Result<Vec<u8>, Error>;
            }
        }
        let mut mock_summary_loader = MockSummaryLoaderHandle::new();
        let path = "test.txt";
        let path_ref = path.clone();

        mock_summary_loader
            .expect_get_data()
            .with(eq(path_ref.to_string()))
            .returning(|_| Ok(vec![1, 2, 3]));

        let result = SummaryLoader::new(path.to_string(), &mut mock_summary_loader);
        let summary = result.unwrap();
        assert_eq!(summary.data, vec![1, 2, 3]);
        assert_eq!(summary.start, 0);
        assert_eq!(summary.finish, 4);
        assert_eq!(summary.end, 3);
    }


    #[test]
    fn test_extract_file_data() {


        let data = get_fake_data();

        let mut summary = SummaryLoader {
            start: 0,
            finish: 4,
            end: 16,
            data: data
        };
        let result = summary.extract_file_header();
        assert_eq!(result.0, 1);
        assert_eq!(result.1, 10);
        assert_eq!(result.2, 2);
        assert_eq!(summary.start, 12);
        assert_eq!(summary.finish, 16);
    }

    #[test]
    fn test_extract_summary_header() {
        let data = get_fake_data();

        let mut summary = SummaryLoader {
            start: 0,
            finish: 4,
            end: 16,
            data: data
        };
        let result = summary.extract_summary_header();
        assert_eq!(result.0, 1);
        assert_eq!(result.1, 10);
        assert_eq!(result.2, 2);
        assert_eq!(summary.start, 12);
        assert_eq!(summary.finish, 16);
    }

    #[test]
    fn test_extract_loss() {
        let data: Vec<u8> = vec![
            0x01, 0x00, 0x00, 0x00, // sidx = 1
            0xCD, 0xCC, 0xCC, 0x3D, // loss = 0.1
            0x00, 0x01, 0x02, 0x03, // additional data
        ];

        let mut summary = SummaryLoader {
            start: 0,
            finish: 4,
            end: 16,
            data: data
        };

        let result = summary.extract_loss();
        let rounded_loss = (result.1 * 10.0).round() / 10.0;
        assert_eq!(result.0, 1);
        assert_eq!(rounded_loss, 0.1);
    }

}
