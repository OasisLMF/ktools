# getmodel

#### Introduction

This section provides a specification of getmodel.

getmodel generates a stream of cumulative effective damageability distributions (cdfs) from an input list of events. Specifically, it reads pre-generated Oasis format cdfs and converts them into a binary stream. This source data originates from an Oasis back-end database when the program is invoked by Oasis webservices.  To invoke it on a standalone basis, the source data must have been generated as binary files.

As well as being a core component of the Oasis in-memory kernel, getmodel also acts as a reference example of the class of programs which generates the damage distributions and streams them into memory. It is envisaged that model developers who wish to use ktools (the Oasis Developer Toolkit) as back-end components to their existing platforms can write their own version of getmodel, reading in their own source data and converting it into Oasis format cdfs. As long as the standard input and output structures are adhered to, the program can be written in any langauge and read any input data.

#### Standard inputs and outputs

The standard input and outputs are specified in DataStructures. 

The first header in the standard output is 'stream type'. This is an flag which identifies the data structure of the output, where stream type '1' is standard Oasis format. It is therefore possible to define a different output format for cdfs by setting this field to a different value.  However components that are downstream from getmodel, such as gulcalc, must also be written to handle bespoke stream types.

#### Parameters
The single parameter is Chunk_id (int). 

#### Usage
```
$ [stdin] | getmodel 1 | [stdout]
```

#### Internal data
