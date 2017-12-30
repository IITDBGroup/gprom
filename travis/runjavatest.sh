#!/bin/bash

# move to parent directory for build
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
pushd ${DIR}/..

# copy oracle jdbc driver from container
docker run --rm -d --name dockbuild iitdbgroup/gprom_travis:latest sleep 50000
docker cp dockerbuild:/usr/local/oracle/lib/oracle/12.2/client64/lib/ojdbc8.jar ./build/javalib
docker stop dockerbuild

# run tests
docker run --rm --name dockbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest ant -f blackboxtests run-test-only
RESULT=$?

popd > /dev/null
exit $RESULT

