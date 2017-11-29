Test wrapper on FMCalcs with friendlier data structures and illustrative
reinsurance functaionality.

To run the example, use the following command. This assumes that the latest version of ktools is installed.

  python ri_tests.py -n test1 -p policies.json -r1 ri_1.json -r2 ri_2.json

The test run generates the ILs for the test portfolio (policies.json), then calculates the losses net of a location fac treaty (ri_1.json), then calculates the losses net of a cat treaty (r1_2.json).
