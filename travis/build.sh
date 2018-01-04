#!/bin/bash

# function to test exist status
testexit()
{
	LOG_FILE="$1"
	EXIT_STATUS=$?
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
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make -f Makefile.maintainer > log.autotools 2>&1
testexit log.autotools

echo "************************* RUN CONFIGURE"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest ./configure --with-oci-headers=/usr/local/oracle/include/oracle/12.2/client64/ --with-oci-lib=/usr/local/oracle/lib/oracle/12.2/client64/lib/ > log.configure 2>&1
testexit log.configure

echo "************************* RUN MAKE"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make > log.make 2>&1
testexit log.make

echo "************************* RUN MAKE CHECK"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make check > log.check 2>&1
testexit log.check

echo "************************* RUN MAKE DIST"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make dist > log.dist 2>&1
testexit log.dist

echo "************************* RUN MAKE DISTCHECK"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make distcheck > log.distcheck 2>&1
testexit log.distcheck

echo "************************* RUN MAKE PKG-DEB"
docker run --rm --name dockbuild -e USER=root -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make pkg-deb > log.deb 2>&1
testexit log.deb

echo "************************* RUN LIBGPROM CROSS-COMPILATION"
docker run --rm --name dockbuild -v /var/run/docker.sock:/var/run/docker.sock -e GPROM_HOST_DIR="$(pwd)" -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest ant -Dskipivy=true jar-fat > log.jar-fat 2>&1
testexit log.jar-fat

echo "************************* RUN REBUILD JAR WITH ALL NATIVE LIBRARIES"
docker run --rm --name dockbuild -v /var/run/docker.sock:/var/run/docker.sock -e GPROM_HOST_DIR="$(pwd)" -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest ant -Dskipivy=true jar > log.jar 2>&1
testexit log.jar

popd > /dev/null

exit 0
