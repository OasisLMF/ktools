# Input data tools <a id="inputtools"></a>

The following components convert input data in csv format to the binary format required by the calculation components in the reference model;

* **[evetobin](#events)** is a utility to convert a list of event_ids into binary format.
* **[damagetobin](#damagebins)** is a utility to convert the Oasis damage bin dictionary table into binary format. 
* **[exposuretobin](#exposuretobin)** is a utility to convert the Oasis exposure instance table into binary format. 
* **[randtobin](#randtobin)** is a utility to convert a list of random numbers into binary format. 
* **[cdfdatatobin](#cdfdatatobin)** is a utility to convert the Oasis cdf data into binary format.
* **[fmdatatobin](#fmdatatobin)** is a utility to convert the Oasis FM instance data into binary format.
* **[fmxreftobin](#fmxreftobin)** is a utility to convert the Oasis FM xref table into binary format.


These components are intended to allow users to generate the required input binaries from csv independently of the original data store and technical environment. All that needs to be done is first generate the csv files from the data store (SQL Server database, etc).

```
Note that oatools contains a component 'gendata' which generates all of the input binaries directly from a SQL 
Server Oasis back-end database. This component is specific to the implementation of the in-memory kernel as a
calculation back-end to the Oasis mid-tier which is why it is kept in a separate project.
```

The following components convert the binary input data required by the calculation components in the reference model into csv format;
* **[evetocsv](#events)** is a utility to convert the event binary into csv format.
* **[damagetocsv](#damagebins)** is a utility to convert the Oasis damage bin dictionary binary into csv format.
* **[exposuretocsv](#exposuretocsv)** is a utility to convert the Oasis exposure instance binary into csv format.
* **[randtocsv](#randtocsv)** is a utility to convert the random numbers binary into csv format.
* **[cdfdatatocsv](#cdfdatatocsv)** is a utility to convert the Oasis cdf data binary into csv format.
* **[fmdatatocsv](#fmdatatocsv)** is a utility to convert the Oasis FM instance binary into csv format.
* **[fmxreftocsv](#fmxreftocsv)** is a utility to convert the Oasis FM xref binary into csv format.

These components are provided for convenience of viewing the data and debugging.

<a id="events"></a>
## Events
One or more event binaries are required by eve and getmodel. It must have the following filename format, each uniquely identified by a chunk number (integer >=0);
* e_chunk_{chunk}_data.bin

The chunks represent subsets of events.

```
In general more than 1 chunk of events is not necessary for the in-memory kernel as the computation can be 
parallelized across the processes. This feature may be removed in future releases.
```

#### File format
The csv file should contain a list of event_ids (integers) and no header.

| Name              | Type   |  Bytes | Description         | Example     |
|:------------------|--------|--------| :-------------------|------------:|
| event_id          | int    |    4   | Oasis event_id      |   4545      |

#### evetobin
```
$ evetobin < e_chunk_1_data.csv > e_chunk_1_data.bin
```

#### evetocsv
```
$ evetocsv < e_chunk_1_data.bin > e_chunk_1_data.csv
```
[Return to top](#inputtools)

## Damage bins <a id="damagebins"></a>
The damage bin dictionary is a reference table in Oasis which defines how the effective damageability cdfs are discretized on a relative damage scale (normally between 0 and 1). It is required by getmodel and gulcalc and must have the following filename format;
* damage_bin_dict.bin

#### File format
The csv file should contain the following fields and include a header row.

| Name              | Type   |  Bytes | Description                                                   | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------|------------:|
| bin_index         | int    |    4   | Identifier of the damage bin                                  |     1       |
| bin_from          | float  |    4   | Lower damage threshold for the bin                            |   0.01      |
| bin_to            | float  |    4   | Upper damage threshold for the bin                            |   0.02      |
| interpolation     | float  |    4   | Interpolation damage value for the bin (usually the mid-point)|   0.015     |
| interval_type     | int    |    4   | Identifier of the interval type, e.g. closed, open            |   1201      | 

The data should be ordered by bin_index ascending with bin_index starting from 1 and not contain nulls.

#### damagetobin
```
$ damagetobin < damage_bin_dict.csv > damage_bin_dict.bin
```

#### damagetocsv
```
$ damagetocsv < damage_bin_dict.bin > damage_bin_dict.csv
```
[Return to top](#inputtools)

<a id="exposures"></a>
## Exposures 
The exposures binary contains the list of exposures for which ground up loss will be sampled in the kernel calculations. The data format is that of the Oasis Exposure instance. It is required by gulcalc and outputcalc and must have the following filename format;
* exposures.bin

#### File format
The csv file should contain the following fields and include a header row.

| Name              | Type   |  Bytes | Description                                                   | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------|------------:|
| item_id           | int    |    4   | Identifier of the exposure item                               |     1       |
| areaperil_id      | int    |    4   | Identifier of the locator and peril of the item               |   4545      |
| vulnerability_id  | int    |    4   | Identifier of the vulnerability distribution of the item      |   345456    |
| tiv               | float  |    4   | The total insured value of the item                           |   200000    |
| group_id          | int    |    4   | Identifier of the correlation group of the item               |    1        |  

The data should be ordered by areaperil_id, vulnerability_id ascending and not contain nulls.

#### exposuretobin
```
$ exposuretobin < exposures.csv > exposures.bin
```

#### exposuretocsv
```
$ exposuretocsv < exposures.bin > exposures.csv
```

[Return to top](#inputtools)

<a id="random"></a>
## Random numbers 
One or more random number files may be provided for the gulcalc component as an option (using gulcalc -r parameter) The random number binary contains a list of random numbers used for ground up loss sampling in the kernel calculation. It should be provided for the same number of chunks as events and must have the following filename format;
* random_{chunk}.bin

If the gulcalc -r parameter is not used, the random number binary is not required and random numbers are generated dynamically in the calculation. 

#### File format
The csv file should contain the runtime number of samples as the first value followed by a simple list of random numbers. It should not contain any headers.

First value;

| Name              | Type     |  Bytes | Description                  Example     |
|:------------------|----------|--------| :--------------------------|------------:|
| number of samples | integer  |    4   | Runtime number of samples  |  100        |

Subsequent values;

| Name              | Type   |  Bytes | Description                    | Example     |
|:------------------|--------|--------| :------------------------------|------------:|
| rand              | float  |    4   | Random number between 0 and 1  |  0.75875    |  

The first value in the file is an exception and should contain the runtime number of samples (integer > 0).  This is required for running gulcalc in 'Reconciliation mode' (using gulcalc -R parameter) which uses the same random numbers as Oasis classic and produces identical sampled values.  If not running in reconciliation mode, it is not used and can be set to 1.

#### randtobin
```
$ randtobin < random_1.csv > random_1.bin
```

#### randtocsv
```
$ randtocsv < random_1.bin > random_1.csv
```

[Return to top](#inputtools)

<a id="cdfs"></a>
## CDFs 
One or more cdf data files are required for the getmodel component, as well as an index file containing the starting positions of each event block. These should be located in a cdf sub-directory of the main working directory and have the following filename format;
* cdf/damage_cdf_chunk_{chunk}.bin
* cdf/damage_cdf_chunk_{chunk}.idx

If chunked, the binary file should contain the cdf data for the same events as the corresponding chunk event binary. 

#### File format
The csv file should contain the following fields and include a header row.

| Name              | Type   |  Bytes | Description                                                   | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------|------------:|
| event_id          | int    |    4   | Oasis event_id                                                |     1       |
| areaperil_id      | int    |    4   | Oasis areaperil_id                                            |   4545      |
| vulnerability_id  | int    |    4   | Oasis vulnerability_id                                        |   345456    |
| bin_index         | int    |    4   | Identifier of the damage bin                                  |     10      |
| prob_to           | float  |    4   | The cumulative probability at the upper damage bin threshold  |    0.765    | 

The data should be ordered by event_id, areaperil_id, vulnerability_id, bin_index ascending with bin_index starting at 1, and not contain nulls. All bins corresponding to the bin indexes in the damage bin dictionary should be present, except records may be truncated after the last bin where the prob_to = 1.

#### cdfdatatobin
Not yet implemented. A component will be provided to produce both binary and index file from the csv.

#### cdfdatatocsv
```
$ cdfdatatocsv < damage_cdf_chunk_1.bin > damage_cdf_chunk_1.csv
```

[Return to top](#inputtools)

<a id="fmdata"></a>
## FM data 
The fmdata binary file contains the policy terms and conditions required to perform a loss calculation, and is required for fmcalc only. The source format is Oasis FM Instance data, which is the Oasis native format data tables which describe an insurance programme. These four tables have been combined into one with the below structure.

This file should be located in a fm sub-directory of the main working directory and have the following filename.
* fm/fm_data.bin

#### File format
The csv file should contain the following fields and include a header row.


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

The data should be ordered by item_id and not contain any nulls (replace with 0).

#### fmdatatobin
```
$ fmdatatobin < fm_data.csv > fm_data.bin
``` 

#### fmdatatocsv
```
$ fmdatatocsv < fm_data.bin > fm_data.csv
``` 

[Return to top](#inputtools)

<a id="fmxref"></a>
## FM xref 
The fmxref binary file contains cross reference data linking the output_id in the fmcalc output back to item_id, and is required for outputcalc only. This should be located in a fm sub-directory of the main working directory and have the following filename.

* fm/fmxref.bin

#### File format
The csv file should contain the following fields and include a header row.

| Name                        | Type   |  Bytes | Description                                    | Example     |
|:----------------------------|--------|--------| :----------------------------------------------|------------:|
| item_id                     | int    |    4   | Identifier of the exposure item                |    56745    |
| output_id                   | int    |    4   | Oasis Financial Module output_id               |     546     |

The data should not contain any nulls.

#### fmxreftobin
```
$ fmxreftobin < fmxref.csv > fmxref.bin
``` 

#### fmxreftocsv
```
$ fmxreftocsv < fmxref.bin > fmxref.csv
``` 

[Return to top](#inputtools)

[Go to Output data tools](Outputtools.md)

[Back to Contents](Contents.md)
