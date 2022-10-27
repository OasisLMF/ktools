// use byteorder::{LittleEndian, ReadBytesExt}; // 1.2.7
use std::{
    // fs::File,
    io::{self, Read},
};
use tokio::{fs::File, io::AsyncReadExt};
mod data_access_layer;

use data_access_layer::occurrence::OccurrenceData;


#[tokio::main]
async fn main() {
    let outcome = std::fs::read("./input/occurrence.bin").unwrap();
    let mut file = File::open("./input/occurrence.bin").await.unwrap();
    let mut buffer = vec![0; 8];
    // let another_test = file.read_exact(buf)
    let test = file.read_exact(&mut buffer).await.unwrap();

    let occ_data = OccurrenceData::new(String::from("./input/occurrence.bin")).await;

    println!("{}", occ_data.period_number);
    println!("{}", occ_data.chunk_size);
    println!("{:?}", occ_data.date_format);
    // println!("{:?}", outcome[4:]);
}
