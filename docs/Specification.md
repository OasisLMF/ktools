# Specification <a id="specification"></a>

### Introduction

This section specifies the components and data stream structures in the in-memory kernel.
These components are;

* **[eve](#eve)**
* **[getmodel](#getmodel)**
* **[gulcalc](#gulcalc)**
* **[fmcalc](#fmcalc)**
* **[outputcalc](#outputcalc)**

Most components have a standard input (stdin) and output (stdout) data stream structure. These data structures are not defined explicitly as meta data in the code as they would be in a database language, and they have been designed to minimise the volume flowing through the pipeline. For example, indexes which are common to a block of data are defined as a header record and then only the variable data records that are relevant to the header key are part of the data stream. The names of the data fields given below are unimportant, only their position in the data stream in order to perform the calculations defined in the program.

#### Stream types

The architecture supports multiple stream types. Therefore a developer can define a new type of data stream within the framework by specifying a unique stream_id of the stdout of one or more of the components, or even write a new component which performs an intermediate calculation between the existing components.

The stream_id is the first 4 byte header of the stdout streams. The higher byte is reserved to identify the type of stream, and the 2nd to 4th bytes hold the identifier of the stream.

The current reserved values are as follows;

Higher byte;

| Byte 1 |  Stream type   |
|:-------|:---------------|
|    0   | getmodel       |
|    1   | gulcalc        |
|    2   | fmcalc         |

Reserved stream_ids;

| Byte 1 | Bytes 2-4 |  Description                                                        		 |
|:-------|-----------|:--------------------------------------------------------------------------|
|    0   |     1     |  getmodel - Reference example for Oasis format CDF output                 |
|    1   |     1     |  gulcalc - Reference example for Oasis format ground up loss sample output|
|    2   |     1     |  fmcalc - Reference example for Oasis format insured loss sample output   |

The final calculation component, outputcalc, has no stream_id as it outputs results directly to csv.

There are rules about which stream types can be accepted as inputs to the components. These are;
* gulcalc can only take stream type 0 (getmodel standard output) as input
* fmcalc can only take stream type 1 (gulcalc standard output) as input
* outputcalc can take either stream type 1 (gulcalc standard output) or 2 (fmcalc standard output) as input

<a id="eve"></a>
## eve 

eve is an 'event emitter' and its job is to read a list of events from file and send out a subset of events as a binary data stream. It has no standard input. 
eve is used to partition lists of events such that a workflow can be distributed across multiple processes.

##### Output

Data packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| event_id          | int    |    4   | Oasis event_id                                                      |   4545      |

Note that eve has no stream_id header.

[Return to top](#specification)

<a id="getmodel"></a>
## getmodel 

getmodel is the component which generates a stream of cdfs for a given set of event_ids. 

##### Input
Same as eve output or a binary file of the same input format can be piped into getmodel.

##### Output
Header packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| stream_id         | int    |   1/3  | Identifier of the data stream type.                                 |    0/1      |
| event_id          | int    |    4   | Oasis event_id                                                      |   4545      |
| areaperil_id      | int    |    4   | Oasis areaperil_id                                                  |  345456     |
| vulnerability_id  | int    |    4   | Oasis vulnerability_id                                              |   345       |
| no_of_bins        | int    |    4   | Number of records (bins) in the data package                        |    20       |        
Data packet structure (record repeated no_of_bin times)

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| prob_to           | float  |    4   | The cumulative probability at the upper damage bin threshold        |     0.765   |
| bin_mean          | float  |    4   | The conditional mean of the damage bin                              |     0.45    |

[Return to top](#specification)

<a id="gulcalc"></a>
## gulcalc 

gulcalc is the component which calculates ground up loss. It takes the cdfs as standard input and based on the sampling parameters specified, performs Monte Carlo sampling and numerical integration. The output is a table of ground up loss samples in Oasis kernel format, with mean (sidx=0) and standard deviation (sidx=-1).

##### Input
Same as getmodel output or a binary file of the same data structure can be piped into gulcalc.

##### Output
Stream header packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| stream_id         | int    |   1/3  | Identifier of the data stream type.                                 |    1/1      |
| no_of_samples     | int    |   4    | Number of samples                                                   |    100      |

Gul header packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| event_id          | int    |    4   | Oasis event_id                                                      |   4545      |
| item_id           | int    |    4   | Oasis item_id                                                       |    300      |

Gul data packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| sidx              | int    |    4   | Sample index                                                        |    10       |
| gul               | float  |    4   | The ground up loss for the sample                                   | 5675.675    |

The data packet may be a variable length and so an sidx of 0 identifies the end of the data packet.

There are two values of sidx with special meaning as follows;

| sidx   |  Meaning                                 |
|:-------|:-----------------------------------------|
|   -1   | numerical integration mean               |
|   -2   | numerical integration standard deviation |

[Return to top](#specification)

## fmcalc <a id="fmcalc"></a>

fmcalc is the component which takes the gulcalc output stream as standard input and applies the policy terms and conditions to produce insured loss samples. The output is a table of insured loss samples in Oasis kernel format, including the insured loss for the mean ground up loss (sidx=-1).

##### Input
Same as gulcalc output or a binary file of the same data structure can be piped into fmcalc.

##### Output
Stream Header packet structure

| Name              | Type   |  Bytes | Description                          | Example     |
|:------------------|--------|--------| :------------------------------------|------------:|
| stream_id         | int    |   1/3  | Identifier of the data stream type.  |    2/1      |
| no_of_samples     | int    |   4    | Number of samples                    |    100      |

fmcalc header packet structure

| Name              | Type   |  Bytes | Description                          | Example     |
|:------------------|--------|--------| :------------------------------------|------------:|
| event_id          | int    |    4   | Oasis event_id                       |   4545      |
| prog_id           | int    |    4   | Oasis prog_id                        |    300      |
| layer_id          | int    |    4   | Oasis layer_id                       |    300      |
| output_id         | int    |    4   | Oasis output_id                      |    300      |

fmcalc data packet structure

| Name              | Type   |  Bytes | Description                          | Example     |
|:------------------|--------|--------| :------------------------------------|------------:|
| sidx              | int    |    4   | Sample index                         |   10        |
| loss              | float  |    4   | The insured loss for the sample      | 5625.675    |

The data packet may be a variable length and so a sidx of 0 identifies the end of the data packet.

The sidx field is the same as the sidx in the gul stdout stream. 

[Return to top](#specification)

## outputcalc <a id="outputcalc"></a>

outputcalc is the component which performs results analysis such as an event loss table or EP curve on the sampled output from either the gulcalc or fmcalc program.  The output is a results table in csv format.

##### Input
gulcalc stdout or fmcalc stdout. Binary files of the same data structures can be piped into outputcalc.

##### Output
No standard output stream. The results table is exported to a csv file. See the [Reference model](ReferenceModel.md) for example output.

[Return to top](#specification)

[Go to Reference model](ReferenceModel.md)

[Back to Contents](Contents.md)
