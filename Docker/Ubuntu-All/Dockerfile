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
	&& env LDFLAGS="-L/monetdb/install/lib" LIBS='-lstream -lcrypto -llzma -lcurl -lbz2' ./configure --enable-monetdb --enable-postgres --enable-oracle --enable-sqlite --with-oci-headers=/usr/local/oracle/include/oracle/12.2/client64/ --with-oci-lib=/usr/local/oracle/lib/oracle/12.2/client64/lib/ --enable-monetdb --with-mapi-headers=/usr/include/monetdb/ --with-libpq-headers=/usr/include/postgresql --with-mapi-headers=/monetdb/install/include/monetdb/ --with-mapi-lib=/monetdb/install/lib/ \
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
