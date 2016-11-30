########################################
# Derived from ubuntu distribution
########################################
FROM ubuntu:latest
MAINTAINER Boris Glavic <bglavic@iit.edu>
########################################
# Create working directories
########################################
WORKDIR /gprom
WORKDIR examples
########################################
# Install packages needed for building
########################################
RUN apt-get -y update && apt-get install -y \
	autotools-dev \
	autoconf \
	bison \
	flex \
	git \
	graphviz \
	libsqlite3-dev \
	libtool \
	make \
	man-db \
	libreadline6 \
	libreadline6-dev \
	sqlite3 \
	&& rm -rf /var/lib/apt/lists/*
########################################
# Clone GProM github repository
########################################
RUN git clone https://github.com/IITDBGroup/gprom.git /gprom/src
########################################
# Build GProM
########################################
RUN cd /gprom/src \
	&& make -f Makefile.maintainer \
	&& ./configure --disable-oracle --disable-postgres --disable-java \
	&& make \
	&& make install
########################################
# Per default start the gprom command line interface
########################################
ENTRYPOINT ["/usr/local/bin/gprom"]
########################################
# Per default connect to the example SQLite database
########################################
CMD ["-Psqlserializer", "sqlite", "-Pmetadata", "sqlite", "-db", "/gprom/src/examples/test.db"]
