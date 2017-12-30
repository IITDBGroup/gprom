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

docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make -f Makefile.maintainer
testexit
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest ./configure --with-oci-headers=/usr/local/oracle/include/oracle/12.2/client64/ --with-oci-lib=/usr/local/oracle/lib/oracle/12.2/client64/lib/
testexit
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make
testexit
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make check
testexit
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make dist
testexit

popd > /dev/null

exit 0
