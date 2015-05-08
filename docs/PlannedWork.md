# Planned work

#### 1. Generate data components
Reference examples of components that generate the input binaries will be provided.  These will convert csv files into binaries to help users generate their own input binary data.

#### 2. Remove chunk concept from eve and getmodel
The input data for the reference components eve and getmodel can be split across several files, where each is identified by a chunk_id under a fixed naming convention. Eve and getmodel have 'chunk_id' as an input parameter which identifies the relevant input binary file. 

Because chunk is an Oasis mid-tier concept, the chunk parameter will be removed and each internal data input to the ktools reference model will be a single consolidated file.  

Note that the events can still be chunked and each chunk distributed to a separate back-end running the in-memory kernel.

#### 3. Multi-output workflows
Currently if the results of the intermediate calculation steps are required to be persisted, then they must be written out to disk and read back into memory to continue downstream processing.
##### Figure 1. Multiple output file processing - now
![alt text](https://github.com/OasisLMF/ktools/blob/master/docs/img/MultipleOutput1.jpg "Multiple output file processing")

The plan is to enable intermediate calculation steps to be written out to disk whilst continuing the in-memory workflow.
##### Figure 2. Multiple output file processing - future
![alt text](https://github.com/OasisLMF/ktools/blob/master/docs/img/MultipleOutput2.jpg "Multiple output file processing")

#### 4. Sidx field format in gulcalc and fmcalc
The sample index field will be simplified to be a simple 4 byte integer format, with no higher byte special meaning. The sample index value for the mean will be changed to -1 (currently 0) and -2 for standard deviation (currently -1).

[Back to Contents](Contents.md)
