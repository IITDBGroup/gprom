########################################
# Derived from ubuntu distribution with LXDE and VNC
########################################
FROM accetto/ubuntu-vnc-xfce-g3:24.04
#    dorowu/ubuntu-desktop-lxde-vnc
LABEL org.opencontainers.image.authors="Boris Glavic <bglavic@uic.edu>"

########################################
# Set some databases as versions
########################################
ARG DUCKDB_VERSION=1.4.4
# ARG MONETDB_VERSION=
ARG ORACLE_INSTANTCLIENT_VERSION=12.1.0.2.0

########################################
# Run as root
########################################
USER 0

########################################
# Create working directories
########################################
WORKDIR /home/ubuntu
########################################
# Install monetdb
########################################
RUN apt-get -y update && apt-get -y  --no-install-recommends install apt-transport-https wget gpg-agent
RUN echo "deb http://dev.monetdb.org/downloads/deb/ noble monetdb" > /etc/apt/sources.list.d/monetdb.list \
     && echo "deb-src http://dev.monetdb.org/downloads/deb/ noble monetdb" >> /etc/apt/sources.list.d/monetdb.list \
     && wget -q --output-document=- https://www.monetdb.org/downloads/MonetDB-GPG-KEY | apt-key add -
########################################
# Install packages needed for building
########################################
RUN apt-get -y update && apt-get install -y --no-install-recommends  \
	ant \
	autotools-dev \
	autoconf \
	automake \
	bison \
    build-essential \
    bzip2 \
	curl \
	emacs \
	flex \
	gdb \
	git \
	graphviz \
    openjdk-8-jdk \
    openjdk-8-jdk-headless \
    openssl \
    libaio1t64    \
    libbz2-dev \
    libcurl4-openssl-dev \
    liblzma-dev \
	libpq-dev \
	libreadline-dev \
    libreadline8t64 \
	libsqlite3-dev \
    libtool \
    libzip-dev \
	locate \
	make \
	man-db \
	pandoc \
	postgresql \
	rman \
	sqlite3 \
	unzip \
	monetdb5-sql \
	monetdb-client \
    libmonetdb-client-dev \
    valgrind \
    zlib1g-dev \
    && rm -rf /var/lib/apt/list
########################################
# Install duckdb
########################################
RUN wget https://github.com/duckdb/duckdb/releases/download/v${DUCKDB_VERSION}/libduckdb-linux-amd64.zip \
    && unzip libduckdb-linux-amd64.zip -d libduckdb \
    && mkdir -p /usr/local/include \
    && mkdir -p /usr/local/lib \
    && mv libduckdb/duckdb.* /usr/local/include/ \
    && mv libduckdb/libduckdb.so /usr/local/lib/ \
    && rm -rf *.zip
RUN wget https://install.duckdb.org/v1.4.4/duckdb_cli-linux-amd64.zip \
    && unzip duckdb_cli-linux-amd64.zip \
    && mv duckdb /usr/local/bin \
    && rm -rf *.zip
########################################
# Setup JAVA_HOME
########################################
ENV JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
########################################
# Setup MonetDB
########################################
RUN usermod -a -G monetdb root
########################################
# Add oracle OCI library files
########################################
RUN mkdir -p /opt/oracle
COPY ./instantclient* /home/ubuntu/instantclient/

RUN unzip /home/ubuntu/instantclient/instantclient-basic-linux.x64-${ORACLE_INSTANTCLIENT_VERSION}.zip -d /opt/oracle \
  && unzip /home/ubuntu/instantclient/instantclient-sdk-linux.x64-${ORACLE_INSTANTCLIENT_VERSION}.zip -d /opt/oracle  \
  && unzip /home/ubuntu/instantclient/instantclient-sqlplus-linux.x64-${ORACLE_INSTANTCLIENT_VERSION}.zip -d /opt/oracle  \
  && mv /opt/oracle/instantclient_12_1 /opt/oracle/instantclient \
  && ln -s /opt/oracle/instantclient/libclntsh.so.12.1 /opt/oracle/instantclient/libclntsh.so \
  && ln -s /opt/oracle/instantclient/libocci.so.12.1 /opt/oracle/instantclient/libocci.so

ENV LD_LIBRARY_PATH="/opt/oracle/instantclient"
ENV OCI_HOME="/opt/oracle/instantclient"
ENV OCI_LIB_DIR="/opt/oracle/instantclient"
ENV OCI_INCLUDE_DIR="/opt/oracle/instantclient/sdk/include"

RUN echo '/opt/oracle/instantclient/' | tee -a /etc/ld.so.conf.d/oracle_instant_client.conf && ldconfig
########################################
# Clone OCILIB
########################################
RUN git clone https://github.com/vrogier/ocilib.git /home/ubuntu/ocilib/src
########################################
# Build OCILIB
########################################
RUN cd /home/ubuntu/ocilib/src \
	&& sh reconf \
	&& ./configure --with-oracle-headers-path=/opt/oracle/instantclient/sdk/include --with-oracle-lib-path=/opt/oracle/instantclient \
	&& make \
	&& make install \
	&& echo "/usr/local/bin/" >> /etc/ld.so.conf \
    && /sbin/ldconfig
########################################
# Create missing symlink needed for monetdb and OCI libraries
########################################
RUN ln -s /usr/lib/x86_64-linux-gnu/libmapi-11.55.3.so /usr/lib/x86_64-linux-gnu/libmapi.so
RUN ln -s /usr/lib/x86_64-linux-gnu/libstream.so.13 /usr/lib/x86_64-linux-gnu/libstream.so
RUN ln -s /usr/lib/x86_64-linux-gnu/libaio.so.1t64.so.0.2 /usr/lib/x86_64-linux-gnu/libaio.so.1
########################################
# Clone GProM github repository
########################################
RUN mkdir -p /home/headless/gprom
RUN chown -R headless /home/headless
USER headless
RUN git clone https://github.com/IITDBGroup/gprom.git /home/headless/gprom
########################################
# Build GProM
########################################
RUN cd /home/headless/gprom \
	&& make -f Makefile.maintainer \
    && ./configure \
       --prefix=/home/headless/gprominstall \
       --enable-sqlite \
       --enable-postgres --with-libpq-headers=/usr/include/postgresql \
       --enable-duckdb \
       --with-mapi-lib=/usr/lib/x86_64-linux-gnu --with-mapi-headers=/usr/include/monetdb \
       --enable-sqlite --with-oci-headers=/opt/oracle/instantclient/sdk/include/ --with-oci-lib=/opt/oracle/instantclient/ \
    && make -j \
    && make install
########################################
# Build GProM
########################################
RUN echo 'export PATH="$PATH:/home/headless/gprominstall/bin"' >> ~/.bashrc
########################################
# Expose VNC port
########################################
#EXPOSE 5900
########################################
# copy ojdbc driver into gprom lib dir
########################################
RUN mkdir -p /home/headless/gprom/build/javalib/ /home/headless/gprom/build/javatestlib/ \
    && cp /opt/oracle/instantclient/ojdbc7.jar /home/headless/gprom/build/javalib/ojdbc7.jar \
    && cp /opt/oracle/instantclient/ojdbc7.jar /home/headless/gprom/build/javatestlib/ojdbc7.jar
########################################
# Per default start the gui (entrypoint from accetto/ubuntu-vnc-xfce-g3) which requires root
########################################
USER 0
