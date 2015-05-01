# ktools

This is the POSIX-compliant Oasis In-Memory Kernel SDK.

### Beta

This library is in Beta. 

## Linux Installation

### Pre-requisites

The g++ compiler needs to be installed in Linux.

### Instructions

If installing from a release tar.gz, copy ktools-[version].tar.gz onto your machine and untar.
``` sh
$ tar -xvf ktools-[version].tar.gz
```

Go into the ktools folder and configure using the following command;
``` sh
$ cd ktools-[version]
$ ./configure
```
If installing from a git repository, clone the ktools repository onto your machine.

Go into the ktools folder and configure using the following command;
``` sh
$ cd ktools
$ ./kconfigure
```

After the configure stage for either source, make using the following command;
``` sh
$ make
```

Next run the automated test to check the build and numerical results;
``` sh
$ make check
```
No messages means the test completed successfully.

Finally, install the executables using the following command;
``` sh
$ make install
```

The installation is complete. The executables are located in ~/usr/local/bin. 

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

![alt text](https://github.com/OasisLMF/ktools/blob/master/docs/img/cygwin1.jpg "Add-in packages")

### Instructions

If installing from a release tar.gz, copy ktools-[version].tar.gz onto your machine and untar.
``` sh
$ tar -xvf ktools-[version].tar.gz
```

Go into the ktools folder and configure using the following command;
``` sh
$ cd ktools-[version]
$ ./winconfigure
```
If installing from a git repository, clone the ktools repository onto your machine.

Go into the ktools folder and configure using the following command;
``` sh
$ cd ktools
$ ./winconfigure
```

After the configure stage for either source, make using the following command;
``` sh
$ make
```

Next run the automated test to check the build and numerical results;
``` sh
$ make check
```
No messages means the test completed successfully.

Finally, install the executables using the following command;
``` sh
$ make install
```

The executables are located in C:/Oasis/bin. You should add this bin folder to your path in System Environment Variables.

![alt text](https://github.com/OasisLMF/ktools/blob/master/docs/img/windowspath.jpg "Adding the path in system environment variables")

The installation is complete.

## Usage

There is sample data and three example scripts which demonstrate how to invoke ktools in the /examples folder. These are written in bash, python and vbscript. 
For example, to run the python script, go into the examples folder and run the following command (you must have python installed to run this script.)

``` sh
$ cd examples
$ python concurrent_example.py 
```

### Questions/problems?

Email support@oasislmf.org
