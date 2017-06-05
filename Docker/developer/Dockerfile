########################################
# Derived from ubuntu distribution with LXDE and VNC
########################################
FROM dorowu/ubuntu-desktop-lxde-vnc
MAINTAINER Boris Glavic <bglavic@iit.edu>
########################################
# Create working directories
########################################
WORKDIR /home/ubuntu
########################################
# Install monetdb
########################################
touch /etc/apt/sources.list.d/monetdb.list
########################################
# Install packages needed for building
########################################
RUN apt-get -y update && apt-get install -y \
	ant \
	autotools-dev \
	autoconf \
	automake \
	bison \
	build-essential \
	curl \
	eclipse \
	eclipse-cdt \
	eclipse-cdt-autotools \
	eclipse-cdt-jni \
	eclipse-cdt-perf \
	eclipse-jdt \
	eclipse-cdt-valgrind \
	eclipse-xsd \
	emacs \
	flex \
	gdb \
	git \
	graphviz \
	openjdk-7-jdk \
	libaio1 \
	libpq-dev \
	libreadline6 \
	libreadline6-dev \
	libsqlite3-dev \
	libtool \
	locate \
	make \
	man-db \
	pandoc \
	postgresql \
	rman \
	sqlite3 \
	unzip \
	&& rm -rf /var/lib/apt/lists/*
ENV JAVA_HOME /usr/lib/jvm/java-7-openjdk-amd64
########################################
# Add oracle OCI library files
########################################
RUN mkdir -p /opt/oracle
ADD ./instantclient* /home/ubuntu/instantclient/

RUN unzip /home/ubuntu/instantclient/instantclient-basic-linux.x64-12.1.0.2.0.zip -d /opt/oracle \
  && unzip /home/ubuntu/instantclient/instantclient-sdk-linux.x64-12.1.0.2.0.zip -d /opt/oracle  \
  && unzip /home/ubuntu/instantclient/instantclient-sqlplus-linux.x64-12.1.0.2.0.zip -d /opt/oracle  \
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
	&& ./configure --with-oracle-headers-path=/opt/oracle/instantclient/sdk/include --with-oracle-lib-path=/opt/oracle/instantclient --disable-monetdb \
	&& make \
	&& make install \
	&& echo "/usr/local/bin/" >> /etc/ld.so.conf \
	&& /sbin/ldconfig
########################################
# Clone GProM github repository
########################################
RUN git clone https://github.com/IITDBGroup/gprom.git /home/ubuntu/gprom
########################################
# Build GProM
########################################
RUN cd /home/ubuntu/gprom \
	&& make -f Makefile.maintainer \
	&& ./configure --with-oci-headers=/opt/oracle/instantclient/sdk/include/ --with-oci-lib=/opt/oracle/instantclient \
	&& make \
	&& make install \
	&& make dist \
	&& make distcheck
########################################
# Expose VNC port
########################################
EXPOSE 5900
########################################
# Per default start the gui (entrypoint from dorowu/ubuntu-desktop-lxde-vnc)
########################################
########################################
# copy ojdbc driver into gprom lib dir
########################################
RUN cp /opt/oracle/instantclient/ojdbc7.jar /home/ubuntu/gprom/javalib/ojdbc7.jar
