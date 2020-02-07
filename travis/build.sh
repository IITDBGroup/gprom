#!/bin/bash

# function to test exist status
testexit()
{
	EXIT_STATUS=$?
	LOG_FILE="$1"
	if [ "x${EXIT_STATUS}" != "x0" ]; then
		echo "COMMAND EXITED WITH STATUS ${EXIT_STATUS}"
		echo "LOG OUTPUT:"
		cat ${LOG_FILE}
		exit ${EXIT_STATUS}
	fi
}

# move to parent directory for build
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
pushd ${DIR}/..

echo "************************* RUN AUTOTOOLS SETUP"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make -f Makefile.maintainer > log_autotools.log 2>&1
testexit log_autotools.log

echo "************************* RUN CONFIGURE"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest env LDFLAGS="-L/monetdb/install/lib" LIBS='-lstream -lcrypto -llzma -lcurl -lbz2' ./configure --with-oci-headers=/usr/local/oracle/include/oracle/12.2/client64/ --with-oci-lib=/usr/local/oracle/lib/oracle/12.2/client64/lib/ --enable-monetdb --with-mapi-headers=/usr/include/monetdb/ --with-libpq-headers=/usr/include/postgresql --with-mapi-headers=/monetdb/install/include/monetdb/ --with-mapi-lib=/monetdb/install/lib/ > log_configure.log 2>&1
testexit log_configure.log

echo "************************* RUN MAKE"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make -j32 > log_make.log 2>&1
testexit log_make.log

echo "************************* RUN MAKE CHECK"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make -j32 check > log_check.log 2>&1
testexit log_check.log

echo "************************* RUN MAKE DIST"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make -j32 dist > log_dist.log 2>&1
testexit log_dist.log

echo "************************* RUN MAKE DISTCHECK"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make -j32 distcheck > log_distcheck.log 2>&1
testexit log_distcheck.log

echo "************************* RUN MAKE PKG-DEB"
docker run --rm --name dockbuild -e USER=root -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make pkg-deb > log_deb.log 2>&1
testexit log_deb.log

echo "************************* RUN LIBGPROM CROSS-COMPILATION"
docker run --rm --name dockbuild -v /var/run/docker.sock:/var/run/docker.sock -e GPROM_HOST_DIR="$(pwd)" -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest ant -Dskipivy=true jar-fat > log_jar-fat.log 2>&1
testexit log_jar-fat.log

echo "************************* RUN REBUILD JAR WITH ALL NATIVE LIBRARIES"
docker run --rm --name dockbuild -v /var/run/docker.sock:/var/run/docker.sock -e GPROM_HOST_DIR="$(pwd)" -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest ant -Dskipivy=true jar > log_jar.log 2>&1
testexit log_jar.log

popd > /dev/null

exit 0
