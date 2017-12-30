########################################
# Derived from ubuntu distribution
########################################
FROM collinestes/docker-node-oracle
MAINTAINER Boris Glavic <bglavic@iit.edu>
########################################
# Create working directories
########################################
WORKDIR /gprom
########################################
# Install Java 8
########################################
RUN apt-get -y update && apt-get -y install \
	apt-transport-https \
	wget
RUN echo "deb http://http.debian.net/debian jessie-backports main" >> /etc/apt/sources.list.d/jessie-backports.list \
	&& apt-get -y update \
    && apt-get -y install -t jessie-backports openjdk-8-jdk \
	&& update-java-alternatives -s java-1.8.0-openjdk-amd64
########################################
# Install monetdb repository
########################################
RUN echo "deb http://dev.monetdb.org/downloads/deb/ jessie monetdb" > /etc/apt/sources.list.d/monetdb.list \
    && 	echo "deb-src http://dev.monetdb.org/downloads/deb/ jessie monetdb" >> /etc/apt/sources.list.d/monetdb.list \
	&& wget -q --output-document=- https://www.monetdb.org/downloads/MonetDB-GPG-KEY | apt-key add -
########################################
# Install packages needed for building
########################################
RUN apt-get -y update && apt-get install -y \
	ant \
	autotools-dev \
	autoconf \
	automake \
	bison \
	flex \
	git \
	graphviz \
	libpq-dev \
	libreadline6 \
	libreadline6-dev \
	libsqlite3-dev \
	libtool \
	make \
	man-db \
	sqlite3 \
	monetdb5-sql \
	monetdb-client \
	libmonetdb-client-dev \
	&& rm -rf /var/lib/apt/lists/*
ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64
########################################
# Setup MonetDB
########################################
RUN usermod -a -G monetdb root
########################################
# Clone OCILIB
########################################
RUN git clone https://github.com/vrogier/ocilib.git /ocilib/src
########################################
# Build OCILIB
########################################
RUN cd /ocilib/src \
	&& sh reconf \
	&& ./configure --with-oracle-headers-path=/opt/oracle/instantclient/sdk/include --with-oracle-lib-path=/opt/oracle/instantclient \
	&& make \
	&& make install \
	&& echo "/usr/local/bin/" >> /etc/ld.so.conf \
	&& echo "/opt/oracle/instantclient/" >> /etc/ld.so.conf \
	&& /sbin/ldconfig
########################################
# Per default run bash
########################################
ENTRYPOINT ["/bin/bash"]
