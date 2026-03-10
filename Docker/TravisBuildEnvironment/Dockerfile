########################################
# Derived from ubuntu distribution
########################################
FROM oraclelinux:9

########################################
# Image Metadata
########################################
LABEL org.opencontainers.image.authors="Boris Glavic <bglavic@uic.edu>"

########################################
# Arguments
########################################
ARG TARGETARCH
ARG DUCKDB_VERSION=1.5.0

########################################
# Create working directories
########################################
WORKDIR /gprom

########################################
# Instantclient
########################################
RUN  dnf -y install oracle-instantclient-release-23ai-el9 && \
     dnf -y install oracle-instantclient-basic oracle-instantclient-devel oracle-instantclient-sqlplus

# Uncomment if the tools package is added
ENV PATH=$PATH:/usr/lib/oracle/23/client64/bin

RUN dnf -y groupinstall "Development Tools"

########################################
# Install packages needed for building
########################################
RUN dnf -y install \
	ant \
    bison \
    bzip2 \
	curl \
    flex \
	git \
    graphviz \
    libcurl-devel \
    openssl-libs \
    xz-devel \
	libpq-devel \
    readline \
	readline-devel \
	sqlite-devel \
    sqlite-libs \
    libtool \
    libzip \
    libzip-tools \
	make \
    man-db \
    openssl \
    pkg-config \
	postgresql \
	sshpass \
    sqlite \
    sudo \
	unzip \
    vim \
    zlib \
    zlib-devel \
    wget \
    && rm -rf /var/cache/dnf

# dh-make libbz2-dev libcurl4-openssl-dev liblzma-dev libpq-dev rman zlib1g-dev
#     libcurl4-openssl-dev \
# 	pandoc \     libbz2-dev \ 	rman \
########################################
# Install duckdb
########################################
RUN wget https://github.com/duckdb/duckdb/releases/download/v${DUCKDB_VERSION}/libduckdb-linux-${TARGETARCH}.zip \
    && unzip libduckdb-linux-${TARGETARCH}.zip -d libduckdb \
    && mkdir -p /usr/local/include \
    && mkdir -p /usr/local/lib \
    && mv libduckdb/duckdb.* /usr/local/include/ \
    && mv libduckdb/libduckdb.so /usr/local/lib/ \
    && rm -rf *.zip
RUN wget https://install.duckdb.org/v1.4.4/duckdb_cli-linux-${TARGETARCH}.zip \
    && unzip duckdb_cli-linux-${TARGETARCH}.zip \
    && mv duckdb /usr/local/bin \
    && rm -rf *.zip

########################################
# Install MonetDB
########################################
ARG MONETDB_RELEASE=Dec2025-SP1
ARG MONETDB_VERSION=11.55.3
RUN dnf install openssl-devel libxml2-devel pcre-devel wget
# RUN dnf install https://dev.monetdb.org/downloads/epel/MonetDB-release-epel.noarch.rpm \
#     && update-crypto-policies --set DEFAULT:SHA1 \
#     && rpm --import https://dev.monetdb.org/downloads/MonetDB-GPG-KEY
RUN groupadd monetdb && \
    usermod -a -G monetdb root
RUN  mkdir -p /monetdb \
    && cd /monetdb \
    && wget https://www.monetdb.org/downloads/sources/${MONETDB_RELEASE}/MonetDB-${MONETDB_VERSION}.tar.bz2 --no-check-certificate \
    && bzip2 -d MonetDB-${MONETDB_VERSION}.tar.bz2 \
    && tar xf MonetDB-${MONETDB_VERSION}.tar \
    && cd MonetDB-${MONETDB_VERSION} \
    && ./configure --prefix=/monetdb/install --enable-assert --enable-debug --with-libxml2=no \
    && make -j && make install \
    && rm -fr /monetdb/MonetDB-${MONETDB_VERSION}.tar /monetdb/MonetDB-${MONETDB_VERSION}
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
