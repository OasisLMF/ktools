# 4.3 ORD Output Components <a id="ordoutputcomponents"></a>

As well as the set of legacy outputs described in OutputComponents.md, ktools also supports Open Results Data "ORD" output calculations and reports. 

Open Results Data is a data standard for catastrophe loss model results developed as part of Open Data Standards "ODS". ODS is curated by OasisLMF and governed by the Open Data Standards Steering Committee (SC), comprised of industry experts representing (re)insurers, brokers, service providers and catastrophe model vendors. More information about ODS can be found here.

### ordleccalc <a id="leccalc"></a>
***
This component produces several variants of loss exceedance curves, known as Exceedance Probability Tables "EPT" under ORD. 

Exceedance Probability Table is a set of user-specified percentiles of (typically) annual loss on one of two bases – AEP (sum of losses from all events in a year) or OEP (maximum of any one event’s losses in a year).  In ORD the percentiles are expressed as Return Periods, which is the reciprocal of the percentile. 

How EPTs are derived in general depends on the methodology of calculating the underlying ground up and insured losses. In the Oasis kernel the methodology is Monte Carlo sampling from damage distributions, which results in several samples (realisations) of an event loss for every event in the model's catalogue. The event losses are assigned to a year timeline and the years rank ordered by loss, so the method of computing the percentiles is by taking the ratio of the frequency of years of loss exceeding a given threshold over the total number of years.

The sampling approach gives rise to the four variations of calculation of these statistics:

*	EP Table from Mean Damage Losses  – this means do the loss calculation for a year using the event mean damage loss computed by numerical integration of the effective damageability distributions. 
*	EP Table of Sample Mean Losses – this means do the loss calculation for a year using the statistical sample event mean.
*	Full Uncertainty EP Table – this means do the calculation across all samples (treating the samples effectively as repeat years) - this is the most accurate of all the single EP Curves.
*	Per Sample EPT (PSEPT) – this means calculate the EP Curve for each sample and leave it at the sample level of detail, resulting in multiple "curves".

Exceedance Probability Tables are further generalised in Oasis to represent not only annual loss percentiles but loss percentiles over potentially any period of time. Thus the typical use of 'Year' label in outputs is replaced by the more general term 'Period', which can be any period of time as defined in the model data 'occurrence' file (although the normal period of interest is a year).


##### Parameters

* -K{sub-directory}. The subdirectory of /work containing the input summarycalc binary files.
Then the following parameters must be specified for at least one analysis type;
* Analysis type. Use -F for Full Uncertainty Aggregate, -f for Full Uncertainty Occurrence, -W for Per Sample Aggregate,  -w for Per Sample Occurrence, -S for Sample Mean Aggregate, -s for Sample Mean Occurrence, -M for Per Sample Mean Aggregate, -m for Per Sample Mean Occurrence
* -O {ept.csv} is the output flag for the EPT csv ( for analysis types -F, -f, -S, -s, -M, -m)
* -o {psept.csv} is the output flag for the PSEPT csv (for analysis types -W or -w)

An optional parameter is; 
* -r. Use return period file - use this parameter if you are providing a file with a specific list of return periods. If this file is not present then all calculated return periods will be returned, for losses greater than zero.

##### Usage

```
$ ordleccalc [parameters] 

```

##### Examples
```
'First generate summarycalc binaries by running the core workflow, for the required summary set
$ eve 1 2 | getmodel | gulcalc -r -S100 -c - | summarycalc -g -1 - > work/summary1/summarycalc1.bin
$ eve 2 2 | getmodel | gulcalc -r -S100 -c - | summarycalc -g -1 - > work/summary1/summarycalc2.bin

'Then run ordleccalc, pointing to the specified sub-directory of work containing summarycalc binaries.
'Write aggregate and occurrence full uncertainty
$ ordleccalc -Ksummary1 -F -f -O ept.csv

'Write occurrence per sample (PSEPT)
$ ordleccalc -Ksummary1 -w -o psept.csv

'Write aggregate and occurrence per sample (written to PSEPT) and per sample mean (written to EPT file)
$ ordleccalc -Ksummary1 -W -w -M -m -O ept.csv -o psept.csv

'Write full output
$ ordleccalc -Ksummary1 -F -f -W -w -S -s -M -m -O ept.csv -o psept.csv
```

##### Internal data

ordleccalc requires the occurrence.bin file

* input/occurrence.bin

