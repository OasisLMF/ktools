


pub trait ReadSummaryData {

    fn get_data(&self, path: String) -> Result<Vec<u8>, std::io::Error>;

}


pub trait ExtractSummaryData {

    fn extract_summary_header(&mut self) -> (i32, i32, i32);

    fn extract_file_header(&mut self) -> (i32, i32, i32);

    fn extract_loss(&mut self) -> (i32, f64);

}
