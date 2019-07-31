![alt text](../img/banner.jpg "banner")
# Appendix B: FM Profiles <a id="fmprofiles"></a>

This section specifies the attributes and rules for the following list of Financial module profiles.


| Profile description                               |calcrule_id |d1   |d2   |d3   |a1   |l1   |s1   |s2   |s3   |
|:--------------------------------------------------|------------|-----|-----|-----|-----|-----|-----|-----|----:|
|deductible and limit                               |   1        |x    |     |     |     |x    |     |     |     |
|deductible with attachment, limit and share        |   2        |x    |     |     |x    |x    |x    |     |     |
|franchise deductible and limit                     |   3        |x    |     |     |     |x    |     |     |     |
|deductible % TIV and limit                         |   4        |x    |     |     |     |x    |     |     |     |
|deductible and limit % loss                        |   5        |x    |     |     |     |x    |     |     |     |
|deductible % TIV                                   |   6        |x    |     |     |     |     |     |     |     |
|limit and maximum deductible                       |   7        |     |     |x    |     |x    |     |     |     |
|limit and minimum deductible                       |   8        |     |x    |     |     |x    |     |     |     |
|limit with deductible % limit                      |   9        |x    |     |     |     |x    |     |     |     |
|maximum deductible                                 |   10       |     |     |x    |     |     |     |     |     |
|minimum deductible                                 |   11       |     |x    |     |     |     |     |     |     |
|deductible                                         |   12       |x    |     |     |     |     |     |     |     |
|minimum and maximum deductible                     |   13       |     |x    |x    |     |     |     |     |     |
|limit only                                         |   14       |     |     |     |     |x    |     |     |     |
|limit % loss                                       |   15       |     |     |     |     |x    |     |     |     |
|deductible % loss                                  |   16       |x    |     |     |     |     |     |     |     |
|deductible % loss with attachment, limit and share |   17       |x    |     |     |x    |x    |x    |     |     |
|deductible % tiv with attachment, limit and share  |   18       |x    |     |     |x    |x    |x    |     |     |
|% loss deductible with min and max deductible      |   19       |x    |x    |x    |     |     |     |     |     |
|reverse franchise deductible                       |   20       |x    |     |     |     |     |     |     |     |
|% tiv deductible with min and max deductible       |   21       |x    |x    |x    |     |     |     |     |     |
|reinsurance % ceded, limit and % placed            |   22       |     |     |     |     |x    |x    |x    |x    |
|reinsurance limit and % placed                     |   23       |     |     |     |     |x    |     |x    |x    |
|reinsurance excess terms                           |   24       |     |     |     |x    |x    |x    |x    |x    |
|reinsurance proportional terms                     |   25       |     |     |     |     |     |x    |x    |x    |
|deductible amount with min and max deductible      |   26       |x    |x    |x    |     |     |     |     |     |

The fields with an x are those which are required by the profile. The full names of the fields are as follows;


| Short name | Profile field name  |
|:-----------|--------------------:|
|     d1     |     deductible_1    |
|     d2     |     deductible_2    |
|     d3     |     deductible_3    |
|     a1     |     attachment_1    |
|     l1     |       limit_1       |
|     s1     |     share_1         |
|     s2     |     share_2         |
|     s3     |     share_3         |

An allocation rule can be assigned to each call of fmcalc, which determines whether calculated losses should be back-allocated to the contributing items, and if so how. This is specified via the command line parameter -a. 

The choices are as follows;

| Allocrule description                                              | allocrule_id|
|:-------------------------------------------------------------------|------------:|
| Don't back-allocate losses (default if no parameter supplied)      | 0           |
| Back allocate losses to items in proportion to input loss          | 1           |
| Back-allocate losses to items in proportion to prior level loss    | 2           |

## Effective deductibles

Often there are more than one hierarchal levels with deductibles, and there is a choice of methods of accumulation of deductibles through the hierarchy. Whenever a rule with a deductible is used in the loss calculation then it is accumulated through the calculation in an **effective_deductible** variable. The effective deductible is the smaller of the deductible amount and the loss. 

All deductibles amounts calculated from the deductible_1 field are simply additive through the hierarchy.

Ay any level, the user can specify a calcrule using a minimum and/or maximum deductible which changes the way that effective deductibles are accumulated. 

For a minimum deductible specified in calcrules using the deductible_2 field, the calculation increases the effective_deductible carried forward from the previous levels up to the minimum deductible if it is smaller. 

For a maximum deductible specified in calcrules using the deductible_3 field, the calculation decreases the effective_deductible carried forward from the previous levels down to the maximum deductible if it is larger.

## Calcrules

In the following notation;
* x.loss is the input loss to the calculation
* x.effective_deductible is the input effective deductible to the calculation (where required)
* loss is the calculated loss 


### 1. Deductible and limit

