########################################
# Derived from ubuntu distribution
########################################
FROM ubuntu:latest
MAINTAINER Boris Glavic <bglavic@iit.edu>
########################################
# Create working directories
########################################
WORKDIR /gprom
########################################
# Install packages needed for building
########################################
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get -y --no-install-recommends  update && apt-get install -y --no-install-recommends --force-yes -q \
	ant \
	autoconf \
	automake \
    autotools-dev \
    bison \
    build-essential:native \
    bzip2 \
    curl \
    cmake \
	dh-make \
    fakeroot \
    flex \
	git \
    graphviz \
    libbz2-dev \
    libcurl4-openssl-dev \
    liblzma-dev \
    libpq-dev \
    libreadline-dev \
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
# Install Java 8
########################################
# RUN apt-get update && \
#     apt-get install -y --no-install-recommends openjdk-8-jdk-headless openjdk-8-jdk
# ENV JAVA_VER 8
# ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64/

########################################
# Install MonetDB
########################################
# https://www.monetdb.org/downloads/sources/Jul2021/MonetDB-11.41.5.tar.bz2
ARG MONETDB_VERSION=11.41.5
RUN mkdir -p /monetdb \
    && cd /monetdb \
    && wget --no-check-certificate https://www.monetdb.org/downloads/sources/Jul2021/MonetDB-${MONETDB_VERSION}.tar.bz2 \
    && bzip2 -d MonetDB-${MONETDB_VERSION}.tar.bz2 \
    && tar xf MonetDB-${MONETDB_VERSION}.tar \
    && cmake -DCMAKE_INSTALL_PREFIX=/monetdb/install -DWITH_XML2=off ./MonetDB-${MONETDB_VERSION} \
    && cmake --build . \
    && cmake --build . --target install \
    && rm -fr /monetdb/MonetDB-${MONETDB_VERSION}.tar /monetdb/MonetDB-${MONETDB_VERSION}

########################################
# Clone GProM github repository
########################################
RUN git clone https://github.com/IITDBGroup/gprom.git /gprom/src
########################################
# Build GProM
########################################
RUN cd /gprom/src \
	&& make -f Makefile.maintainer \
	&& ./configure --disable-java --enable-postgres --enable-sqlite --enable-monetdb --with-libpq-headers=/usr/include/postgresql/ --with-libpq=/usr/lib/aarch64-linux-gnu/ --with-mapi-headers=/monetdb/install/include/monetdb/ --with-mapi-lib=/monetdb/install/lib/ \
	&& make \
	&& make install
########################################
# Per default start the gprom command line interface
########################################
ENTRYPOINT ["/usr/local/bin/gprom"]
########################################
# Per default connect to the example SQLite database
########################################
CMD ["-help"]
