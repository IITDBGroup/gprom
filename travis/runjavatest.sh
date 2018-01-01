#!/bin/bash

# move to parent directory for build
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
pushd ${DIR}/..

# copy oracle jdbc driver from container
docker run --rm -d --name dockerbuild iitdbgroup/gprom_travis:latest sleep 50000
docker cp dockerbuild:/usr/local/oracle/lib/oracle/12.2/client64/lib/ojdbc8.jar ./build/javalib
docker stop dockerbuild
docker rm dockerbuild

# run tests
#TODO manage creditional in travis because activating this again: docker run --rm --name dockerbuild -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest ant -Dskipivy=true -f blackboxtests/build.xml run-test-only
echo "SKIP TESTS UNTIL WE HAVE IMPLEMENTED CREDENTIAL PASSING FROM TRAVIS"
RESULT=$?

popd > /dev/null
exit $RESULT