| Attributes                        | Example |
|:----------------------------------|--------:|
| policytc_id                       | 1       |
| calcrule_id                       | 1       |
| deductible_1                      | 50000   |
| limit_1                           | 900000  |


##### Calculation logic
``` sh
loss = x.loss - deductible_1;
if (loss < 0) loss = 0;
if (loss > limit_1) loss = limit_1;
```

### 2. Deductible, attachment, limit and share

| Attributes                        | Example |
|:----------------------------------|--------:|
| policytc_id                       | 1       |
| calcrule_id                       | 2       | 
| deductible_1                      | 70000   |
| attachment_1                      | 0       |
| limit_1                           | 1000000 |
| share_1                           | 0.1     |

##### Calculation logic

``` sh
loss = x.loss - deductible_1
if (loss < 0) loss = 0;
if (loss > (attachment_1 + limit_1)) loss = limit_1;
	else loss = loss - attachment_1;
if (loss < 0) loss = 0;
loss = loss * share_1;
```

### 3. Franchise deductible and limit

| Attributes                        | Example |
|:----------------------------------|--------:|
| policytc_id                       | 1       |
| calcrule_id                       | 3       | 
| deductible_1                      | 100000  |
| limit_1                           | 1000000 |


##### Calculation logic

``` sh
if (x.loss < deductible_1) loss = 0;
	else loss = x.loss;
if (loss > limit_1) loss = limit_1;
```

### 5. Deductible and limit as a proportion of loss

| Attributes                        | Example |
|:----------------------------------|--------:|
| policytc_id                       | 1       |
| calcrule_id                       | 5       | 
| deductible_1                      | 0.05    |
| limit_1                           | 0.3     |


##### Calculation logic

``` sh
loss = x.loss - (x.loss * deductible_1);
if (loss > (x.loss * limit_1)) loss = x.loss * lim;
```


### 9. Limit with deductible as a proportion of limit

| Attributes                        | Example |
|:----------------------------------|--------:|
| policytc_id                       | 1       |
| calcrule_id                       | 9       | 
| deductible_1				        | 0.05    |
| limit_1                           | 100000  |


##### Calculation logic

``` sh
loss = x.loss - (deductible_1 * limit_1);
if (loss < 0) loss = 0;
if (loss > limit_1) loss = limit_1;
```

### 10. Maximum deductible

If the effective deductible carried forward from the previous level exceeds the maximum deductible, the effective deductible is decreased to the maximum deductible value

| Attributes                        | Example |
|:----------------------------------|--------:|
| policytc_id                       | 1       |
| calcrule_id                       | 10      |
| deductible_3                      | 40000   |
 

##### Calculation logic

``` sh
if (x.effective_deductible > deductible_3) { 
	loss = x.loss + x.effective_deductible - deductible_3;
	if (loss < 0) loss = 0;
	}
else {
	loss = x.loss;
     }
```

### 11. Minimum deductible

If the effective deductible carried forward from the previous level is less than the minimum deductible, the deductible is increased to the total loss or the minimum deductible value, whichever is greater.

| Attributes                        | Example |
|:----------------------------------|--------:|
| policytc_id                       | 1       |
| calcrule_id                       | 11      |
| deductible_2                      | 70000   |


##### Calculation logic

``` sh
if (x.effective_deductible < deductible_2) { 
	loss = x.loss + x.effective_deductible - deductible_2;
	if (loss < 0) loss = 0;
	}
else {
	loss = x.loss;
     }
```


### 12. Deductible only

| Attributes                        | Example |
|:----------------------------------|--------:|
| policytc_id                       | 1       |
| calcrule_id                       | 12      | 
| deductible_1                      | 100000  |


##### Calculation logic

``` sh
loss = x.loss - deductible_1;
if (loss < 0) loss = 0;
```

### 14. Limit only

| Attributes                        | Example |
|:----------------------------------|--------:|
| policytc_id                       | 1       |
| calcrule_id                       | 14      | 
| limit                             | 100000  |

##### Calculation logic

``` sh
loss = x.loss;
if (loss > limit_1) loss = limit_1;
```

### 15. Limit as a proportion of loss

| Attributes                        | Example |
|:----------------------------------|--------:|
| policytc_id                       | 1       |
| calcrule_id                       | 15      | 
| limit_1                           | 0.3     |

##### Calculation logic

``` sh
loss = x.loss * limit_1;
```

### 16. Deductible as a proportion of loss

| Attributes                        | Example |
|:----------------------------------|--------:|
| policytc_id                       | 1       |
| calcrule_id                       | 16      | 
| deductible_1                      | 0.05    |


##### Calculation logic

``` sh
loss = x.loss - (x.loss * deductible_1);
if (loss < 0) loss = 0;
```
[Return to top](#fmprofiles)

[Go to Appendix C Multi-peril model support](MultiPeril.md)

[Back to Contents](Contents.md)
