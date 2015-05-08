# Reference Model

This section provides an explanation of the reference model, which is an implementation of each of the components in the framework. 

The set of core components provided in this release is as follows;
* **eve** is the event distributing utility. Based on the number of events in the input and the number of processes specified as a parameter, eve outputs subsets of the events as a stream. The output streams into getmodel.
* **getmodel** generates a stream of effective damageability cdfs for the input stream of events. The reference example reads in Oasis format damage cdf data from binary file. getmodel streams into gulcalc or can be output to a binary file.
* **gulcalc** performs the ground up loss sampling calculations and numerical integration. The output is the Oasis kernel gul sample table. This can be output to a binary file or streamed into  fmcalc or outputcalc.
* **fmcalc** performs the insured loss calculations on the gul samples and the mean ground up loss. The output is the Oasis format loss sample table. The functionality covered in fmcalc is the same as the current financial Module in Oasis R1.4 (see R1.2 Financial Module documentation for more information).  The result can be output to a binary file or streamed into outputcalc.
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
Note that no examples of the component which generates the binary files have been provided in the tool set 
as yet. This component is, in general, specific to the technical environment and may also be calling 
the data from a remote server as an API. However an example of 'gendata' is provided with Oasis R1.4 
in a github project called 'oatools' which reads the input data from a SQL Server database.
```

The following sections explain the usage and internal processes and data requirements of each of the reference components. The standard input and output data streams for the components are generic and are covered in the Specification.

#### getmodel

getmodel generates a stream of effective damageability distributions (cdfs) from an input list of events. Specifically, it reads pre-generated Oasis format cdfs and converts them into a binary stream. The source input data must have been generated as binary files by a separate program.

This is reference example of the class of programs which generates the damage distributions for an event set and streams them into memory. It is envisaged that model developers who wish to use the toolkit as a back-end calculator of their existing platforms can write their own version of getmodel, reading in their own source data and converting it into the standard output stream. As long as the standard input and output structures are adhered to, the program can be written in any language and read any input data.

##### Stream_id

| Byte 1 | Bytes 2-4 |  Description                                   |
|:-------|-----------|:-----------------------------------------------|
|    0   |     1     |  getmodel reference example                    |

##### Parameters
The single parameter is chunk_id (int). 

##### Usage
```
$ [stdin component] | getmodel [parameters] | [stout component]
$ [stdin component] | getmodel [parameters] > [stdout].bin
$ getmodel [parameters] < [stdin].bin > [stdout].bin
```

##### Example
```
$ eve 1 1 1 | getmodel 1 | gulcalc -C1 -S100
$ eve 1 1 1 | getmodel 1 > cdf_chunk1.bin
$ getmodel 1 < e_chunk1_data.bin > cdf_chunk1.bin 
```

##### Internal data
The program requires the damage bin dictionary for the model, the Oasis damage cdf in chunks, and an index file for each cdf chunk as binary files. The files are picked up from the directory where the program is invoked and have the following filenames;
* damage_bin_dictionary.bin

And in a cdf specific subdirectory;
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

##### Calculation
The program reads the damage bin mid-point (interpolation field) from the damage bin dictionary and includes it as a new field in the CDF stream as 'bin_mean'.  This field is the conditional mean damage for the bin and it is used to facilitate the calculation of mean and standard deviation in the gulcalc component. No calculations are performed except to construct the standard output stream.

#### gulcalc
The gulcalc program performs Monte Carlo sampling of ground up loss and calculates mean and standard deviation by numerical integration of the cdfs. The sampling methodology of Oasis classic has been extended beyond linear interpolation to include bin value sampling and quadratic interpolation. This supports damage bin intervals which represent a single discrete damage value, and damage distributions with cdfs that are described by a piecewise quadratic function. 

##### Stream_id

| Byte 1 | Bytes 2-4 |  Description                                   |
|:-------|-----------|:-----------------------------------------------|
|    1   |     1     |  gulcalc reference example                     |

##### Parameters
The parameters are;
* -C chunk_id
* -S number of samples
* -R reconciliation mode (optional)
* -r read random numbers from file (optional)

##### Usage
```
$ [stdin component] | gulcalc [parameters] | [stout component]
$ [stdin component] | gulcalc [parameters] > [stdout].bin
$ gulcalc [parameters] < [stdin].bin > [stdout].bin
```

##### Example
```
$ eve 1 1 1 | getmodel 1 | gulcalc -C1 -S100 | fmcalc
$ eve 1 1 1 | getmodel 1 | gulcalc -C1 -S100 > gul_chunk1.bin
$ gulcalc -C1 -S100 < cdf_chunk1.bin > gul_chunk1.bin 
```

##### Internal data
The program requires the damage bin dictionary for the model and the exposure instance table, both as binary files. The files are picked up from the directory where the program is invoked and have the following filenames;
* damage_bin_dictionary.bin
* exposures.bin

If the user specifies -r as a parameter, then the program also picks up a random number file from the working directory. The filename is;
random_{chunk_id}.bin

The data structure of damage_bin_dictionary.bin is the same as for the getmodel component given above, and the data structure for the exposure instance is as follows;

| Name              | Type   |  Bytes | Description                                                   | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------|------------:|
| item_id           | int    |    4   | Identifier of the exposure item                               |     1       |
| areaperil_id      | int    |    4   | Identifier of the locator and peril of the item               |   4545      |
| vulnerability_id  | int    |    4   | Identifier of the vulnerability distribution of the item      |   345456    |
| tiv               | float  |    4   | The total insured value of the item                           |   200000    |
| group_id          | int    |    4   | Identifier of the correlation group of the item               |    1        |   

The structure of the random number table is as follows;

| Name              | Type   |  Bytes | Description                                                   | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------|------------:|
| rand              | float  |    4   | Random number between 0 and 1                                 |  0.75875    |

##### Calculation
The program constructs a cdf for each item, based on matching the areaperil_id and vulnerability_id from the stdin and the exposure data. The stdin stream is a block of cdfs which are ordered by event.

For each item cdf and for the number of samples specified, the program reads a random number from the random number file and uses it to sample ground up loss from the cdf using one of three methods. If a random number file is not provided, a random number is generated on the fly for each event and group of items which have a common group_id using the Mersenne twister psuedo random number generator (the default RNG of the C++ v11 compiler).

For a given damage interval corresponding to a cumulative probability interval that each random number falls within;
* If the conditional mean damage (of the cdf) is the mid-point of the damage bin interval (of the damage bin dictionary) then the gulcalc program performs linear interpolation. 
* If the conditional mean damage is equal to the lower and upper damage threshold of the damage bin interval (i.e the bin represents a damage value, not a range) then that value is sampled.
* Else, the gulcalc program performs quadrative interpolation.

An example of the three cases and methods is given below;
 
| bin_from | bin_to |  bin_mean | Method used             |
|:---------|--------|-----------| :-----------------------|
|    0.1   |  0.2   |    0.15   | Linear interpolation    |
|    0.1   |  0.1   |    0.1    | Sample bin value        |
|    0.1   |  0.2   |    0.14   | Quadratic interpolation |

Each sampled damage is multiplied by the item TIV and output to the stdout stream.

A second calculation which occurs in the gulcalc program is of the mean and standard deviation of ground up loss. For each cdf, the mean and standard deviation of damage is calculated by numerical integration and the result is multiplied by the item TIV. The results are output to the stdout stream as IDX=0 (mean) and IDX=-1 (standard deviation), for each item and event.

#### fmcalc
fmcalc is the in-memory implementation of the Oasis Financial Module. It applies policy terms and conditions to the ground up losses and produces insured loss sample output.

##### Stream_id

| Byte 1 | Bytes 2-4 |  Description                                   |
|:-------|-----------|:-----------------------------------------------|
|    2   |     1     |  fmcalc reference example                      |

##### Parameters
There are no parameters as all of the information is taken from the gul stdout stream and internal data.

##### Usage
```
$ [stdin component] | fmcalc | [stout component]
$ [stdin component] | fmcalc > [stdout].bin
$ fmcalc < [stdin].bin > [stdout].bin
```

##### Example
```
$ eve 1 1 1 | getmodel 1 | gulcalc -C1 -S100 | fmcalc | outputcalc > output.csv
$ eve 1 1 1 | getmodel 1 | gulcalc -C1 -S100 | fmcalc > fm_chunk1.bin
$ fmcalc < gul_chunk1.bin > fm_chunk1.bin 
```

##### Internal data
The program requires the FM Instance data, which is the Oasis native format data tables which describe an insurance programme. These four tables have been combined into one which has the following data structure;

| Name                     | Type   |  Bytes | Description                                    | Example     |
|:-------------------------|--------|--------| :----------------------------------------------|------------:|
| item_id                  | int    |    4   | Identifier of the exposure item                |    56745    |
| agg_id                   | int    |    4   | Oasis Financial Module agg_id                  |     546     |
| prog_id                  | int    |    4   | Oasis Financial Module prog_id                 |     4       |
| level_id                 | int    |    4   | Oasis Financial Module level_id                |     1       |
| policytc_id              | int    |    4   | Oasis Financial Module policytc_id             |     34      |
| layer_id                 | int    |    4   | Oasis Financial Module layer_id                |      1      |
| calcrule_id              | int    |    4   | Oasis Financial Module calcrule_id             |      2      |
| allocrule_id             | int    |    4   | Oasis Financial Module allocrule_id            |      0      |
| deductible               | float  |    4   | Deductible                                     |   50        |
| limit                    | float  |    4   | Limit                                          |   100000    |
| share_prop_of_lim        | float  |    4   | Share/participation as a proportion of limit   |   0.25      |
| deductible_prop_of_loss  | float  |    4   | Deductible as a proportion of loss             |   0.05      |
| limit_prop_of_loss       | float  |    4   | Limit as a proportion of loss                  |   0.5       |
| deductible_prop_of_tiv   | float  |    4   | Deductible as a proportion of TIV              |   0.05      |
| limit_prop_of_tiv        | float  |    4   | Limit as a proportion of TIV                   |   0.8       |
| deductible_prop_of_limit | float  |    4   | Deductible as a proportion of limit            |   0.1       |


##### Calculation
fmcalc performs the same calculations as the Oasis Financial Module in R1.4.  Information about the Oasis FinancialModule can be found on the public area of the Oasis Loss Modelling Framework website, and detailed information and examples are available to Oasis community members in the members area.

#### outputcalc
The reference example of an output produces an event loss table 'ELT' for either ground up loss or insured losses.

##### Stream_id

There is no output stream_id, the results table is exported directly to csv.

##### Parameters
There are no parameters as all of the information is taken from the input stream and internal data.

##### Usage
Either gulcalc or fmcalc stdout can be input streams to outputcalc
```
$ [stdin component] | outputcalc | [output].csv
$ outputcalc < [stdin].bin > [output].csv
```

##### Example
Either gulcalc or fmcalc stdout streams can be input streams to outputcalc. For example;
```
$ eve 1 1 1 | getmodel 1 | gulcalc -C1 -S100 | outputcalc > output.csv
$ eve 1 1 1 | getmodel 1 | gulcalc -C1 -S100 | fmcalc | outputcalc > output.csv
$ outputcalc < gul.bin > output.csv
$ outputcalc < fm.bin > output.csv
```

##### Internal data
The program requires the exposure instance table as a binary file, and for fmcalc input it requires an additional cross reference file to relate the output_id from the fm stdout stream (which represents an abstract grouping of exposure items) back to the original item_id. This file is picked up from the fm subdirectory;

fm/fmxref.bin

The data structure of exposures.bin is given in the gulcalc section above, and the data structure of fmxref.bin is as follows;

| Name                        | Type   |  Bytes | Description                                    | Example     |
|:----------------------------|--------|--------| :----------------------------------------------|------------:|
| item_id                     | int    |    4   | Identifier of the exposure item                |    56745    |
| output_id                   | int    |    4   | Oasis Financial Module output_id               |     546     |

##### Calculation
The program sums the sampled losses from either gulcalc stream or fmstream across the portfolio/programme by event and sample, and then computes sample mean and standard deviation. It reads the TIVs from the exposure instance table and sums them for the group of items affected by each event.

[Go to Planned work](PlannedWork.md)

[Back to Contents](Contents.md)
