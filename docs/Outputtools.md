# Output data tools <a id="outputtools"></a>

The following components convert the binary output of each calculation component to csv format;
* **[cdftocsv](#cdftocsv)** is a utility to convert binary format CDFs to a csv. getmodel standard output can be streamed directly into cdftocsv, or a binary file of the same format can be input.
* **[gultocsv](#gultocsv)** is a utility to convert binary format GULs to a csv. gulcalc standard output can be streamed directly into gultocsv, or a binary file of the same format can be input.
* **[fmtocsv](#fmtocsv)** is a utility to convert binary format losses to a csv. fmcalc standard output can be streamed directly into fmtocsv, or a binary file of the same format can be input.

Figure 1 shows the workflows for the data conversion components.

##### Figure 1. Data Conversion Workflows
![alt text](https://github.com/OasisLMF/ktools/blob/master/docs/img/Dbtools2.jpg "Data Conversion Workflows")

## cdftocsv <a id="cdftocsv"></a>

A component which converts the getmodel output stream, or binary file with the same structure, to a csv file.

##### Stdin stream_id

| Byte 1 | Bytes 2-4 |  Description      |
|:-------|-----------|:------------------|
|    0   |     1     |  getmodel stdout  |

A binary file of the same format can be piped into cdftocsv.

##### Usage
```
$ [stdin component] | cdftocsv > [output].csv
$ cdftocsv < [stdin].bin > [output].csv
```

##### Example
```
$ eve 1 1 1 | getmodel | cdftocsv > cdf.csv
$ cdftocsv < getmodel.bin > cdf.csv 
```

##### Output
Csv file with the following fields;

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| event_id          | int    |    4   | Oasis event_id                                                      |   4545      |
| areaperil_id      | int    |    4   | Oasis areaperil_id                                                  |  345456     |
| vulnerability_id  | int    |    4   | Oasis vulnerability_id                                              |   345       |
| bin_index         | int    |    4   | Damage bin index                                                    |    20       | 
| prob_to           | float  |    4   | The cumulative probability at the upper damage bin threshold        |   0.765     |
| bin_mean          | float  |    4   | The conditional mean of the damage bin                              |   0.45      |

[Return to top](#outputtools)

## gultocsv <a id="gultocsv"></a>

A component which converts the gulcalc output stream, or binary file with the same structure, to a csv file.

##### Stdin stream_id

| Byte 1 | Bytes 2-4 |  Description      |
|:-------|-----------|:------------------|
|    1   |     1     |  gulcalc stdout   |

A binary file of the same format can be piped into gultocsv.

##### Usage
```
$ [stdin component] | gultocsv > [output].csv
$ gultocsv < [stdin].bin > [output].csv
```

##### Example
```
$ eve 1 1 1 | getmodel | gulcalc -S100 -C1 | gultocsv > gulcalc.csv
$ gultocsv < gulcalc.bin > gulcalc.csv 
```

##### Output
Csv file with the following fields;

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| event_id          | int    |    4   | Oasis event_id                                                      |   4545      |
| item_id           | int    |    4   | Oasis item_id                                                       |    300      |
| sidx              | int    |    4   | Sample index                                                        |     10      |
| gul               | float  |    4   | The ground up loss value                                            | 5675.675    |

[Return to top](#outputtools)

## fmtocsv <a id="fmtocsv"></a>

A component which converts the fmcalc output stream, or binary file with the same structure, to a csv file.

##### Stdin stream_id

| Byte 1 | Bytes 2-4 |  Description      |
|:-------|-----------|:------------------|
|    2   |     1     |  fmcalc stdout    |

A binary file of the same format can be piped into fmtocsv.

##### Usage
```
$ [stdin component] | fmtocsv > [output].csv
$ fmtocsv < [stdin].bin > [output].csv
```

##### Example
```
$ eve 1 1 1 | getmodel | gulcalc -S100 -C1 | fmcalc | fmtocsv > fmcalc.csv
$ fmtocsv < fmcalc.bin > fmcalc.csv 
```

##### Input
Same as fmcalc output or a binary file of the same format can be piped into fmtocsv.

##### Output
Csv file with the following fields;

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| event_id          | int    |    4   | Oasis event_id                                                      |   4545      |
| prog_id           | int    |    4   | Oasis prog_id                                                       |    1        |
| layer_id          | int    |    4   | Oasis layer_id                                                      |    1        |
| output_id         | int    |    4   | Oasis output_id                                                     |    5        |
| sidx              | int    |    4   | Sample index                                                        |    10       |
| loss              | float  |    4   | The insured loss value                                              | 5375.675    |

[Return to top](#outputtools)

[Go to Planned work](PlannedWork.md)

[Back to Contents](Contents.md)

