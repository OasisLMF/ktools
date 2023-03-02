//! This tool is responsible for calculating the average annual losses.
//!
//! # Running
//! You can run the program with the following command:
//! ```bash
//! aalcalc -k summary_aal
//! ```
//!
mod data_access_layer;
mod processes;
mod collections;

use data_access_layer::occurrence::OccurrenceData;
use data_access_layer::period_weights::PeriodWeights;
use data_access_layer::summary::{SummaryData};
use collections::summary_statistics::SummaryStatistics;
use processes::{get_all_binary_file_paths, add_two_vectors};

use std::collections::HashMap;
use std::mem::drop;
use std::path::Path;

use clap::Parser;


#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
    #[arg(short, long)]
    k: String,
}


fn main() {
    let args = Args::parse();

    let pwd = std::env::current_dir().unwrap().into_os_string().into_string().unwrap();
    let occurrence_path = format!("{}/input/occurrence.bin", pwd);
    let period_weights_path = format!("{}/input/periods.bin", pwd);

    // get data around the occurrences
    let mut occ_data = OccurrenceData::new(occurrence_path);
    let raw_data = occ_data.get_data();
    let occurrences = raw_data;
    let number_of_periods = occ_data.period_number;

    // define map for summary statistics
    let mut summary_map: HashMap<i32, SummaryStatistics> = HashMap::new();

    // get all files in directory
    let file_pattern = format!("{}/work/{}/*.bin", pwd, args.k);
    let files = get_all_binary_file_paths(file_pattern);

    // run the processes in parallel
    let _ = files.iter().map(|i| {

        // load all the data from the file
        let mut sum_data = SummaryData::new(i.clone());
        let vec_capacity = sum_data.no_of_samples;
        let summaries = sum_data.get_data(&occurrences, vec_capacity);
        drop(sum_data);

        for summary in summaries {

            // extract the summary statistics if it exists, crease one if not
            let mut summary_statistics: &mut SummaryStatistics;
            match summary_map.get_mut(&summary.summary_id) {
                Some(statistics) => {
                    summary_statistics = statistics;
                },
                None => {
                    let statistics = SummaryStatistics::new(*&summary.summary_id.clone());
                    summary_map.insert(summary.summary_id.clone(), statistics);
                    summary_statistics = summary_map.get_mut(&summary.summary_id).unwrap();
                }
            }
            // update the summary statistics with the data from the summary
            summary_statistics.sample_size = vec_capacity;
            summary_statistics.squared_total_loss += summary.squared_total_loss;
            summary_statistics.total_loss += summary.total_loss;
            summary_statistics.ni_loss_squared += summary.ni_loss_squared;
            summary_statistics.ni_loss += summary.ni_loss;

            for (key, value) in summary.period_categories {
                match summary_statistics.period_categories.get_mut(&key) {
                    Some(data) => {
                        add_two_vectors(data, &value);
                    },
                    None => {
                        summary_statistics.period_categories.insert(key, value);
                    }
                }
            }

            for (key, value) in summary.ni_loss_map {
                match summary_statistics.ni_loss_map.get_mut(&key) {
                    Some(data) => {
                        *data += value;
                    },
                    None => {
                        summary_statistics.ni_loss_map.insert(key, value);
                    }
                }
            }
        }
        return 1
    }).collect::<Vec<i32>>();

    // order the summary IDs for printing out
    let mut summary_ids: Vec<&i32> = summary_map.keys().collect();
    summary_ids.sort();
    
    let period_weights: Option<&Vec<f64>>;
    let period_weights_loader: Option<PeriodWeights>;

    if Path::new(&period_weights_path).exists() == true {
        println!("period weights are firing");
        period_weights_loader = Some(PeriodWeights::new(period_weights_path));
    }
    else {
        period_weights_loader = None;
    }
    match &period_weights_loader {
        Some(period_weights_reference) => {
            period_weights = Some(&period_weights_reference.weights);
        }, 
        None => {
            period_weights = None;
        }
    }

    println!("summary_id,type,mean,standard_deviation");

    // print out summary statistics
    for i in &summary_ids {
        let sum_stats = &summary_map.get(i).unwrap();
        sum_stats.print_type_one_stats(number_of_periods, period_weights);
    }

    for i in &summary_ids {
        let sum_stats = &summary_map.get(i).unwrap();
        sum_stats.print_type_two_stats(number_of_periods, period_weights);
    }
}
