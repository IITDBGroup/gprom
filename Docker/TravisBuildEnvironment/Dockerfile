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
	autoconf \
	automake \
	autotools-dev \
	bison \
	curl \
	flex \
	git \
	graphviz \
	libmonetdb-client-dev \
	libpq-dev \
	libreadline6 \
	libreadline6-dev \
	libsqlite3-dev \
	libtool \
	make \
	man-db \
	monetdb5-sql \
	monetdb-client \
	pandoc \
	postgresql \
	rman \
	sqlite3 \
	unzip \
	vim \
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
########################################
# Install docker client
########################################
RUN mkdir -p /tmp/download && \
 	curl -L https://get.docker.com/builds/Linux/x86_64/docker-1.13.1.tgz | tar -xz -C /tmp/download && \
	rm -rf /tmp/download/docker/dockerd && \
	mv /tmp/download/docker/docker* /usr/local/bin/ && \
	rm -rf /tmp/download
