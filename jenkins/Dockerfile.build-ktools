FROM gcc:latest

RUN apt upgrade -y && rm -rf /var/lib/apt/lists/*
WORKDIR /var/ktools
COPY . /var/ktools
COPY ./jenkins/entrypoint_build.sh /usr/local/bin/build.sh
ENTRYPOINT build.sh
