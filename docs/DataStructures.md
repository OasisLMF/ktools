# Data Structures

### Introduction

This section outlines the data structures of the components of ktools.

Most components in the toolkit have a standard input (stdin) and output (stdout) data stream structure. These data structures are not defined explicitly as meta data in the code as they would be in a database language, and they have been designed to minimise the volume flowing through the pipeline. For example, indexes which are common to a block of data are defined as a header record and then only the variable data records that are relevant to the header key are part of the data stream. The names of the data fields given below are unimportant, only their positions in the data stream matters.

#### eve

Eve is an 'event emitter' and its job is to read a list of events from file and send out a subset of events as a binary data stream. 

##### Input
* Format: binary file with filename e_chunk_{chunk_id}_data.bin
* Data: event_id as 4 byte int.

##### Output
Data packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|-------------|
| event_id          | int    |    4   | Oasis event_id                                                      |   4545      |

#### getmodel

getmodel is the reference example of generating a stream of cdfs for a given set of event ids. The cdfs are read from binary file (it is assumed the required data has already been read from a database or csv file by a separate utility).

##### Input
Same as eve output or a binary file of the same input format can be piped into getmodel

##### Output
Header packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|-------------|
| stream type       | int    |    4   | Identifier of the data stream structure. 1 is Oasis                 |     1       |
| event_id          | int    |    4   | Oasis event_id                                                      |   4545      |
| areaperil_id      | int    |    4   | Oasis areaperil_id                                                  |  345456     |
| vulnerability_id  | int    |    4   | Oasis vulnerability_id                                              |   345       |
| no_of_bins        | int    |    4   | Number of records (bins) in the data package                        |    20       |        |
Data packet structure (record repeated no_of_bin times)

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|-------------|
| prob_to           | float  |    4   | The cumulative probability at the upper damage bin threshold        |     0.765   |
| bin_mean          | float  |    4   | The conditional mean of the damage bin                              |     0.45    |

#### gulcalc

gulcalc takes the cdfs as standard input and based on the sampling parameters specified, performs Monte Carlo sampling and numerical integration. The output is a table of ground up loss samples, mean (IDX=0) and standard deviation (IDX=-1).

##### Input
Same as getmodel output or a binary file of the same format can be piped into gulcalc.

##### Output
Header packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|-------------|
| event_id          | int    |    4   | Oasis event_id                                                      |   4545      |
| item_id           | int    |    4   | Oasis item_id                                                       |    300      |

Data packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|-------------|
| idx               | int    |    4   | Sample index                                                        |     10      |
| gul               | float  |    4   | The conditional mean of the damage bin                              | 5675.675    |

#### cdftocsv

Converts the cdf binary file or stream to a csv file.

##### Input
Same as getmodel output or a binary file of the same format can be piped into cdftocsv

##### Output
Csv file with the following fields;

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|-------------|
| event_id          | int    |    4   | Oasis event_id                                                      |   4545      |
| areaperil_id      | int    |    4   | Oasis areaperil_id                                                  |  345456     |
| vulnerability_id  | int    |    4   | Oasis vulnerability_id                                              |   345       |
| bin_index         | int    |    4   | Damage bin index                                                    |    20       | 
| prob_to           | float  |    4   | The cumulative probability at the upper damage bin threshold        |   0.765     |
| bin_mean          | float  |    4   | The conditional mean of the damage bin                              |   0.45      |

#### gultocsv

Converts the gul binary file or stream to a csv file.

##### Input
Same as gulcalc output or a binary file of the same format can be piped into gultocsv.

##### Output
Csv file with the following fields;
| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|-------------|
| event_id          | int    |    4   | Oasis event_id                                                      |   4545      |
| item_id           | int    |    4   | Oasis item_id                                                       |    300      |
| idx               | int    |    4   | Sample index                                                        |     10      |
| gul               | float  |    4   | The ground up loss value                                            | 5675.675    |
