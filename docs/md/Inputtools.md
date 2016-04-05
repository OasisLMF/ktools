# Input data tools <a id="inputtools"></a>

The following components convert input data in csv format to the binary format required by the calculation components in the reference model;

* **[evetobin](#events)** is a utility to convert a list of event_ids into binary format.
* **[damagetobin](#damagebins)** is a utility to convert the Oasis damage bin dictionary table into binary format. 
* **[exposuretobin](#exposuretobin)** is a utility to convert the Oasis exposure instance table into binary format. 
* **[randtobin](#randtobin)** is a utility to convert a list of random numbers into binary format. 
* **[cdfdatatobin](#cdfdatatobin)** is a utility to convert the Oasis cdf data into binary format.
* **[fmprogrammetobin](#fmprogrammetobin)** is a utility to convert the Oasis FM programme data into binary format.
* **[fmprofiletobin](#fmprofiletobin)** is a utility to convert the Oasis FM profile data into binary format.
* **[fmpolicytctobin](#fmpolicytctobin)** is a utility to convert the Oasis FM policytc data into binary format.
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
* **[fmprogrammetocsv](#fmprogrammetocsv)** is a utility to convert the Oasis FM programme data into csv format.
* **[fmprofiletocsv](#fmprofiletocsv)** is a utility to convert the Oasis FM profile data into csv format.
* **[fmpolicytctocsv](#fmpolicytctocsv)** is a utility to convert the Oasis FM policytc data into csv format.
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
```
$ cdfdatatobin damage_cdf_chunk_1 102 < damage_cdf_chunk_1.bin
```
This command will create a binary file damage_cdf_chunk_1.bin and an index file damage_cdf_chunk_1.idx in the working directory.

In general the usage is;

```
$ cdfdatatobin damage_cdf_chunk_[chunk] [maxbins] < input.csv
```
chunk is an integer and maxbins is the maximum number of bins in the cdf. input.csv must conform to the csv format given above.

#### cdfdatatocsv
```
$ cdfdatatocsv < damage_cdf_chunk_1.bin > damage_cdf_chunk_1.csv
```

[Return to top](#inputtools)

<a id="fmprogramme"></a>
## fm programme 
The fm programme binary file contains the level heirarchy and defines aggregations of losses required to perform a loss calculation, and is required for fmcalc only. 

This file should be located in a fm sub-directory of the main working directory and have the following filename.
* fm/fm_programme.bin

#### File format
The csv file should contain the following fields and include a header row.


| Name                     | Type   |  Bytes | Description                                    | Example     |
|:-------------------------|--------|--------| :----------------------------------------------|------------:|
| prog_id                  | int    |    4   | Oasis Financial Module prog_id                 |    1        |
| from_agg_id              | int    |    4   | Oasis Financial Module from_agg_id             |    1        |
| level_id                 | int    |    4   | Oasis Financial Module level_id                |     1       |
| to_agg_id                | int    |    4   | Oasis Financial Module to_agg_id               |     1       |

* All fields must have integer values and no nulls
* Must have at least one level where level_id = 1, 2, 3 ...
* For level_id = 1, the set of values in from_agg_id must be equal to the set of item_ids in the input ground up loss stream (which has fields event_id, item_id, idx, gul).  Therefore level 1 always defines a group of items.
* For subsequent levels, the from_agg_id must be the distinct values from the previous level to_agg_id field.
* Each programme table may only have a single integer value in prog_id. Note that this field is a cross-reference to a separate prog dictionary and business meaningful information such as account number, and is not currently used in calculations.  This field may be deprecated in future versions.
* The from_agg_id and to_agg_id values, for each level, should be a contiguous block of integers (a sequence with no gaps).  This is not a strict rule in this version and it will work with non-contiguous integers, but it is recommended as best practice.

#### fmprogrammetobin
```
$ fmprogrammetobin < fm_programme.csv > fm_programme.bin
``` 

#### fmprogrammetocsv
```
$ fmprogrammetocsv < fm_programme.bin > fm_programme.csv
```

<a id="fmprofile"></a>
## fm profile
The fmprofile binary file contains the list of calculation rules with profile values (policytc_ids) that appear in the policytc file. This is required for fmcalc only. 

This file should be located in a fm sub-directory of the main working directory and have the following filename.
* fm/fm_profile.bin

#### File format
The csv file should contain the following fields and include a header row.

| Name                     | Type   |  Bytes | Description                                    | Example     |
|:-------------------------|--------|--------| :----------------------------------------------|------------:|
| policytc_id              | int    |    4   | Oasis Financial Module policytc_id             |     34      |
| calcrule_id              | int    |    4   | Oasis Financial Module calcrule_id             |      1      |
| allocrule_id             | int    |    4   | Oasis Financial Module allocrule_id            |      0      |
| sourcerule_id            | int    |    4   | Oasis Financial Module sourcerule_id           |      0      |
| levelrule_id             | int    |    4   | Oasis Financial Module levelrule_id            |      0      |
| ccy_id                   | int    |    4   | Oasis Financial Module ccy_id                  |      0      |
| deductible               | float  |    4   | Deductible                                     |   50        |
| limit                    | float  |    4   | Limit                                          |   100000    |
| share_prop_of_lim        | float  |    4   | Share/participation as a proportion of limit   |   0         |
| deductible_prop_of_loss  | float  |    4   | Deductible as a proportion of loss             |   0         |
| limit_prop_of_loss       | float  |    4   | Limit as a proportion of loss                  |   0         |
| deductible_prop_of_tiv   | float  |    4   | Deductible as a proportion of TIV              |   0         |
| limit_prop_of_tiv        | float  |    4   | Limit as a proportion of TIV                   |   0         |
| deductible_prop_of_limit | float  |    4   | Deductible as a proportion of limit            |   0         |

* All distinct policytc_id values that appear in the policytc table must appear once in the policytc_id field of the profile table. We suggest that policytc_id=1 is included by default using calcrule_id = 12 and DED = 0 as a default 'null' calculation rule whenever no terms and conditions apply to a particular level_id / agg_id in the policytc table.
* All data fields that are required by the relevant profile must be provided, with the correct calcrule_id (see FM Profiles)
* Any fields that are not required for the profile should be set to zero.
* allocrule_id may be set to 0 or 1 for each policytc_id.  Generally, it is recommended to use 0 everywhere except for the final level calculations when back-allocated losses are required, else 0 everywhere.
* The fields not currently used at all are ccy_id, sourcerule_id and levelrule_id

#### fmprofiletobin
```
$ fmprofiletobin < fm_profile.csv > fm_profile.bin
``` 

#### fmprofiletocsv
```
$ fmprofiletocsv < fm_profile.bin > fm_profile.csv
```

<a id="fmpolicytc"></a>
## fm policytc
The fm policytc binary file contains the cross reference between the aggregations of losses defined in the fm programme file at a particular level and the calculation rule that should be applied as defined in the fm profile file. This is required for fmcalc only. 

This file should be located in a fm sub-directory of the main working directory and have the following filename.
* fm/fm_policytc.bin

#### File format
The csv file should contain the following fields and include a header row.


| Name                     | Type   |  Bytes | Description                                    | Example     |
|:-------------------------|--------|--------| :----------------------------------------------|------------:|
| prog_id                  | int    |    4   | Oasis Financial Module prog_id                 |    1        |
| layer_id                 | int    |    4   | Oasis Financial Module layer_id                |    1        |
| level_id                 | int    |    4   | Oasis Financial Module level_id                |     1       |
| agg_id                   | int    |    4   | Oasis Financial Module agg_id                  |     1       |
| policytc_id              | int    |    4   | Oasis Financial Module policytc_id             |     1       |

* All fields must have integer values and no nulls
* Must contain the same levels as the fm programme where level_id = 1, 2, 3 ...
* For every distinct combination of to_agg_id and level_id in the programme table, there must be a corresponding record matching level_id and agg_id values in the policytc table with a valid value in the policytc_id field.  
* layer_id = 1 at all levels except the last where there may be multiple layers, with layer_id = 1, 2, 3 ... This allows for the specification of several policy contracts applied to the same aggregation of losses defined in the programme table.
 
#### fmpolicytctobin
```
$ fmpolicytctobin < fm_policytc.csv > fm_policytc.bin
``` 

#### fmpolicytctocsv
```
$ fmpolicytctocsv < fm_policytc.bin > fm_policytc.csv
```

[Return to top](#inputtools)


<a id="fmxref"></a>
## fm xref 
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
