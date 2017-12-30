#!/bin/bash

# function to test exist status
testexit()
{
	EXIT_STATUS=$?
	if [ "x${EXIT_STATUS}" != "x0" ]; then
		echo "COMMAND EXITED WITH STATUS ${EXIT_STATUS}"
		exit ${EXIT_STATUS}
	fi
}

# move to parent directory for build
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
pushd ${DIR}/..

echo "************************* RUN AUTOTOOLS SETUP"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make -f Makefile.maintainer
testexit

echo "************************* RUN CONFIGURE"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest ./configure --with-oci-headers=/usr/local/oracle/include/oracle/12.2/client64/ --with-oci-lib=/usr/local/oracle/lib/oracle/12.2/client64/lib/
testexit

echo "************************* RUN MAKE"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make
testexit

echo "************************* RUN MAKE CHECK"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make check
testexit

echo "************************* RUN MAKE DIST"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make dist
testexit

echo "************************* RUN MAKE DISTCHECK"
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make distcheck
testexit

echo "************************* RUN LIBGPROM CROSS-COMPILATION"
docker run --rm --name dockbuild -v /var/run/docker.sock:/var/run/docker.sock -e GPROM_HOST_DIR="$(pwd)" -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest ant jar-fat
testexit

echo "************************* RUN REBUILD JAR WITH ALL NATIVE LIBRARIES"
docker run --rm --name dockbuild -v /var/run/docker.sock:/var/run/docker.sock -e GPROM_HOST_DIR="$(pwd)" -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest ant jar
testexit

popd > /dev/null

exit 0
