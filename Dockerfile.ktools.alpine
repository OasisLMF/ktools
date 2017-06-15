# Set the base image
FROM alpine 

RUN apk --no-cache add bash
RUN apk --no-cache add autoconf
RUN apk --no-cache add git
RUN apk --no-cache add automake
RUN apk --no-cache add libtool
#RUN apk --no-cache add gcc
RUN apk --no-cache add g++
RUN apk --no-cache add make
RUN apk --no-cache add diffutils
RUN apk --no-cache add zlib-dev
RUN mkdir ktools 
COPY . ktools

RUN cd /ktools && ./autogen.sh && ./configure && make && make install && rm -rf ..?* .[!.]* *

RUN cd / && rmdir /ktools

RUN apk --no-cache del zlib-dev diffutils make g++ gcc libtool automake git autoconf

MAINTAINER Ben Matharu <neb@sky.com> 
