Ktools Changelog
================

`v3.6.2`_
 ---------
* [#208](https://github.com/OasisLMF/ktools/pull/208) - Added Guthub templates
* [#217](https://github.com/OasisLMF/ktools/pull/218) - Construct Period Average Loss Table (PALT) without SourceNumLocs field
* [#222](https://github.com/OasisLMF/ktools/pull/223) - Bash exit handler not triggering on OOM kill  
* [#215](https://github.com/OasisLMF/ktools/pull/216) - OASIS_FLOAT in kat
.. _`v3.6.2`:  https://github.com/OasisLMF/ktools/compare/v3.6.1...v3.6.2

`v3.6.1`_
 ---------
* [#203](https://github.com/OasisLMF/ktools/pull/204) - Construct Sample Event Loss Table (SELT) without ImpactedNumLocs field
* [#197, #183](https://github.com/OasisLMF/ktools/pull/206) - Feature/197 ordleccalc tests
* [#208](https://github.com/OasisLMF/ktools/pull/208) - Added Guthub templates
* [#209](https://github.com/OasisLMF/ktools/pull/209) - Feature/auto changelog
* [#130](https://github.com/OasisLMF/ktools/pull/210) - Investigate high memory use in leccalc
* [#211, #212](https://github.com/OasisLMF/ktools/pull/213) - Change ensemble.bin directory and drop sample_id column
.. _`v3.6.1`:  https://github.com/OasisLMF/ktools/compare/v3.6.0...v3.6.1

`v3.6.1rc1`_
 ---------
* [#203](https://github.com/OasisLMF/ktools/pull/204) - Construct Sample Event Loss Table (SELT) without ImpactedNumLocs field
* [#197, #183](https://github.com/OasisLMF/ktools/pull/206) - Feature/197 ordleccalc tests
* [#208](https://github.com/OasisLMF/ktools/pull/208) - Added Guthub templates
* [#209](https://github.com/OasisLMF/ktools/pull/209) - Feature/auto changelog
* [#130](https://github.com/OasisLMF/ktools/pull/210) - Investigate high memory use in leccalc
* [#211, #212](https://github.com/OasisLMF/ktools/pull/213) - Change ensemble.bin directory and drop sample_id column
.. _`v3.6.1rc1`:  https://github.com/OasisLMF/ktools/compare/v3.6.0...v3.6.1rc1

.. * [#_](https://github.com/OasisLMF/OasisLMF/issues/_) -  

`v3.6.0`_
-------------
.. start_latest_release
* [#184](https://github.com/OasisLMF/ktools/issues/184) - Calculate Tail Value at Risk (TVaR)
* [#186](https://github.com/OasisLMF/ktools/issues/186) - Fix truncation of outer return periods in leccalc
* [#193](https://github.com/OasisLMF/ktools/issues/193) - Fix areaperil ID output in cdftocsv
* [#195](https://github.com/OasisLMF/ktools/pull/195) - Write Exceedance Probability Table (EPT) and Per Sample Exceedance Probability Table (PSEPT)
* [#196](https://github.com/OasisLMF/ktools/issues/196) - Reduce memory footprint in aalcalc
.. end_latest_release

`v3.5.1`_
---------
* [#159](https://github.com/OasisLMF/ktools/issues/159) - Bring over/under limit logic in fmcalc in line with fmpy

`v3.5.0`_
---------
* [#36](https://github.com/OasisLMF/ktools/issues/36) - Sort eltcalc output with kat
* [#119](https://github.com/OasisLMF/ktools/issues/119) - Add options to partition events between multiple processes deterministically or using Fisher-Yates shuffle
* [#180](https://github.com/OasisLMF/ktools/pull/180) - Evenly distribute events among processes

`v3.4.3`_
---------
* [#168](https://github.com/OasisLMF/ktools/issues/168) - Remove unrequired lines in getmodel output
* [#172](https://github.com/OasisLMF/ktools/issues/172) - Better error reporting and handling of partial output in eltcalc
* [#174](https://github.com/OasisLMF/ktools/issues/174) - Better error reporting and handling of partial output in aalcalc and leccalc
* [#181](https://github.com/OasisLMF/ktools/issues/181) - Correction to Wheatsheaf Mean output in leccalc

`v3.4.2`_
---------
* [#175](https://github.com/OasisLMF/ktools/issues/175) - Calculate accumulated TIV without duplication of TIVs for multi-peril models
* [#176](https://github.com/OasisLMF/ktools/issues/176) - Include TIVs for coverages where there are no input losses

`v3.4.1`_
---------
* [#38](https://github.com/OasisLMF/ktools/issues/38) - Drop interval type column from damage bin dictionary
* [#155](https://github.com/OasisLMF/ktools/issues/155) - Provide support for compressed and indexed vulnerability files
* [#165](https://github.com/OasisLMF/ktools/issues/165) - Introduce option to write original data size to footprint index file

`v3.4.0`_
---------
* [#154](https://github.com/OasisLMF/ktools/issues/154) - Improve performance of getmodel when there is uncertainty in the footprint file
* [#160](https://github.com/OasisLMF/ktools/issues/160) - Add stream type argument to gultobin
* Implement step policies from loss stream in fmcalc

`v3.3.8`_
---------
* [#98](https://github.com/OasisLMF/ktools/issues/98) - Fix integer overflow problem in leccalc
* [#114](https://github.com/OasisLMF/ktools/issues/114) - Remove records where interpolated return period loss runs of bounds in leccalc

`v3.3.7`_
---------
* [#148](https://github.com/OasisLMF/ktools/issues/148) - Fix gulcalc performance issues

`v3.3.6`_
---------
* [#91](https://github.com/OasisLMF/ktools/issues/91) - Add gulcalc alloc rule 2 where total peril loss = maximum subperil loss
* [#606](https://github.com/OasisLMF/OasisLMF/issues/606) - Add calcrules 19, 26, 35 and 36

`v3.3.5`_
---------
* [#123](https://github.com/OasisLMF/ktools/issues/123) - handle over limit - under limit when some prior losses are unlimited
* Updated FM diagrams and clarified hierarchy in Financial Module documentation

`v3.3.4`_
---------
* [#397](https://github.com/OasisLMF/OasisPlatform/issues/397) - Calculate metrics by ensemble ID in aalcalc and leccalc
* Add ensembletobin and ensembletocsv executables to convert file containing ensemble IDs between csv and binary formats

`v3.3.3`_
---------
* [#131](https://github.com/OasisLMF/ktools/issues/131) - Changed missing intensity bins error to warning in validatevulnerability
* [#133](https://github.com/OasisLMF/ktools/issues/133) - Better handling of invalid vulnerability IDs in getmodel
* [#134](https://github.com/OasisLMF/ktools/issues/134) - Fixed issue with fully correlated output in gulcalc when using alloc rule 0

`v3.3.2`_
---------
* [#566](https://github.com/OasisLMF/OasisLMF/issues/566) - Handle unlimited LayerLimit without large default value
* [#578](https://github.com/OasisLMF/OasisLMF/issues/578) - Missing combination of terms in calcrules to add

`v3.3.1`_
---------

* [#117](https://github.com/OasisLMF/ktools/issues/117) - Reduced gulcalc memory use for gul alloc rule 1
* [#127](https://github.com/OasisLMF/ktools/issues/127) - Introduce support for full correlation output for gul alloc rule 0


`v3.3.0`_
---------
* [#124](https://github.com/OasisLMF/ktools/issues/124) - Update CMake build files   
* [#103](https://github.com/OasisLMF/ktools/issues/103) - Fixed issue with leccalc and the periods file
* [#120](https://github.com/OasisLMF/ktools/issues/120) - Add calcrule for ded % loss and normal limit


`v3.2.6`_
---------
* #121 - Move footprint file open/close to outside of event loop


`v3.2.5`_
---------
* #111 - Add maximum intensity bin checks to validatevulnerability
* #115 - Fix integer overflow issue with file validation
* #116 - Fix full uncertainty and Wheatsheaf with period weighting calculations in leccalc


`v3.2.4`_
---------
* #105 - Update readme build instructions for OS X
* #106 - Fix weighted standard deviation in aalcalc
* #109 - Fix gulcalc loss stream header for alloc rule 0

`v3.2.3`_
---------
* Added support for ded and min ded only https://github.com/OasisLMF/OasisPlatform/issues/296 https://github.com/OasisLMF/OasisLMF/issues/480
* #23 - Weighted periods documentation
* #99 - Occurrence file format 
* #101 - Remove exposure value from aalcalc output 

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
---------
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


.. _`v3.6.0`:  https://github.com/OasisLMF/ktools/compare/v3.5.1...v3.6.0
.. _`v3.5.1`:  https://github.com/OasisLMF/ktools/compare/v3.5.0...v3.5.1
.. _`v3.5.0`:  https://github.com/OasisLMF/ktools/compare/v3.4.3...v3.5.0
.. _`v3.4.3`:  https://github.com/OasisLMF/ktools/compare/v3.4.2...v3.4.3
.. _`v3.4.2`:  https://github.com/OasisLMF/ktools/compare/v3.4.1...v3.4.2
.. _`v3.4.1`:  https://github.com/OasisLMF/ktools/compare/v3.4.0...v3.4.1
.. _`v3.4.0`:  https://github.com/OasisLMF/ktools/compare/v3.3.8...v3.4.0
.. _`v3.3.8`:  https://github.com/OasisLMF/ktools/compare/v3.3.7...v3.3.8
.. _`v3.3.7`:  https://github.com/OasisLMF/ktools/compare/v3.3.6...v3.3.7
.. _`v3.3.6`:  https://github.com/OasisLMF/ktools/compare/v3.3.5...v3.3.6
.. _`v3.3.5`:  https://github.com/OasisLMF/ktools/compare/v3.3.4...v3.3.5
.. _`v3.3.4`:  https://github.com/OasisLMF/ktools/compare/v3.3.3...v3.3.4
.. _`v3.3.3`:  https://github.com/OasisLMF/ktools/compare/v3.3.2...v3.3.3
.. _`v3.3.2`:  https://github.com/OasisLMF/ktools/compare/v3.3.1...v3.3.2
.. _`v3.3.1`:  https://github.com/OasisLMF/ktools/compare/v3.3.0...v3.3.1
.. _`v3.3.0`:  https://github.com/OasisLMF/ktools/compare/v3.2.6...v3.3.0
.. _`v3.2.6`:  https://github.com/OasisLMF/ktools/compare/v3.2.4...v3.2.6
.. _`v3.2.5`:  https://github.com/OasisLMF/ktools/compare/v3.2.4...v3.2.5
.. _`v3.2.4`:  https://github.com/OasisLMF/ktools/compare/v3.2.3...v3.2.4
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
