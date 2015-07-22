# Random numbers <a id="randomnumbers"></a>

Simple uniform random numbers are assigned to each event, group and sample number to sample ground up loss in the gulcalc process. A group is a collection of items which share the same group_id, and is the method of supporting spatial correlation in ground up loss sampling in Oasis and ktools.

#### Correlation

Items (typically representing, in insurance terms, the underlying risk coverages) that are assigned the same group_id will use the same random number to sample damage for a given event and sample number. Items with different group_ids will be assigned independent random numbers.  Therefore sampled damage is fully correlated within groups and fully independent between groups, where group is an abstract collection of items defined by the user.

The item_id, group_id data is provided by the user in the exposure input file (exposures.bin).

### Methodology

The method of assigning random numbers in gulcalc uses an random number index (ridx) which is used as a position reference into a list of random numbers.  S random numbers corresponding to the runtime number of samples are drawn from the list starting at the ridx position.

There are three options in ktools for choosing random numbers to apply in the sampling process.

#### 1. Generate dynamically during the calculation

##### Usage
No random number parameters are required, this is the default.

##### Example
```
$ gulcalc -C1 -S100
```
This will run 100 samples using dynamically generated random numbers.

##### Method

Random numbers are sampled dynamically using the Mersenne twister psuedo random number generator (the default RNG of the C++ v11 compiler). 
A sparse array capable of holding 1 million random numbers is allocated to each event. The RIDX is generated from the group_id and number of samples S using the following modulus function;

ridx=mod(group_id x S, 1000000)

This formula pseudo-randomly assigns ridx indexes to each GROUP_ID between 0 and 999,999. 

As a ridx is sampled, the section in the array starting at the ridx position of length S is populated with random numbers unless they have already been populated, in which case the existing random numbers are re-used.

The array is cleared for the next event and a new set of random numbers is generated.  

#### 2. Use numbers from random number file

##### Usage
Use -r as a parameter

##### Example
```
$ gulcalc -C1 -S100 -r
```
This will run 100 samples using random numbers from file random_1.bin.

##### Method
The random number file(s) is read into memory at the start of the gulcalc process. The same set of numbers are used for all events in the corresponding event file e_chunk_1_data.bin.

The ridx is generated from the event_id and group_id using the following modulus function;

ridx=mod(event_id x P1 + group_id x P2, D)

D is the divisor of the modulus, equal to the total number of random numbers in the list.
P1 and P2 are the first two prime numbers which are greater than half of D.

This formula pseudo-randomly assigns ridx indexes to each event_id and group_id combo between 0 and D-1.

#### 3. Use numbers from random number file in 'reconciliation mode'

##### Usage
Use -r -R as parameters

##### Example
```
$ gulcalc -C1 -S100 -r -R
```
This will run 100 samples using random numbers from file random_1.bin in reconciliation mode.

##### Method

Reconciliation mode uses the same method as 2. but calculates the ridx in the same way as Oasis classic, thereby producing exactly the same losses for comparison.  

In Oasis classic, random numbers are held in a matrix with the sample index in columns and the ridx is a reference into a row of random numbers for a sample set of size S.  

In reconciliation mode, the divisor D is set to the total number of rows. This is calculated from the number of columns S which is the first 4 byte integer value in the random number file.

[Back to Contents](Contents.md)
