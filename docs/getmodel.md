# Reference Model

This section provides an explanation of the reference model of the SDK, which is an implementation of each of the components in the framework. 

The set of core components provided in this release is as follows;
* **eve** is the event distributing utility. Based on the number of events in the input and the number of processes specified as a parameter, eve outputs subsets of the events as a stream. The output streams into getmodel.
* **getmodel** generates a stream of effective damageability cdfs for the input stream of events. The reference example reads in Oasis format damage cdf data from binary file. getmodel streams into gulcalc or can be output to a binary file.
* **gulcalc** performs the ground up loss sampling calculations and numerical integration. The output is the Oasis kernel gul sample table. This can be output to a binary file or streamed into  fmcalc or outputcalc.
* **fmcalc** performs the insured loss calculations on the gul samples. The output is the Oasis format loss sample table. The reference example applies coverage deductibles and then aggregates losses before applying a single policy deductible, limit and share. Finally, it back-allocates losses to the coverages. The result can be output to a binary file or streamed into outputcalc.
* **outputcalc** performs an output analysis on the gul or loss samples. The reference example is an event loss table containing TIV, sample mean and standard deviation for each event at portfolio/programme summary level. The results are written directly into csv file as there is no downstream processing.

In addition, some components which convert the binary output of each calculation step to csv format are provided.
* **cdftocsv** is a utility to convert binary format CDFs to a csv. getmodel standard output can be streamed directly into cdftocsv, or a binary file of the same format can be input.
* **gultocsv** is a utility to convert binary format GULs to a csv. gulcalc standard output can be streamed directly into gultocsv, or a binary file of the same format can be input.
* **fmtocsv** is a utility to convert binary format losses to a csv. fmcalc standard output can be streamed directly into fmtocsv, or a binary file of the same format can be input.

Figure 1 shows the data stream workflow of the reference model with its particular internal data files.

##### Figure 1. Reference Model Workflow
![alt text](https://github.com/OasisLMF/ktools/blob/master/docs/img/KtoolsWorkflow.jpg "Reference Model Workflow")

The input data for the reference components, shown as red source files, are the events, Damage CDFs, Exposure Instance, Damage Bin Dictionary and FM Instance.  These are Oasis concepts with Oasis format data as outlined below. 

Figure 2 shows the workflows for the data conversion components.

##### Figure 2. Data Conversion Workflows
![alt text](https://github.com/OasisLMF/ktools/blob/master/docs/img/Dbtools2.jpg "Data Conversion Workflows")

```
Note that no examples of the component which generates the binary files have been provided in the SDK
as yet. This component is, in general, specific to the technical environment and may also be calling 
the data from a remote server as an API. However an example of 'gendata' is provided with Oasis R1.4 
which reads the input data from a SQL Server database.
```

The following sections explain the usage and internal processes and data requirements of each of the reference components. The standard input and output data streams for the components are generic and are covered in the Specification.

#### getmodel

getmodel generates a stream of effective damageability distributions (cdfs) from an input list of events. Specifically, it reads pre-generated Oasis format cdfs and converts them into a binary stream. The source input data must have been generated as binary files by a separate program.

This is reference example of the class of programs which generates the damage distributions for an event set and streams them into memory. It is envisaged that model developers who wish to use the SDK as a back-end calculator of their existing platforms can write their own version of getmodel, reading in their own source data and converting it into the standard output stream. As long as the standard input and output structures are adhered to, the program can be written in any langauge and read any input data.

##### Stream_id

| Byte 1 | Bytes 2-4 |  Description                                   |
|:-------|-----------|:-----------------------------------------------|
|    0   |     1     |  getmodel reference example                    |

##### Parameters
The single parameter is chunk_id (int). 

##### Usage
```
$ [stdin] | getmodel [chunk_id] | [stout]
$ [stdin] | getmodel 1 > [filename].bin
```

##### Example
```
$ e_chunk1_data.bin | getmodel 1 | cdf_chunk1_data.bin
```

##### Internal data
The program requires the damage bin dictionary for the model, the Oasis damage cdf in chunks, and an index file for each cdf chunk as binary files. These are currently hardcoded filenames found in the directory where the program is invoked;
* damage_bin_dictionary.bin
and in a cdf specific subdirectory
* cdf/damage_cdf_chunk_{chunk_id}.bin
* cdf/damage_cdf_chunk_{chunk_id}.idx

The data structure of damage_bin_dictionary.bin is as follows;

| Name              | Type   |  Bytes | Description                                                   | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------|------------:|
| bin_index         | int    |    4   | Identifier of the damage bin                                  |     1       |
| bin_from          | float  |    4   | Lower damage threshold for the bin                            |   0.01      |
| bin_to            | float  |    4   | Upper damage threshold for the bin                            |   0.02      |
| interpolation     | float  |    4   | Interpolation damage value for the bin (usually the mid-point)|   0.015     |
| interval_type     | int    |    4   | Identifier of the interval type, e.g. closed, open            |    1201     |   

The data structure of damage_cdf_chunk_{chunk_id}.bin is as follows;

| Name              | Type   |  Bytes | Description                                                   | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------|------------:|
| event_id          | int    |    4   | Oasis event_id                                                |     1       |
| areaperil_id      | int    |    4   | Oasis areaperil_id                                            |   4545      |
| vulnerability_id  | int    |    4   | Oasis vulnerability_id                                        |   345456    |
| bin_index         | int    |    4   | Identifier of the damage bin                                  |     10      |
| prob_to           | float  |    4   | The cumulative probability at the upper damage bin threshold  |    0.765    | 

The cdfs are ordered by event and streamed out in blocks for each event.  The index file damage_cdf_chunk_{chunk_id}.bin contains the position of each new event_id in the stream, for faster processing.

Note that the prob_from field from the existing database table structure of Oasis damage cdfs has been dropped to minimise the size of the table, as it is implied from the prior record prob_to field.

The program reads the damage bin mid-point (interpolation field) from the damage bin dictionary and includes it as a new field in the CDF stream as 'bin_mean'.  This field is the conditional mean damage for the bin and it is used to facilitate the calculation of mean and standard deviation in the gulcalc component.

##### Calculation
No calculations are performed except to read the relevant columns from the input files and to construct the standard output stream.


* If the conditional mean is the mid-point of the bin then the gulcalc program performs linear interpolation. 
* If the conditional mean is equal to the lower and upper damage threshold of the bin (i.e the bin represents a damage value, not a range) then that value is always sampled.
* Else, the gulcalc performs quadrative interpolation.
 
