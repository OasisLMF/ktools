![alt text](../img/banner.jpg "banner")
# 3. Specification <a id="specification"></a>

### Introduction

This section specifies the data stream structures and core components in the in-memory kernel.

The data stream structures are;
* **[cdf stream](#cdf)**
* **[gulcalc stream](#gulstream)**
* **[loss stream](#loss)**
* **[summary stream](#summary)**

The stream data structures have been designed to minimise the volume flowing through the pipeline, using data packet 'headers' to remove redundant data. For example, indexes which are common to a block of data are defined as a header record and then only the variable data records that are relevant to the header key are part of the data stream. The names of the data fields given below are unimportant, only their position in the data stream is important in order to perform the calculations defined in the program.

The components are;

* **[eve](#eve)**
* **[getmodel](#getmodel)**
* **[gulcalc](#gulcalc)**
* **[fmcalc](#fmcalc)**
* **[summarycalc](#summarycalc)**
* **[outputcalc](#outputcalc)**


The components have a standard input (stdin) and/or output (stdout) data stream structure. eve is the stream-initiating component which only has a standard output stream, whereas "outputcalc" (a generic name representing an extendible family of output calculation components) is a stream-terminating component with only a standard input stream.

An implementation of each of the above components is provided in the [Reference Model](ReferenceModelOverview.md), where usage instructions and command line parameters are provided. A functional overview is given below.

#### Stream types

The architecture supports multiple stream types. Therefore a developer can define a new type of data stream within the framework by specifying a unique stream_id of the stdout of one or more of the components, or even write a new component which performs an intermediate calculation between the existing components.

The stream_id is the first 4 byte header of the stdout streams. The higher byte is reserved to identify the type of stream, and the 2nd to 4th bytes hold the identifier of the stream. This is used for validation of pipeline commands to report errors if the components are not being used in the correct order.

The current reserved values are as follows;

Higher byte;

| Byte 1 |  Stream name  |
|:-------|:--------------|
|    0   | cdf           |
|    1   | gulcalc       |
|    2   | loss          |
|    3   | summary       |

Reserved stream_ids;

| Byte 1   | Bytes 2-4    |  Description                                                             		         
|:---------|--------------|:-------------------------------------------------------------------------------------|
|    0     |     1        |  cdf - Oasis format effective damageability CDF output                               |
|    1     |     1        |  gulcalc -  Oasis format item level ground up loss sample output **(deprecated)**    |
|    1     |     2        |  gulcalc - Oasis format coverage level ground up loss sample output **(deprecated)** |
|    2     |     1        |  loss -  Oasis format loss sample output (any loss perspective)                      |
|    3     |     1        |  summary - Oasis format summary level loss sample output                             |

The supported standard input and output streams of the Reference model components are summarized here;

| Component    | Standard input                        |  Standard output                      | Stream option parameters          |
|:-------------|:--------------------------------------|:--------------------------------------|:----------------------------------|
| getmodel     | none                                  | 0/1 cdf                               | none                              |
| gulcalc      | 0/1 cdf                               | 1/1 gulcalc item **(deprecated)**     | -i                                |
| gulcalc      | 0/1 cdf                               | 1/2 gulcalc coverage **(deprecated)** | -c                                |
| gulcalc      | 0/1 cdf                               | 2/1 loss                              | -i -a{}                           |
| fmcalc       | 1/1 gulcalc item  **(deprecated)**    | 2/1 loss                              | none                              |
| fmcalc       | 2/1 loss                              | 2/1 loss                              | none                              |
| summarycalc  | 1/2 gulcalc coverage  **(deprecated)**| 3/1 summary                           | -g                                | 
| summarycalc  | 2/1 loss                              | 3/1 summary                           | -i gulcalc input, -f fmcalc input | 
| outputcalc   | 3/1 summary                           | none                                  | none                              | 


## Stream structure

<a id="cdf"></a>
### cdf stream

Stream header packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| stream_id         | int    |   1/3  | Identifier of the data stream type.                                 |    0/1      |

Data header packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
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

<a id="gulstream"></a>
### gulcalc item stream (deprecated)

Stream header packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| stream_id         | int    |   1/3  | Identifier of the data stream type.                                 |    1/1      |
| no_of_samples     | int    |   4    | Number of samples                                                   |    100      |

Data header packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| event_id          | int    |    4   | Oasis event_id                                                      |   4545      |
| item_id           | int    |    4   | Oasis item_id                                                       |    300      |

Data packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| sidx              | int    |    4   | Sample index                                                        |    10       |
| loss              | float  |    4   | The ground up loss for the sample                                   | 5675.675    |

The data packet may be a variable length and so an sidx of 0 identifies the end of the data packet.

There are two values of sidx with special meaning as follows;

| sidx   |  Meaning                                      | Required / optional |
|:-------|:----------------------------------------------|---------------------|
|   -1   | numerical integration mean loss               |   required          |
|   -2   | numerical integration standard deviation loss |   required          |

sidx -1 and -2 must come at the beginning of the data packet before the other samples.

### gulcalc coverage stream (deprecated)

The main difference from the gulcalc item stream is that the field in the gulcalc header packet structure is coverage_id, representing a grouping of item_id, rather than item_id. The distinction and rationale for having this as a alternative stream is explained in the Reference Model section.  

Stream header packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| stream_id         | int    |   1/3  | Identifier of the data stream type.                                 |    1/2      |
| no_of_samples     | int    |   4    | Number of samples                                                   |    100      |

Data header packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| event_id          | int    |    4   | Oasis event_id                                                      |   4545      |
| coverage_id       | int    |    4   | Oasis coverage_id                                                   |    150      |

Data packet structure

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| sidx              | int    |    4   | Sample index                                                        |    10       |
| loss              | float  |    4   | The ground up loss for the sample                                   | 5675.675    |

Only the numerical integration mean loss is output.

| sidx   |  Meaning                                 | Required / optional |
|:-------|:-----------------------------------------|---------------------|
|   -1   | numerical integration mean loss          |      required       |

sidx -1 must come at the beginning of the data packet before the other samples.

[Return to top](#specification)

<a id="loss"></a>
### loss stream

Stream header packet structure

| Name              | Type   |  Bytes | Description                          | Example     |
|:------------------|--------|--------| :------------------------------------|------------:|
| stream_id         | int    |   1/3  | Identifier of the data stream type.  |    2/1      |
| no_of_samples     | int    |   4    | Number of samples                    |    100      |

Data header packet structure

| Name               | Type   |  Bytes | Description                                              | Example     |
|:-------------------|--------|--------| :--------------------------------------------------------|------------:|
| event_id           | int    |    4   | Oasis event_id                                           |   4545      |
| item_id /output_id | int    |    4   | Oasis item_id (gulcalc) or output_id (fmcalc)            |    300      |

Data packet structure

| Name              | Type   |  Bytes | Description                          | Example     |
|:------------------|--------|--------| :------------------------------------|------------:|
| sidx              | int    |    4   | Sample index                         |   10        |
| loss              | float  |    4   | The  loss for the sample             | 5625.675    |

The data packet may be a variable length and so a sidx of 0 identifies the end of the data packet.

There are three values of sidx with special meaning as follows;

| sidx   |  Meaning                                      | Required / optional|
|:-------|:----------------------------------------------|--------------------|
|   -1   | numerical integration mean loss               |   required         |
|   -2   | numerical integration standard deviation loss |   optional         |
|   -3   | maximum exposure value                        |   required         |

sidx -1 to -3 must come at the beginning of the data packet before the other samples.

[Return to top](#specification)

### summary stream <a id="summary"></a>

Stream header packet structure

| Name              | Type   |  Bytes | Description                          | Example     |
|:------------------|--------|--------| :------------------------------------|------------:|
| stream_id         | int    |   1/3  | Identifier of the data stream type.  |    3/1      |
| no_of_samples     | int    |   4    | Number of samples                    |    100      |

Data header packet structure

| Name              | Type   |  Bytes | Description                                          | Example     |
|:------------------|--------|--------| :----------------------------------------------------|------------:|
| event_id          | int    |    4   | Oasis event_id                                       |   4545      |
| summary_id        | int    |    4   | Oasis summary_id                                     |    300      |
| exposure_value    | float  |    4   | Exposure value (sum of sidx -3 losses for summary_id)|    987878   |

Data packet structure

| Name              | Type   |  Bytes | Description                          | Example     |
|:------------------|--------|--------| :------------------------------------|------------:|
| sidx              | int    |    4   | Sample index                         |   10        |
| loss              | float  |    4   | The loss for the sample              | 5625.675    |

The data packet may be a variable length and so a sidx of 0 identifies the end of the data packet.

The sidx -1 mean loss may be present (if non-zero)

| sidx   |  Meaning                                      | Required / optional|
|:-------|:----------------------------------------------|--------------------|
|   -1   | numerical integration mean loss               |   optional         |


[Return to top](#specification)

## Components 

<a id="eve"></a>
### eve 

eve is an 'event emitter' and its job is to read a list of events from file and send out a subset of events as a binary data stream. It has no standard input and emits a list of event_ids, which are 4 byte integers.

eve is used to partition lists of events such that a workflow can be distributed across multiple processes.

<a id="getmodel"></a>
### getmodel 

getmodel is the component which generates a stream of effective damageability cdfs for a given set of event_ids and the impacted exposed items on the basis of their areaperil_ids (location) and vulnerability_ids (damage function). 

<a id="gulcalc"></a>
### gulcalc 

gulcalc is the component which calculates ground up loss. It takes the getmodel output as standard input and based on the sampling parameters specified, performs Monte Carlo sampling and numerical integration. The output is a stream of ground up loss samples in Oasis kernel format, with numerical integration mean (sidx=-1), standard deviation (sidx=-2) and exposure_value (sidx =-3).

The maximum exposed value, sidx=-3 represents the 100% ground up loss scenario to all items impacted by an event and is used by outputs that require an exposure value.

<a id="fmcalc"></a>
### fmcalc 

fmcalc is the component which takes the gulcalc output stream as standard input and applies the policy terms and conditions to produce insured loss samples. fmcalc can also take the fmcalc output stream as input to perform recursive financial module calculations (e.g for the application of reinsurance terms). The output is a table of loss samples in Oasis kernel format, including the (re)insured loss for the numerical integration mean ground up loss (sidx=-1) and the maximum exposed value (sidx=-3).

The maximum exposed value, sidx=-3 represents the 100% loss scenario to all items impacted by an event after applying the (re)insurance terms and conditions and is used by outputs that require an exposure value.

<a id="summarycalc"></a>
### summarycalc
summarycalc is a component which sums the sampled losses from either gulcalc or fmcalc to the users required level(s) for reporting results.  This is a simple sum of the loss value by event_id, sidx and summary_id, where summary_id is a grouping of coverage_id or item_id for gulcalc or output_id for fmcalc defined in the user's input files.  


<a id="outputcalc"></a>
### outputcalc 

Outputcalc is a general term for an end-of-pipeline component which represents one of a potentially unlimited set of output components. Four examples are provided in the Reference Model. These are; 

* eltcalc
* leccalc
* aalcalc
* pltcalc

outputcalc performs results analysis such as an event loss table or loss exceedance curve on the sampled output from summarycalc.  The output is a results table in csv format. 

[Return to top](#specification)

[Go to 4. Reference model](ReferenceModelOverview.md)

[Back to Contents](Contents.md)
