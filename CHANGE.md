Ktools Changelog
================

## [v3.0.1](https://github.com/OasisLMF/ktools/releases/tag/v3.0.1)

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

