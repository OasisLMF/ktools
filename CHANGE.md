Ktools Changelog
================

## [v3.1.1](https://github.com/OasisLMF/ktools/releases/tag/v3.1.1)
* Fix and binary build for OSX
* Added check in occurrence data
* Fix for exitcode 
* Fix a fmcalc seg fault
* Added support for CMake builds


## [v3.1.0](https://github.com/OasisLMF/ktools/releases/tag/v3.1.0)
* New loss stream type for gulcalc added
* Fix item stream processing
* Added support for back allocation to gulcalc
* Made fm and gulcalc streams symmetric 

## [v3.0.8](https://github.com/OasisLMF/ktools/releases/tag/v3.0.8)
* Update to aalcalc
* Introduced limit_surplus carry through rules
* fix for min max deductible over or under limit scenario
* Trim summary calc output remove zero exposure values

## [v3.0.7](https://github.com/OasisLMF/ktools/releases/tag/v3.0.7)
* fmcalc - Remove zeros from summarycalc outputs
* fmcalc - Remove conditial use of agg_id as output_id
* Fix build for Ubuntu 16.04

## [v3.0.6](https://github.com/OasisLMF/ktools/releases/tag/v3.0.6)
* fmcalc - Fix for Alloc rules 1 & 2: 

## [v3.0.5](https://github.com/OasisLMF/ktools/releases/tag/v3.0.5)
* Fix for Alloc rules [0,1],  Netloss calculation in Reinsurance 
* Switch to -O2 compile flag by default, use `./configure --enable-o3` to enable -O3 optimization 

## [v3.0.3](https://github.com/OasisLMF/ktools/releases/tag/v3.0.3)
* Performance optimization for Alloc rule 2
* Fix for aalcalc standard deviation
* Added summarycalctobin and removed fptofmcache
* Improved error handling
* Event shuffling to distribute workload been CPU cores 


## [v3.0.2](https://github.com/OasisLMF/ktools/releases/tag/v3.0.2)
* Added exception handling for memory allocation errors and segmentation faults.

## [v3.0.1](https://github.com/OasisLMF/ktools/releases/tag/v3.0.1)
* Compatability fix for working on MAC OS
## [v3.0.0](https://github.com/OasisLMF/ktools/releases/tag/v3.0.0)

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

## [Release 2.0.3](https://github.com/OasisLMF/ktools/releases/tag/2.0.3)

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

## [Release 2.0.2](https://github.com/OasisLMF/ktools/releases/tag/2.0.2)

* New features:
User can supply return period file to leccalc. eltcalc includes analytical mean as type 1 and sample statistics as type 2
* Bug fixes: added support for variable number of layers per aggid in fmcalc. getmodel windows 64 bit i/o issue fixed. In leccalc, interpolation of below range return period losses has been removed and losses are set to zero. aalcalc : type 2 "nan" records removed when run with zero samples.
* Build: None
* Data formats: eltcalc output now has a 'type' field and includes analytical mean records as type 1.
* Test: md5 changes
* Documentation: minor fixes
* Other: None

## [Release 2.0.1](https://github.com/OasisLMF/ktools/releases/tag/2.0.1)

* New features: none
* Bug fixes: fmcalc
* Build: fixed error: ‘sqrt’ was not declared in this scope
* Data formats: new coverages.bin format
* Test: md5 changes
* Documentation: minor fixes and added appendix on multi-peril support
* Other: performance enhancements to gulcalc coverage stream, summarycalc and getmodel

## [Release 2.0.0](https://github.com/OasisLMF/ktools/releases/tag/2.0.0)

* Four new outputs: event loss tables, loss exceedance curves, average annual loss and period loss tables.
* New component summarycalc supports up to 10 user-defined summary levels per workflow
* New version of getmodel calculates effective damage cdfs from Oasis event footprint and vulnerability model files.
* Substantially revised inputs for model and user data
* Multiple output workflows
* Updated documentation
* Support for native Windows 64-bit executables.

## [Release 1.1.1](https://github.com/OasisLMF/ktools/releases/tag/1.1.1)

* Support for Windows 64-bit build added.

## [Release 1.1.0](https://github.com/OasisLMF/ktools/releases/tag/1.1.0)

* New fmcalc component and financial module documentation

## [Release 1.0.1](https://github.com/OasisLMF/ktools/releases/tag/1.0.1)

* Maintenance release preceding addition of new fmcalc (beta).

## [Release 1.0.0](https://github.com/OasisLMF/ktools/releases/tag/1.0.0)

First production release.

Release features:

* Updated documentation
* Automated test for all data conversion tools
* Windows 64 bit executables provided

## [Release 0.5.0](https://github.com/OasisLMF/ktools/releases/tag/0.5.0)

Release features:

* Added dbtools for conversion of input data between csv and binary
* Performance improvements for dynamic random numbers, fmcalc and outputcalc
* Changed mean and stdev to sidx =-1 and sidx=-2
* Support for 0 samples -S0 and loss threshold -L in gulcalc
* Debug option for outputting random numbers -d in gulcalc

## [Release 0.4.1](https://github.com/OasisLMF/ktools/releases/tag/0.4.1)

* A few bug fixes

## [Release 0.4.0](https://github.com/OasisLMF/ktools/releases/tag/0.4.0)

* Update README.md

## 0.0.3-ALPHA

* First Alpha release of the kernel toolkit 

