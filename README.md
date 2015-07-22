# ktools

This is the POSIX-compliant Oasis In-Memory Kernel toolkit.

### Release

Please click [here](https://github.com/OasisLMF/ktools/releases) to download the latest release. 

The source code will change on a regular basis but only the releases are supported. Support enquiries should be sent to support@oasislmf.org.

The instructions for the Windows build below are for 32-bit executables as the Cygwin mingw cross compiler for Windows does not support 64-bit executables. Until an automated build process for 64-bit is made available, the 64-bit Windows executables will be provided with each release.

Note that the dynamic random number option in the Windows build uses a deterministic seed due to a bug in the mingw compiler. We recommend the random number file option (gulcalc -r) should be used in Windows. 

This issue will be handled in future releases by implementing the rdrand random number generator in all environments.

## Linux Installation

### Pre-requisites

The g++ compiler needs to be installed in Linux.

### Instructions

Copy ktools-[version].tar.gz onto your machine and untar.
``` sh
$ tar -xvf ktools-[version].tar.gz
```

Go into the ktools folder and configure using the following command;
``` sh
$ cd ktools-[version]
$ ./configure
```

Make using the following command;
``` sh
$ make
```

Next run the automated test to check the build and numerical results;
``` sh
$ make check
```

Finally, install the executables using the following command;
``` sh
$ make install
```

The installation is complete. The executables are located in ~/usr/local/bin. 

If installing the latest code from the git repository, clone the ktools repository onto your machine.

Go into the ktools folder and configure using the following command;
``` sh
$ cd ktools
$ ./kconfigure
```
Follow the rest of the process as described above.

## Windows Installation

### Pre-requisites
Cygwin is required for the Windows native build.  Cygwin is a Linux environment running in Windows.
http://www.cygwin.com/

Download and run the set-up program for Cygwin. 
The following Cygwin add-in packages are required;
* gcc-g++
* gcc-core
* mingw64-i686-gcc-g++
* mingw64-i686-gcc-core
* make
* diffutils

Search for 'gcc', 'make' and 'diffutils' to find all of the relevant packages (Only 'gcc' illustrated below).
![alt text](https://github.com/OasisLMF/ktools/blob/master/docs/img/cygwin1.jpg "Add-in packages")

### Instructions

Copy ktools-[version].tar.gz onto your machine. 

Open a Cygwin terminal. 

![alt text](https://github.com/OasisLMF/ktools/blob/master/docs/img/cygwin2.jpg "Cygwin terminal")

Change directory to the location of the tar.gz.

Extract the files using the following command
``` sh
$ tar -xvf ktools-[version].tar.gz
```

Go into the ktools folder and configure using the following command;
``` sh
$ cd ktools-[version]
$ ./winconfigure
```

Make using the following command;
``` sh
$ make
```

Next run the automated test to check the build and numerical results;
``` sh
$ make check
```

Finally, install the executables using the following command;
``` sh
$ make install
```

The executables are located in C:/Oasis/bin. You should add this bin folder to your path in System Environment Variables.

![alt text](https://github.com/OasisLMF/ktools/blob/master/docs/img/windowspath.jpg "Adding the path in system environment variables")

The installation is complete. Restart the Cygwin session to refresh the path before usage.

## Usage

There is sample data and three example scripts which demonstrate how to invoke ktools in the /examples folder. These are written in bash, python and vbscript. 
For example, to run the python script, go into the examples folder and run the following command (you must have python installed to run this script.)

``` sh
$ cd examples
$ python concurrent_example.py 
```

### Questions/problems?

Email support@oasislmf.org
