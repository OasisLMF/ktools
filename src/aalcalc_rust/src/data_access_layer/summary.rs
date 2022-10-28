use byteorder::{ByteOrder, LittleEndian};
use std::{fs::File, io::Read};


/// Holds ```sidx``` and losses for an event.
/// 
/// # Fields
/// * event_id: the ID of the event
/// * losses: vector of tuples (sidx, loss)
#[derive(Debug, Clone)]
pub struct Event {
    pub event_id: i32,
    pub losses: Vec<(i32, f32)>
}

impl Event {

    /// Adds a loss to the ```self.losses```.
    /// 
    /// # Arguments
    /// * sidx: the sample ID of the loss
    /// * loss: the total amount of the loss
    /// 
    /// # Returns
    /// ```true```: loss added and should continue to add losses
    /// ```false```: the end of event stream has been reached
    pub fn add_loss(&mut self, sidx: &[u8], loss: &[u8]) -> bool {
        let sidx_int = LittleEndian::read_i32(sidx);
        let loss_float = LittleEndian::read_f32(loss);

        if sidx_int == 0 && loss_float == 0.0 {
            return false
        }
        self.losses.push((sidx_int, loss_float));
        return true
    }

}


/// Houses the data around a summary. 
/// 
/// # Fields
/// * event_id: the ID of the event that the summary belongs to
/// * summary_id: the ID of the summary
/// * exposure_value: don't know will need to be filled in
/// * events: the events belonging to the summary
/// 
/// # Constructing from bytes 
/// The summary can be instructed from bytes by using the following code:
/// 
/// ```rust
/// let mut file = File::open("path/to/bin/file.bin").unwrap();
/// let mut meta_read_frame = [0; 12];
/// 
/// file.read_exact(&mut meta_read_frame)
/// let mut chunked_meta_frame = meta_read_frame.chunks(4).into_iter();
/// 
/// let mut summary = Summary::from_bytes(
///                         chunked_meta_frame.next().unwrap(), 
///                         chunked_meta_frame.next().unwrap(), 
///                         chunked_meta_frame.next().unwrap());
///                
/// ```
#[derive(Debug, Clone)]
pub struct Summary {
    pub event_id: i32,
    pub summary_id: i32,
    pub exposure_value: i32,
    pub events: Vec<Event>
}

impl Summary {

    /// Constructs the ```Summary``` struct using bytes
    /// 
    /// # Arguments
    /// * event_id: the ID of the event that the summary belongs to
    /// * summary_id: the ID of the summary
    /// * exposure_value: don't know will need to be filled in
    pub fn from_bytes(event_id: &[u8], summary_id: &[u8], exposure_value: &[u8]) -> Self {
        let events: Vec<Event> = vec![];
        return Summary { 
            event_id: LittleEndian::read_i32(event_id), 
            summary_id: LittleEndian::read_i32(summary_id), 
            exposure_value: LittleEndian::read_i32(exposure_value), 
            events: events 
        }
    }
}


/// Houses meta data around the loading and handling of summary data. 
/// 
/// # Fields 
/// * handler: handles the reading and writing of a binary file
/// * stream_id: the ID of the stream for the summary 
/// * no_of_samples: The number of samples in the summary
/// * summary_set: don't know will need to be filled in
#[derive(Debug)]
pub struct SummaryData {
    pub handler: File,
    pub stream_id: i32,
    pub no_of_samples: i32,
    pub summary_set: i32
}

impl SummaryData {

    /// The constructor for the SummaryData struct.
    /// 
    /// # Fields
    /// * path: the path to the binary file that is going to read
    /// 
    /// # Returns 
    /// The constructed ```SummaryData``` struct
    pub fn new(path: String) -> Self {
        let mut file = File::open(path).unwrap();
        let mut num_buffer = [0; 4];

        file.read_exact(&mut num_buffer).unwrap();
        let stream_id = LittleEndian::read_i32(&num_buffer);
        file.read_exact(&mut num_buffer).unwrap();
        let no_of_samples = LittleEndian::read_i32(&num_buffer);
        file.read_exact(&mut num_buffer).unwrap();
        let summary_set = LittleEndian::read_i32(&num_buffer);

        return SummaryData {
            stream_id,
            no_of_samples,
            summary_set,
            handler: file
        }
    }

    /// Gets all the summaries and events belonging to the summary in the binary file attached to the ```SummaryData``` struct.
    /// 
    /// # Returns
    /// all the summaries in the binary file attached to the ```SummaryData``` struct
    pub fn get_data(&mut self) -> Vec<Summary> {
        let mut meta_read_frame = [0; 12];
        let mut event_read_frame = [0; 8];
        let mut buffer = vec![];

        loop {
            // gets new summary
            match self.handler.read_exact(&mut meta_read_frame) {
                Ok(_) => {
                    let mut chunked_meta_frame = meta_read_frame.chunks(4).into_iter();

                    let mut summary = Summary::from_bytes(
                        chunked_meta_frame.next().unwrap(), 
                        chunked_meta_frame.next().unwrap(), 
                        chunked_meta_frame.next().unwrap()
                    );

                    let mut event = Event{event_id: summary.event_id.clone(), losses: vec![]};

                    // loop through adding the losses to the event
                    loop {
                        match self.handler.read_exact(&mut event_read_frame) {
                            Ok(_) => {
                                let mut chunked_event_read_frame = event_read_frame.chunks(4).into_iter();
                                let sidx = chunked_event_read_frame.next().unwrap();
                                let loss = chunked_event_read_frame.next().unwrap();
                                let pushed = event.add_loss(sidx, loss);
                                if pushed == false {
                                    summary.events.push(event);
                                    break
                                }
                            },
                            Err(error) => {
                                panic!("{}", error);
                            }
                        }
                    }
                    buffer.push(summary);
                },
                Err(error) => {
                    if error.to_string().as_str() != "failed to fill whole buffer" {
                        println!("{}", error);
                    }
                    break
                }
            };
            
        };
        return buffer
    }
}


#[cfg(test)]
mod summary_data_tests {

    use super::{SummaryData};
    use tokio;

    #[tokio::test]
    async fn test_new() {
        let sum_data = SummaryData::new(String::from("./work/summary_aal/summary_1.bin"));
        assert_eq!(50331649, sum_data.stream_id);
        assert_eq!(10, sum_data.no_of_samples);
        assert_eq!(1, sum_data.summary_set);
    }

    #[tokio::test]
    async fn test_get_data() {
        let mut sum_data = SummaryData::new(String::from("./work/summary_aal/summary_1.bin"));
        let summaries = sum_data.get_data();

        let first_summary = summaries[0].clone();

        assert_eq!(1, first_summary.event_id);
        assert_eq!(1, first_summary.summary_id);
        assert_eq!(1240736768, first_summary.exposure_value);

        let events = first_summary.events[0].clone();

        assert_eq!(12, events.losses.len());
    }

}
