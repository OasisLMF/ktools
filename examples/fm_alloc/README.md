# fmcalc back-allocation test cases

Two small financial structures for testing the fmcalc back-allocation rules (`-a1`, `-a2`,
`-a3`). Both have more than one policy layer and are run by `ktest/runtests.sh`.

## case1 - layers with different loss distributions

Four items, two per location, two coverages per location, two policy layers:

| level | aggregation                    | terms                                                    |
|-------|--------------------------------|----------------------------------------------------------|
| 1     | one agg per item (coverage)    | layer 1 limits items 1 and 3 to 500; layer 2 limits items 2 and 4 to 500 |
| 2     | items 1,2 -> agg 1; 3,4 -> agg 2 (location) | none                                         |
| 3     | all -> agg 1 (account)         | both layers limited to 4000                              |

Ground up losses are 1000, 3000, 3000, 1000 for items 1 to 4.

`fm_xref` output ids are assigned in item then layer order, so output ids 1, 3, 5, 7 are
layer 1 for items 1 to 4 and output ids 2, 4, 6, 8 are layer 2 for items 1 to 4.

Each layer's level 2 losses are:

* layer 1 - location 1 = 500 + 3000 = 3500, location 2 = 500 + 1000 = 1500
* layer 2 - location 1 = 1000 + 500 = 1500, location 2 = 3000 + 500 = 3500

so the two layers distribute their losses over the locations in opposite proportions. The
level 3 limit of 4000 applies to a 5000 loss in both layers, and 4000 is back-allocated:

| output_id | item | layer | -a1  | -a2 and -a3 |
|-----------|------|-------|------|-------------|
| 1         | 1    | 1     | 500  | 400         |
| 2         | 1    | 2     | 500  | 800         |
| 3         | 2    | 1     | 1500 | 2400        |
| 4         | 2    | 2     | 1500 | 400         |
| 5         | 3    | 1     | 1500 | 400         |
| 6         | 3    | 2     | 1500 | 2400        |
| 7         | 4    | 1     | 500  | 800         |
| 8         | 4    | 2     | 500  | 400         |

Alloc rule 1 allocates in proportion to the ground up losses, so both layers give the same
answer. Alloc rules 2 and 3 allocate in proportion to the prior level losses, so each layer
follows its own distribution and the two layers are mirror images of each other.

This is a regression test for [#2055](https://github.com/OasisLMF/OasisLMF/issues/2055),
where alloc rule 2 back-allocated every layer using layer 1's proportions and gave
171.43, 1028.57, 933.33 and 1866.67 for the layer 2 items above. Because layer ids follow
the row order of the account file, that made per item results depend on the order of the
input.

## case2 - a layer with no loss below the top level

Five items, three layers, two levels, with deductibles that leave some level 1
aggregations with no loss in layer 1 but a loss in a later layer. Alloc rule 2 only fills
in the item proportions for layer 1, and skips the aggregations that had no loss, so the
later layers were left with no proportions to allocate with and fmcalc segmentation
faulted. The expected output is pinned for all three allocation rules.
