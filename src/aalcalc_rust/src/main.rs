mod data_access_layer;

use data_access_layer::occurrence::OccurrenceData;


#[tokio::main]
async fn main() {
    let occ_data = OccurrenceData::new(String::from("./input/occurrence.bin")).await;
    println!("{}", occ_data.period_number);
    println!("{}", occ_data.chunk_size);
    println!("{:?}", occ_data.date_format);
}
