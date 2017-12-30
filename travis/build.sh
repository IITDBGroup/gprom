#!/bin/bash

# move to parent directory for build
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
pushd ${DIR}/..

docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make -f Makefile.maintainer
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest ./configure --with-oci-headers=/opt/oracle/instantclient/sdk/include/ --with-oci-lib=/opt/oracle/instantclient
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make check
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest make dist

popd > /dev/null