and will optionally use the following additional files if present

* input/returnperiods.bin
* input/periods.bin

ordleccalc does not have a standard input that can be streamed in. Instead, it reads in summarycalc binary data from a file in a fixed location.  The format of the binaries must match summarycalc standard output. The location is in the 'work' subdirectory of the present working directory. For example;

* work/summarycalc1.bin
* work/summarycalc2.bin
* work/summarycalc3.bin

The user must ensure the work subdirectory exists.  The user may also specify a subdirectory of /work to store these files. e.g.

* work/summaryset1/summarycalc1.bin
* work/summaryset1/summarycalc2.bin
* work/summaryset1/summarycalc3.bin

The reason for ordleccalc not having an input stream is that the calculation is not valid on a subset of events, i.e. within a single process when the calculation has been distributed across multiple processes.  It must bring together all event losses before assigning event losses to periods and ranking losses by period.  The summarycalc losses for all events (all processes) must be written to the /work folder before running leccalc.

##### Calculation

All files with extension .bin from the specified subdirectory are read into memory, as well as the occurrence.bin. The summarycalc losses are grouped together and sampled losses are assigned to period according to which period the events occur in.

If multiple events occur within a period;
* For **aggregate** loss exceedance curves, the sum of losses is calculated.
* For **occurrence** loss exceedance curves, the maximum loss is calculated.

Then the calculation differs by EPCalc type, as follows;

* 1. The mean damage loss (sidx = -1) is output as a standard exceedance probability table.  If the calculation is run with 0 samples, then leccalc will still return the mean damage loss exceedance curve.  The 'EPType' field in the output identifies the type of loss exceedance curve.

* 2. Full uncertainty - all losses by period are rank ordered to produce a single loss exceedance curve. 

* 3. Per Sample mean - the return period losses from the Per Sample EPT are averaged, which produces a single loss exceedance curve.

* 4. Sample mean - the losses by period are first averaged across the samples, and then a single loss exceedance table is created from the period sample mean losses.

The 'EPtypes' are;

* 1. OEP

* 2. OEP TVAR

* 3. AEP

* 4. AEP TVAR

TVAR results are generated automatically if the OEP or AEP report is selected in the analysis options. TVAR, or Tail Conditional Expectation (TCE), is computed by averaging the rank ordered losses exceeding a given return period loss from the respective OEP or AEP result.

##### Output

Exceedance Probability Tables (EPT)

csv files with the following fields;

**Exceedance Probability Table (EPT)**

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| SummaryId         | int    |    4   | identifier representing a summary level grouping of losses          |   10        |
| EPCalc            | int    |    4   | 1, 2, 3 or 4								                        |    2        |
| EPType 			| int    |    4   | 1, 2, 3 or 4			                                            |    1        |
| ReturnPeriod		| float  |    4   | return period interval                                              |    250      |
| loss              | float  |    4   | loss exceedance threshold or TVAR for return period                 |    546577.8 |

**Per Sample Exceedance Probability Tables (PSEPT)**

| Name              | Type   |  Bytes | Description                                                         | Example     |
|:------------------|--------|--------| :-------------------------------------------------------------------|------------:|
| SummaryId         | int    |    4   | identifier representing a summary level grouping of losses          |   10        |
| SampleID          | int    |    4   | Sample number								                        |    20       |
| EPType 			| int    |    4   | 1, 2, 3 or 4			                                            |    3        |
| ReturnPeriod		| float  |    4   | return period interval                                              |    250      |
| loss              | float  |    4   | loss exceedance threshold or TVAR for return period                 |    546577.8 |

##### Period weightings

An additional feature of ordleccalc is available to vary the relative importance of the period losses by providing a period weightings file to the calculation. In this file, a weight can be assigned to each period make it more or less important than neutral weighting (1 divided by the total number of periods). For example, if the neutral weight for period 1 is 1 in 10000 years, or 0.0001, then doubling the weighting to 0.0002 will mean that period's loss reoccurrence rate would double.  Assuming no other period losses, the return period of the loss of period 1 in this example would be halved.

All period_nos must appear in the file from 1 to P (no gaps). There is no constraint on the sum of weights. Periods with zero weight will not contribute any losses to the loss exceedance curve.

This feature will be invoked automatically if the periods.bin file is present in the input directory.

[Return to top](#outputcomponents)

[Go to 4.4 Data conversion components section](DataConversionComponents.md)

[Back to Contents](Contents.md)
