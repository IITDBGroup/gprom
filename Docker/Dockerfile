########################################
# Derived from gprom CI build image which is oracle linux which supports all backends
########################################
FROM iitdbgroup/gprom-build-env:latest
LABEL org.opencontainers.image.authors="Boris Glavic <bglavic@uic.edu>"
########################################
# Clone GProM github repository
########################################
RUN git clone https://github.com/IITDBGroup/gprom.git /gprom/src
########################################
# Build GProM
########################################
RUN cd /gprom/src \
	&& make -f Makefile.maintainer \
	&& env LDFLAGS="-L/monetdb/install/lib" LIBS='-lstream -lcrypto -llzma -lcurl -lbz2' ./configure \
       --prefix=/gprom/install \
       --enable-monetdb --with-mapi-lib=/monetdb/install/lib/ --with-mapi-headers=/monetdb/install/include/monetdb/ \
       --enable-postgres --with-libpq-headers=/usr/include/postgresql \
       --enable-oracle --with-oci-headers=/usr/include/oracle/23/client64/ --with-oci-lib=/usr/lib/oracle/23/client64/lib/ \
       --enable-sqlite \
       --enable-duckdb \
	&& make -j \
	&& make -j install
########################################
# Per default start the gprom command line interface
########################################
ENTRYPOINT ["/gprom/install/bin/gprom"]
########################################
# Per default show help message
########################################
CMD ["-help"]
