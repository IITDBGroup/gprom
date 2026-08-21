# setup autotools build system
[group('build')]
autotools:
    make -f Makefile.maintainer

# build
[group('build')]
build:
    make -j all

# run module tests
[group('build')]
test: build
    make -j check

# connection parameters
database := ""
host := "127.0.0.1"
port := "5432"
user := "postgres"
password := "test"

# create duckdb database for blackbox tests
[group('blackboxtests')]
blackboxtest-prepare-duckdb:
    duckdb blackboxtests/test-duckdb.db < blackboxtests/testdb/testDB.sql

# create sqlite database for blackbox tests
[group('blackboxtests')]
blackboxtest-prepare-sqlite:
    sqlite blackboxtests/test-sqlite.db --init blackboxtests/testdb/testDB.sql

# create sqlite database for blackbox tests
[group('blackboxtests')]
blackboxtest-create-venv:
    python -m venv ./blackboxtests/venv
    source ./blackboxtests/venv/bin/activate
    pip install -r ./blackboxtests/requirements.txt

# create virtual environment and create duckdb and sqlite database for the blackboxtests
[group('blackboxtests')]
blackboxtest-prepare: blackboxtest-create-venv blackboxtest-prepare-sqlite blackboxtest-prepare-duckdb
    @echo "Ready to run tests"

# run duckdb blackbox tests
[group('blackboxtests')]
blackboxtest-run-duckdb settings="default.optnopullnoselmove,default.noopt" tests="reg,prov,prov_use_tid,prov_composable,prov_composable_use_tid,semiring,reenact,query_prov,dl.reg,dl.lineage" *extra-args:
    source ./blackboxtests/venv/bin/activate
    ./blackboxtests/gprom_blackbox_tests.py -b duckdb -d "{{ if database == '' { './blackboxtests/test-duckdb.db' } else { database } }}" -s "{{settings}}" -t "{{tests}}" {{extra-args}}

# run sqlite blackbox tests
[group('blackboxtests')]
blackboxtest-run-sqlite settings="default.optnopullnoselmove,default.noopt" tests="reg,prov,prov_use_tid,prov_composable,prov_composable_use_tid,semiring,reenact,query_prov,dl.reg,dl.lineage" *extra-args:
    source ./blackboxtests/venv/bin/activate
    ./blackboxtests/gprom_blackbox_tests.py -b sqlite -d {{database}} -s "{{settings}}" -t "{{tests}}" {{extra-args}}

# run postgres blackbox tests
[group('blackboxtests')]
blackboxtest-run-postgres settings="default.optnopullnoselmove,default.noopt" tests="reg,prov,prov_use_tid,prov_composable,prov_composable_use_tid,semiring,reenact,query_prov,dl.reg,dl.lineage" *extra-args:
    source ./blackboxtests/venv/bin/activate
    ./blackboxtests/gprom_blackbox_tests.py -b postgres -h "{{host}}" -d  -s "{{settings}}" -t "{{tests}}" {{extra-args}}
