########################################
# Derived from ubuntu distribution
########################################
FROM iitdbgroup/oic:latest
MAINTAINER Boris Glavic <bglavic@iit.edu>
########################################
# Create working directories
########################################
WORKDIR /gprom
########################################
# Install monetdb repository
########################################
# RUN apt-get -y update && apt-get -y --no-install-recommends install apt-transport-https software-properties-common wget
# RUN echo "deb http://dev.monetdb.org/downloads/deb/ xenial monetdb" > /etc/apt/sources.list.d/monetdb.list \
#     && echo "deb-src http://dev.monetdb.org/downloads/deb/ xenial monetdb" >> /etc/apt/sources.list.d/monetdb.list \
#     && wget -q --output-document=- https://www.monetdb.org/downloads/MonetDB-GPG-KEY | apt-key add -
########################################
# Install Java 8
########################################
# RUN echo oracle-java8-installer shared/accepted-oracle-license-v1-1 select true | debconf-set-selections && \
#   add-apt-repository -y ppa:webupd8team/java && \
#   apt-get update && \
#   apt-get install -y --no-install-recommends oracle-java8-installer && \
#   rm -rf /var/lib/apt/lists/* && \
#   rm -rf /var/cache/oracle-jdk8-installer
# RUN apt install oracle-java8-set-default
# Try default-jdk instead
RUN apt-get update && \
    apt-get install -y --no-install-recommends openjdk-8-jdk-headless openjdk-8-jdk
ENV JAVA_VER 8
ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64/
########################################
# Install packages needed for building
########################################
RUN apt-get -y update && apt-get --no-install-recommends install -y \
	ant \
	autoconf \
	automake \
	autotools-dev \
    bison \
    bzip2 \
	curl \
	dh-make \
	flex \
	git \
    graphviz \
    libbz2-dev \
    libcurl4-openssl-dev \
    liblzma-dev \
	libpq-dev \
    libreadline6 \
	libreadline6-dev \
	libsqlite3-dev \
    libtool \
    libzip-dev \
	make \
    man-db \
    openssl \
    pkg-config \
	pandoc \
	postgresql \
	rman \
	sshpass \
    sqlite3 \
    sudo \
	unzip \
    vim \
    zlib1g-dev \
    wget \
    && rm -rf /var/lib/apt/lists/*
########################################
# Install MonetDB
########################################
RUN mkdir -p /monetdb \
    && cd /monetdb \
    && wget https://www.monetdb.org/downloads/sources/Apr2019-SP1/MonetDB-11.33.11.tar.bz2 --no-check-certificate \
    && bzip2 -d MonetDB-11.33.11.tar.bz2 \
    && tar xf MonetDB-11.33.11.tar \
    && cd MonetDB-11.33.11 \
    && ./configure --prefix=/monetdb/install --enable-assert --enable-debug --with-libxml2=no \
    && make -j 64 && make install \
    && rm -fr /monetdb/MonetDB-11.33.11.tar /monetdb/MonetDB-11.33.11
########################################
# Setup MonetDB
########################################
#RUN usermod -a -G monetdb root
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
