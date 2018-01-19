########################################
# Derived from gprom travis build image which supports all backends
########################################
FROM iitdbgroup/gprom_travis
MAINTAINER Boris Glavic <bglavic@iit.edu>
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
# Per default show help message
########################################
CMD ["-help"]
