FROM coreoasis/oasis_base:latest

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y gcc g++ build-essential make libtool automake autoconf zlib1g-dev && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /var/oasis/ktools
COPY ./ ./
RUN sh ./autogen.sh; sh ./configure
RUN make; make check; make install
