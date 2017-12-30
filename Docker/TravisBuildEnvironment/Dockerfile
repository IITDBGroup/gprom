########################################
# Derived from ubuntu distribution
########################################
FROM sergeymakinen/oracle-instant-client:12.2
MAINTAINER Boris Glavic <bglavic@iit.edu>
########################################
# Create working directories
########################################
WORKDIR /gprom
########################################
# Install monetdb repository
########################################
RUN apt-get -y update && apt-get -y install apt-transport-https software-properties-common wget
RUN echo "deb http://dev.monetdb.org/downloads/deb/ xenial monetdb" > /etc/apt/sources.list.d/monetdb.list \
    && 	echo "deb-src http://dev.monetdb.org/downloads/deb/ xenial monetdb" >> /etc/apt/sources.list.d/monetdb.list \
	&& wget -q --output-document=- https://www.monetdb.org/downloads/MonetDB-GPG-KEY | apt-key add -
########################################
# Install Java 8
########################################
RUN echo oracle-java8-installer shared/accepted-oracle-license-v1-1 select true | debconf-set-selections && \
  add-apt-repository -y ppa:webupd8team/java && \
  apt-get update && \
  apt-get install -y oracle-java8-installer && \
  rm -rf /var/lib/apt/lists/* && \
  rm -rf /var/cache/oracle-jdk8-installer
RUN apt install oracle-java8-set-default
ENV JAVA_VER 8
ENV JAVA_HOME /usr/lib/jvm/java-8-oracle
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
	libreadline6 \
	libreadline6-dev \
	libsqlite3-dev \
	libtool \
	make \
	man-db \
	sqlite3 \
	postgresql \
	libpq-dev \
	rman \
	pandoc \
	vim \
	unzip \
	monetdb5-sql \
	monetdb-client \
	libmonetdb-client-dev \
	&& rm -rf /var/lib/apt/lists/*
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
	&& ./configure --with-oracle-headers-path=/usr/local/oracle/include/oracle/12.2/client64/ \
	&& make \
	&& make install \
	&& echo "/usr/local/bin/" > /etc/ld.so.conf.d/ocilib.conf \
	&& /sbin/ldconfig
