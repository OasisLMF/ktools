# Oasis Kernel

This is the POSIX-compliant Oasis In-Memory Kernel.

### Alpha

This library is in Alpha. 

### Pre-requisites

The g++ compiler needs to be installed in Linux.

### Linux Installation
The code will be installed in ~/bin so please make sure ~/bin exists.
You can create the folder using the command;

``` sh
$ mkdir ~/bin
```

Also you should add this bin folder to your path.
To do this, add the following line to your .bashrc or .bashrc-local file;

``` sh
PATH=~/bin:$PATH
```

Copy ktools.tar.gz onto your machine and unzip.
``` sh
$ tar -zxvf ktools.tar.gz
```

Go into the ktools folder.
``` sh
$ cd /ktools
```

From the ktools folder, configure using the following command;
``` sh
$ ./configure
```

Then make using the following command;
``` sh
$ make
```

Next run the automated test to check the installation and numerical results;
``` sh
$ make check
```
No messages means the test completed successfully.

Finally, copy the executables into your `~/bin` folder using the following command;
``` sh
$ make install
```

The installation is complete.

## Usage

There is sample data and three example scripts which demonstrate how to invoke ktools in the /examples folder. These are written in bash, python and vbscript. 

### Questions/problems?

Email support@oasislmf.org
