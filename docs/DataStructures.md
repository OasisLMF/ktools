# Data Structures

### Introduction

This section outlines the data structures of the components of ktools.

Most components in the toolkit have a standard input and output data stream structure. These data structures are not defined explicitly as meta data in the code as they would be in a database language, and they have been designed to minimise the volume flowing through the pipeline. For example, indexes which are common to a block of data are defined as a header record and then only the variable data records that are relevant to the header key are part of the data stream. The names of the data fields given below are unimportant, only their positions in the data stream matters.

### eve

Eve is an 'event emmitter' and its job is to read a list of events from file and send out a subset of events as a binary data stream. Because eve initiates a stream in the pipeline, it has no standard input.

#### Input
* Format: binary file with filename e_chunk_{chunk_id}_data.bin
* Data: event_id as 4 byte int.

#### Stdout
* Format: binary stream
* Header: none
* Data: event_id as 4 byte int.

### getmodel

getmodel is the reference example of generating a stream of cdfs for a given set of event ids. The cdfs are read from binary file (it is assumed the required data has already been read from a database or csv file by a separate utility).

#### Stdin
* Format: binary stream
* Header: none
* Data: event_id as 4 byte int.

#### Stdout
* Format: binary stream
* Header: 20 bytes (stream_type as 4 byte int, event_id as 4 byte int, areaperil_id as 4 byte int, vulnerability_id as 4 byte int, no_of_bins as 4 byte int)
* Data: no_of_bins * 8 bytes (prob_to as 4 byte float, bin_mean as 4 byte float)
