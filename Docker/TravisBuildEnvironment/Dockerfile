########################################
# Derived from ubuntu distribution
########################################
FROM oraclelinux:8
MAINTAINER Boris Glavic <bglavic@iit.edu>

########################################
# Create working directories
########################################
WORKDIR /gprom

########################################
# Instantclient
########################################
RUN  dnf -y install oracle-instantclient-release-el8 && \
     dnf -y install oracle-instantclient-basic oracle-instantclient-devel oracle-instantclient-sqlplus

########################################
# Install packages needed for building
########################################
RUN dnf -y install \
	ant \
	autoconf \
	automake \
    autotools-dev \
    bison \
    build-essential:native \
    bzip2 \
	curl \
	dh-make \
    fakeroot \
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
    && rm -rf /var/cache/dnf
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
