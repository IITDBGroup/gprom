########################################
# Derived from ubuntu distribution
########################################
# FROM sath89/oracle-12c:latest
FROM collinestes/docker-node-oracle
########################################
# Create working directories
########################################
WORKDIR /gprom
WORKDIR examples
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
	openjdk-7-jdk \
	libpq-dev \
	libreadline6 \
	libreadline6-dev \
	libsqlite3-dev \
	libtool \
	make \
	sqlite3 \
	&& rm -rf /var/lib/apt/lists/*
ENV JAVA_HOME /usr/lib/jvm/java-7-openjdk-amd64
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
	&& make install
########################################
# Clone GProM github repository
########################################
RUN git clone https://github.com/IITDBGroup/gprom.git /gprom/src
########################################
# Build GProM
########################################
RUN cd /gprom/src \
	&& make -f Makefile.maintainer \
	&& ./configure --with-oci-headers=/opt/oracle/instantclient/sdk/include/ --with-oci-lib=/opt/oracle/instantclient \
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
