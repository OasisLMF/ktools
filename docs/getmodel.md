# getmodel

#### Introduction

This section provides a specification of getmodel.

getmodel generates a stream of cumulative effective damageability distributions (cdfs) from an input list of events. Specifically, it reads pre-generated Oasis format cdfs and converts them into a binary stream. This source data originates from an Oasis back-end database when the program is invoked by Oasis webservices.  To invoke it on a standalone basis, the source data must have been generated as binary files.

As well as being a core component of the Oasis in-memory kernel, getmodel also acts as a reference example of the class of programs which generates the damage distributions for an event set and streams them into memory. It is envisaged that model developers who wish to use ktools (the Oasis Developer Toolkit) as a back-end calculator of their existing platforms can write their own version of getmodel, reading in their own source data and converting it into Oasis format cdfs. As long as the standard input and output structures are adhered to, the program can be written in any langauge and read any input data.

#### Standard inputs and outputs

The standard input and outputs are specified in DataStructures. 

The first header in the standard output is 'stream type'. This is an flag which identifies the data structure of the output, where stream type '1' is standard Oasis format. It is therefore possible to define a different output format for cdfs by setting this field to a different value.  However components that are downstream from getmodel, such as gulcalc, must also be written to handle bespoke stream types.

#### Parameters
The single parameter is chunk_id (int). 

#### Usage
```
$ [stdin] | getmodel 1 | [stdout]
```

#### Internal data
The program requires the damage bin dictionary for the model, the Oasis format cdf in chunks, and an index file for each cdf chunk as binary files. These are currently hardcoded filenames found in the directory where the program is invoked;
* damage_bin_dictionary.bin
and in a cdf specific subdirectory
* cdf/damage_cdf_chunk_{chunk_id}.bin
* cdf/damage_cdf_chunk_{chunk_id}.idx

The data structure of damage_bin_dictionary.bin is as follows;

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| bin_index         | int    |    4   | Identifier of the damage bin                                        |     1       |
| bin_from          | float  |    4   | Lower damage threshold for the bin                                  |   0.01      |
| bin_to            | float  |    4   | Upper damage threshold for the bin                                  |   0.02      |
| interpolation     | float  |    4   | Interpolation damage value for the bin (usually the mid-point)      |   0.015     |
| interval_type     | int    |    4   | Identifier of the interval type, e.g. closed, open                  |    1201     |   
The data structure of damage_cdf_chunk_{chunk_id}.bin is as follows;

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| event_id          | int    |    4   | Oasis event_id                                                      |     1       |
| areaperil_id      | int    |    4   | Oasis areaperil_id                                                  |   4545      |
| vulnerability_id  | int    |    4   | Oasis vulnerability_id                                              |   345456    |
| bin_index         | int    |    4   | Identifier of the damage bin                                        |     10      |
| prob_to           | float  |    4   | The cumulative probability at the upper damage bin threshold        |    0.765    | 

The cdfs are ordered by event and streamed out in blocks for each event.  The index file damage_cdf_chunk_{chunk_id}.bin contains the position of each new event_id in the stream, for faster processing.

Note that the prob_from field from the existing database table structure of Oasis damage cdfs has been dropped to minimise the size of the table, as it is implied from the prior record prob_to field.

The program reads the damage bin mid-point (interpolation field) from the damage bin dictionary and includes it as a new field in the CDF stream as 'bin_mean'.  This field is the conditional mean damage for the bin and it is used to determine the interpolation method and to facilitate the calculation of mean and standard deviation in the stream. When the conditional mean is the mid-point of the bin then the gulcalc program performs linear interpolation. However this stream type can also support quadratic interpolation when the bin_mean is not the mid-point of the damage bin. 
