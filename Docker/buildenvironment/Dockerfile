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
ARG MONETDB_RELEASE=Dec2025-SP1
ARG MONETDB_VERSION=11.55.3

########################################
# Create working directories
########################################
WORKDIR /gprom

########################################
# Instantclient
########################################
RUN  dnf -y install oracle-instantclient-release-23ai-el9 \
     && dnf -y install oracle-instantclient-basic oracle-instantclient-devel oracle-instantclient-sqlplus

# Uncomment if the tools package is added
ENV PATH=$PATH:/usr/lib/oracle/23/client64/bin

RUN dnf -y groupinstall "Development Tools"

########################################
# Install packages needed for building
########################################
RUN dnf -y install \
	ant \
	curl \
	git \
	libpq-devel \
	make \
	postgresql \
    postgresql-server \
	readline-devel \
	sqlite-devel \
	sshpass \
	unzip \
    bison \
    bzip2 \
    cmake \
    flex \
    graphviz \
    libcurl-devel \
    libtool \
    libxml2-devel \
    libzip \
    libzip-tools \
    man-db \
    openssl \
    openssl-devel \
    openssl-libs \
    pcre-devel \
    pkg-config \
    readline \
    sqlite \
    sqlite-libs \
    sudo \
    vim \
    wget \
    xz-devel \
    zlib \
    zlib-devel \
    && dnf clean all \
    && rm -rf /var/cache/dnf
########################################
# Install python3.11
########################################
RUN dnf install python3.12 python3.12-pip -y \
    && ln -s /usr/bin/python3.12 /usr/bin/python \
    && ln -s /usr/bin/pip3.12 /usr/bin/pip \
    && dnf clean all \
    && rm -rf /var/cache/dnf
########################################
# Install duckdb
########################################
RUN wget -q https://github.com/duckdb/duckdb/releases/download/v${DUCKDB_VERSION}/libduckdb-linux-${TARGETARCH}.zip \
    && unzip libduckdb-linux-${TARGETARCH}.zip -d libduckdb \
    && mkdir -p /usr/local/include \
    && mkdir -p /usr/local/lib \
    && mv libduckdb/duckdb.* /usr/local/include/ \
    && mv libduckdb/libduckdb.so /usr/local/lib/ \
    && rm -rf *.zip
RUN wget -q https://install.duckdb.org/v1.4.4/duckdb_cli-linux-${TARGETARCH}.zip \
    && unzip duckdb_cli-linux-${TARGETARCH}.zip \
    && mv duckdb /usr/local/bin \
    && rm -rf *.zip

########################################
# Install MonetDB
########################################
# RUN dnf install https://dev.monetdb.org/downloads/epel/MonetDB-release-epel.noarch.rpm \
#     && update-crypto-policies --set DEFAULT:SHA1 \
#     && rpm --import https://dev.monetdb.org/downloads/MonetDB-GPG-KEY
RUN groupadd monetdb && \
    usermod -a -G monetdb root
RUN mkdir -p /monetdb \
    && cd /monetdb \
    && wget -q https://www.monetdb.org/downloads/sources/${MONETDB_RELEASE}/MonetDB-${MONETDB_VERSION}.tar.bz2 --no-check-certificate \
    && bzip2 -d MonetDB-${MONETDB_VERSION}.tar.bz2 \
    && tar xf MonetDB-${MONETDB_VERSION}.tar \
    && cd /monetdb/MonetDB-${MONETDB_VERSION} \
    && cmake -DCMAKE_INSTALL_PREFIX=/monetdb/install /monetdb/MonetDB-${MONETDB_VERSION} \
    && cmake --build . \
    && cmake --build . --target install \
    && ln -s /monetdb/install/lib64/libmapi-11.55.3.so /monetdb/install/lib64/libmapi.so \
    && echo "/monetdb/install/lib64" > /etc/ld.so.conf.d/monetdb.conf \
	&& /sbin/ldconfig
########################################
# Clone OCILIB
########################################
RUN git clone https://github.com/vrogier/ocilib.git /ocilib/src
########################################
# Build OCILIB
########################################
RUN cd /ocilib/src \
	&& sh reconf \
	&& ./configure --with-oracle-headers-path=/usr/include/oracle/23/client64/ --with-oracle-lib-path=/usr/lib/oracle/23/client64/lib/\
	&& make -j \
	&& make -j install \
	&& echo "/usr/local/lib/" > /etc/ld.so.conf.d/ocilib.conf \
	&& /sbin/ldconfig
########################################
# Build MSSQL client
########################################
RUN if [ "${TARGETARCH}" == "amd64" ]; then \
    curl -o /etc/pki/rpm-gpg/RPM-GPG-KEY-microsoft https://packages.microsoft.com/keys/microsoft.asc \
    && curl -o /etc/yum.repos.d/mssql-server.repo https://packages.microsoft.com/config/rhel/9/mssql-server-2025.repo \
    && curl -o /etc/yum.repos.d/mssql-prod.repo https://packages.microsoft.com/config/rhel/9/prod.repo \
    && ACCEPT_EULA=Y yum install -y mssql-server mssql-tools unixODBC-devel; \
    fi

########################################
# Install docker client
########################################
# RUN mkdir -p /tmp/download && \
#  	curl -L https://get.docker.com/builds/Linux/x86_64/docker-1.13.1.tgz | tar -xz -C /tmp/download && \
# 	rm -rf /tmp/download/docker/dockerd && \
# 	mv /tmp/download/docker/docker* /usr/local/bin/ && \
# 	rm -rf /tmp/download
