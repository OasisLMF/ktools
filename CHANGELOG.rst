Ktools Changelog
================

.. start_latest_release
`v3.2.3`_
---------
* Added support for ded and min ded only https://github.com/OasisLMF/OasisPlatform/issues/296 https://github.com/OasisLMF/OasisLMF/issues/480
* #23 - Weighted periods documentation
* #99 - Occurrence file format 
* #101 - Remove exposure value from aalcalc output 

.. end_latest_release

`v3.2.2`_
---------
* Fixed potential limit_surplus bug
* Fixed bug in alloc rule 3 and make it a bit faster
* Fixed issue #55 - fmcalc max deductible not being applied for certain samples
* Fixed Cmake build issue + gcc warning messages cleanup 
* Added calcrules 30 and 31 for step policies
* Added calcrule 32 for franchise policies and updated 28
* update for Step policies

`v3.2.1`_
---------

* Added logging for modules to `<run-dir>/log` directory
* Added deductible to calcrule 28
* Added stepped calcrules 29, 14, 100
* Fixes for calcrules 27 and 28

`v3.2.0`_
---------

* Added correlated run feature to Gulcalc
* Added fmcalc allocation rule 2 and 3
* #54 - fmcalc allocrule 2 - final losses can be set to zero if prior level losses are zero
* #57 - Footprint to csv for specific event
* #80 - add support for back-allocation of multi-level layers
* #84 - pltcalc output whitespace
* #86 - first event_id is dropped using summarycalc -i

`v3.1.4`_
---------

* fix usage of zip file in getmodel
* fix periods file in aalcalc
* fix summarycalc error message
* ensure item\_ids are contigious in itemstobin
* add alloc rule 3

`v3.1.3`_
---------

* fix to reduce Gulcalc memory footprint size

`v3.1.2`_
--------
* Fixes to CMake build
* Added validation tools
* Fix for compressed footprint filenames
* fix bug handle loss is zero
* Fix fmcalc for missing samples
* Removed aalcalc check for weights sum to 1.00

`v3.1.1`_
---------

* Fix and binary build for OSX
* Added check in occurrence data
* Fix for exitcode
* Fix a fmcalc seg fault
* Added support for CMake builds


`v3.1.0`_
---------

* New loss stream type for gulcalc added
* Fix item stream processing
* Added support for back allocation to gulcalc
* Made fm and gulcalc streams symmetric

`v3.0.8`_
---------

* Update to aalcalc
* Introduced limit_surplus carry through rules
* fix for min max deductible over or under limit scenario
* Trim summary calc output remove zero exposure values

`v3.0.7`_
---------

* fmcalc - Remove zeros from summarycalc outputs
* fmcalc - Remove conditial use of agg_id as output_id
* Fix build for Ubuntu 16.04

`v3.0.6`_
---------

* fmcalc - Fix for Alloc rules 1 & 2:

`v3.0.5`_
---------

* Fix for Alloc rules [0,1],  Netloss calculation in Reinsurance
* Switch to -O2 compile flag by default, use `./configure --enable-o3` to enable -O3 optimization

`v3.0.3`_
---------

* Performance optimization for Alloc rule 2
* Fix for aalcalc standard deviation
* Added summarycalctobin and removed fptofmcache
* Improved error handling
* Event shuffling to distribute workload been CPU cores


`v3.0.2`_
---------

* Added exception handling for memory allocation errors and segmentation faults.

`v3.0.1`_
---------

* Compatability fix for working on MAC OS

`v3.0.0`_
---------

* New features:
fmcalc supports recursion and extended set of calcrules for reinsurance calculations
fmcalc allocrule now a command line parameter
Filtering of zero loss records from fmcalc by default
aalcalc type 2 standard deviation now represents total sample variance, rather than sample mean variance
* Bug fixes: aalcalc standard deviation calculation corrected for multiple events within a period
* Data formats: new format of fm_profile to support reinsurance calculations
* Test: all fm outputs (zeros removed) and aalcalc md5 changes
* Documentation: Updates for reinsurance
* Other: aalsummary component removed and aalcalc component restructured to run on summarycalc output
Fix example scripts to work with python3 as well as python2
Various performance improvements (additional condition checks and loop reductions, and reduced memory footprint requirements for fmcalc)

`v2.0.3`_
---------

* New features:
kat component added to concatenate multiple file inputs to support pipes
Added Visual Studio projects
Added Visual Studio 64-bit debug build support
Version number added to each component
* Bug fixes:
Handle empty output files
Remove white spaces in output headers
Fixed pltcalc bug for reoccurring event_ids
* Build: Added .gitignore files for easier management of development cycle
* Data formats: events and returnperiods file now require headers in csv.
* Test: minor md5 changes
* Documentation: Added appendix on multi-peril support and minor formatting updates
* Other: build support for docker added

`v2.0.2`_
---------

* New features:
User can supply return period file to leccalc. eltcalc includes analytical mean as type 1 and sample statistics as type 2
* Bug fixes: added support for variable number of layers per aggid in fmcalc. getmodel windows 64 bit i/o issue fixed. In leccalc, interpolation of below range return period losses has been removed and losses are set to zero. aalcalc : type 2 "nan" records removed when run with zero samples.
* Build: None
* Data formats: eltcalc output now has a 'type' field and includes analytical mean records as type 1.
* Test: md5 changes
* Documentation: minor fixes
* Other: None

