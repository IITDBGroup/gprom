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
	&& if [ "${TARGETARCH}" == "amd64" ]; then \
       ./configure \
       --prefix=/gprom/install \
       --enable-monetdb --with-mapi-lib=/monetdb/install/lib64/ --with-mapi-headers=/monetdb/install/include/monetdb/ \
       --enable-postgres --with-libpq-headers=/usr/include/postgresql \
       --enable-oracle --with-oci-headers=/usr/include/oracle/23/client64/ --with-oci-lib=/usr/lib/oracle/23/client64/lib/ \
       --enable-sqlite \
       --enable-duckdb \
       --enable-mssql; \
       else \
       ./configure \
       --prefix=/gprom/install \
       --enable-monetdb --with-mapi-lib=/monetdb/install/lib64/ --with-mapi-headers=/monetdb/install/include/monetdb/ \
       --enable-postgres --with-libpq-headers=/usr/include/postgresql \
       --enable-oracle --with-oci-headers=/usr/include/oracle/23/client64/ --with-oci-lib=/usr/lib/oracle/23/client64/lib/ \
       --enable-sqlite \
       --enable-duckdb; \
       fi \
	&& make -j \
	&& make -j install \
    && echo "export PATH=$PATH:/gprom/install/bin" >> ~/.bashrc
########################################
# Install python requirements
########################################
RUN /usr/bin/pip install -r /gprom/src/blackboxtests/requirements.txt
########################################
# Create example databases and default connection
########################################
RUN mkdir /dbs \
    && duckdb -f /gprom/src/blackboxtests/testdb/testDB-sqlite.sql /dbs/duckdb.db \
    && sqlite3 /dbs/sqlite.db < /gprom/src/blackboxtests/testdb/testDB-sqlite.sql \
    && echo "backend=duckdb" >> /root/.gprom \
    && echo "connection.db=/dbs/duckdb.db" >> /root/.gprom
########################################
# Per default start the gprom command line interface connecting to
# /dbs/duckdb.db using the duckdb backend
########################################
ENTRYPOINT ["/gprom/install/bin/gprom"]
