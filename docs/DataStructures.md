# Data Structures

### Introduction

This section outlines the data structures of the components of ktools.

Every component in the toolkit has a standard input and output data structure. These data structures are not defined explicitly as meta data in the code as they would be in a database language, and they have been designed to minimise the volume flowing through the pipeline. For example, indexes which are common to a block of data are defined as a header record and then only the variable data records that are relevant to the header key are part of the data stream. The order in which the data records are streamed have meaning, again, unlike a table in a database where the record order doesn't matter.

### Eve

Eve is an 'event emmitter' and it splits up a block of event_ids . it takes a stream of event_ids as input and outputs a subset of event_ids according to the parameters supplied (int chunk_id_,int pno_,int total_).

#### Stdin
* Format: binary
* Data: event_id

#### Stdount
* Format: binary
* Data: event_id
