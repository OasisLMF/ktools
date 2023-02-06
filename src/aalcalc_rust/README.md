# AAL Calc
In this module calculates the average annual loss in Rust. 


## Summaries
A summary is merely a collection of events 


## Standard Deviation Calculation 
We calculate the standard deviation starting with our sum definitions from samples:

$ sum = \sum_{i=1}^{n} s_i $

$ sum^2 = \sum_{i=1}^{n} s_i^2 $

We calculate the mean with the following:

$ mean = \displaystyle\frac{sum}{\displaystyle{N_{perils} * N_{samples}} } $

We can then calculate the standard deviation with the following:

$ stdd = \displaystyle{sum^2 - {\displaystyle{sum * sum} \over\displaystyle \displaystyle{N_{perils} * N_{samples}} } \over\displaystyle N_{perils} * N_{samples} - 1 } $


## Standard deviation with period weights
When it comes to weighting the periods, we need to apply weights to each event giving the following sums:

$ sum = (E_1 + E_7 . . .)w_1 + . . . $
$ sum = (E_1 + E_7 . . .)^2w_1 + . . . $



##### Math reference sheet:
https://csrgxtu.github.io/2015/03/20/Writing-Mathematic-Fomulars-in-Markdown/