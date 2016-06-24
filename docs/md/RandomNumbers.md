# Appendix A: Random numbers <a id="randomnumbers"></a>

Simple uniform random numbers are assigned to each event, group and sample number to sample ground up loss in the gulcalc process. A group is a collection of items which share the same group_id, and is the method of supporting spatial correlation in ground up loss sampling in Oasis and ktools.

#### Correlation

Items (typically representing, in insurance terms, the underlying risk coverages) that are assigned the same group_id will use the same random number to sample damage for a given event and sample number. Items with different group_ids will be assigned independent random numbers.  Therefore sampled damage is fully correlated within groups and fully independent between groups, where group is an abstract collection of items defined by the user.

The item_id, group_id data is provided by the user in the items input file (items.bin).

### Methodology

The method of assigning random numbers in gulcalc uses an random number index (ridx) which is used as a position reference into a list of random numbers.  S random numbers corresponding to the runtime number of samples are drawn from the list starting at the first ridx position.

There are two options in ktools for choosing random numbers to apply in the sampling process.

#### 1. Generate dynamically during the calculation

##### Usage
Use -R{number of random numbers} as a parameter.

##### Example
```
$ gulcalc -S00 -R1000000 -i -
```
This will run 100 samples drawing from 1 million dynamically generated random numbers.

##### Method

Random numbers are sampled dynamically using the Mersenne twister psuedo random number generator (the default RNG of the C++ v11 compiler). 
A sparse array capable of holding R million random numbers is allocated to each event. The ridx is generated from the group_id and number of samples S using the following modulus function;

ridx= mod(group_id x P1, R)

* P1 is the first prime number which is bigger than the number of samples, S.

This formula pseudo-randomly assigns ridx indexes to each group_id between 0 and 999,999. 

As a ridx is sampled, the section in the array starting at the ridx position of length S is populated with random numbers unless they have already been populated, in which case the existing random numbers are re-used.

The array is cleared for the next event and a new set of random numbers is generated.  

#### 2. Use numbers from random number file

##### Usage
Use -r as a parameter

##### Example
```
$ gulcalc -S100 -r -i -
```
This will run 100 samples using random numbers from file random.bin in the static sub-directory.

##### Method
The random number file(s) is read into memory at the start of the gulcalc process. 

The ridx is generated from the sample index (sidx), event_id and group_id using the following modulus function;

ridx= sidx + mod(group_id x P1 x P3 + event_id x P2, R)

* R is the divisor of the modulus, equal to the total number of random numbers in the list.
* P1 and P2 are the first two prime numbers which are greater than half of R.
* P3 is the first prime number which is bigger than the number of samples, S.

This formula pseudo-randomly assigns a starting position index to each event_id and group_id combo between 0 and R-1, and then S random numbers are drawn by incrementing the starting position by the sidx.

[Return to top](#randomnumbers)

[Go to Appendix B FM Profiles](fmprofiles.md)

[Back to Contents](Contents.md)