`v2.0.1`_
---------

* New features: none
* Bug fixes: fmcalc
* Build: fixed error: ‘sqrt’ was not declared in this scope
* Data formats: new coverages.bin format
* Test: md5 changes
* Documentation: minor fixes and added appendix on multi-peril support
* Other: performance enhancements to gulcalc coverage stream, summarycalc and getmodel

`v2.0.0`_
---------

* Four new outputs: event loss tables, loss exceedance curves, average annual loss and period loss tables.
* New component summarycalc supports up to 10 user-defined summary levels per workflow
* New version of getmodel calculates effective damage cdfs from Oasis event footprint and vulnerability model files.
* Substantially revised inputs for model and user data
* Multiple output workflows
* Updated documentation
* Support for native Windows 64-bit executables.

`v1.1.1`_
---------

* Support for Windows 64-bit build added.

`v1.1.0`_
---------

* New fmcalc component and financial module documentation

`v1.0.1`_
---------

* Maintenance release preceding addition of new fmcalc (beta).

`v1.0.0`_
---------

First production release.

Release features:

* Updated documentation
* Automated test for all data conversion tools
* Windows 64 bit executables provided

`v0.5.0`_
---------

Release features:

* Added dbtools for conversion of input data between csv and binary
* Performance improvements for dynamic random numbers, fmcalc and outputcalc
* Changed mean and stdev to sidx =-1 and sidx=-2
* Support for 0 samples -S0 and loss threshold -L in gulcalc
* Debug option for outputting random numbers -d in gulcalc

`v0.4.1`_
---------

* A few bug fixes

`v0.4.0`_
---------

* Update README.md

`v0.0.3-ALPHA`_
---------------

* First Alpha release of the kernel toolkit

.. _`v3.2.3`:  https://github.com/OasisLMF/ktools/compare/v3.2.2...v3.2.3
.. _`v3.2.2`:  https://github.com/OasisLMF/ktools/compare/v3.2.1...v3.2.2
.. _`v3.2.1`:  https://github.com/OasisLMF/ktools/compare/v3.2.0...v3.2.1
.. _`v3.2.0`:  https://github.com/OasisLMF/ktools/compare/v3.1.4...v3.2.0
.. _`v3.1.4`:  https://github.com/OasisLMF/ktools/compare/v3.1.3...v3.1.4
.. _`v3.1.3`:  https://github.com/OasisLMF/ktools/compare/v3.1.2...v3.1.3
.. _`v3.1.2`:  https://github.com/OasisLMF/ktools/compare/v3.1.1...v3.1.2
.. _`v3.1.1`:  https://github.com/OasisLMF/ktools/compare/v3.1.0...v3.1.1
.. _`v3.1.0`:  https://github.com/OasisLMF/ktools/compare/v3.0.8...v3.1.0
.. _`v3.0.8`:  https://github.com/OasisLMF/ktools/compare/v3.0.7...v3.0.8
.. _`v3.0.7`:  https://github.com/OasisLMF/ktools/compare/v3.0.6...v3.0.7
.. _`v3.0.6`:  https://github.com/OasisLMF/ktools/compare/v3.0.5...v3.0.6
.. _`v3.0.5`:  https://github.com/OasisLMF/ktools/compare/v3.0.4...v3.0.5
.. _`v3.0.4`:  https://github.com/OasisLMF/ktools/compare/v3.0.3...v3.0.4
.. _`v3.0.3`:  https://github.com/OasisLMF/ktools/compare/v3.0.2...v3.0.3
.. _`v3.0.2`:  https://github.com/OasisLMF/ktools/compare/v3.0.1...v3.0.2
.. _`v3.0.1`:  https://github.com/OasisLMF/ktools/compare/v3.0.0...v3.0.1
.. _`v3.0.0`:  https://github.com/OasisLMF/ktools/compare/v2.0.3...v3.0.0
.. _`v2.0.3`:  https://github.com/OasisLMF/ktools/compare/v2.0.2...v2.0.3
.. _`v2.0.2`:  https://github.com/OasisLMF/ktools/compare/v2.0.1...v2.0.2
.. _`v2.0.1`:  https://github.com/OasisLMF/ktools/compare/v2.0.0...v2.0.1
.. _`v2.0.0`:  https://github.com/OasisLMF/ktools/compare/v1.1.1...v2.0.0
.. _`v1.1.1`:  https://github.com/OasisLMF/ktools/compare/v1.1.0...v1.1.1
.. _`v1.1.0`:  https://github.com/OasisLMF/ktools/compare/v1.0.1...v1.1.0
.. _`v1.0.1`:  https://github.com/OasisLMF/ktools/compare/v1.0.0...v1.0.1
.. _`v1.0.0`:  https://github.com/OasisLMF/ktools/compare/v0.5.0...v1.0.0
.. _`v0.5.0`:  https://github.com/OasisLMF/ktools/compare/v0.4.1...v1.5.0
.. _`v0.4.1`:  https://github.com/OasisLMF/ktools/compare/v0.4.0...v0.4.1
.. _`v0.4.0`:  https://github.com/OasisLMF/ktools/compare/v0.0.3-ALPHA...v0.4.0
