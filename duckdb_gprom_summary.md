# Incorporating DuckDB into GProM

### Running with DuckDB

On terminal:

```bash
./autogen.sh

./configure --enable-duckdb --with-duckdb-headers=/Users/gmorei/code/gprom/include/duckdb --with-duckdb-lib=/usr/local/lib

make

sudo make install

gprom -backend duckdb -Pmetadata duckdb -db test.duckdb
```

### Installing DuckDB C API

- Download `https://duckdb.org/docs/installation/index?version=main&environment=cplusplus&platform=macos&download_method=direct`
- Point to duckdb header and lib when running ./configure

### Done

- Compile gprom on MacOS
- Learn basic DuckDB C API
- Learn basic M4 Macro Language
- Modify configure.ac to support DuckDB (search for library and headers)
- Study GProM's architecture and intrumentation pipelines
- Create metadata_lookup_duckdb.c
- Create metadata_lookup_duckdb.h
- Add sql_serializer_duckdb.c to Makefile.am
- Compile code with new metadata_lookup_duckdb.c
- Install dubugging tool and learn how to use it (lldb)
- Test different queries (select, where, group by, ...) to fix bugs in `metadata_lookup_duckdb.c
- Test spatial queries

### ToDo

- [ ] Finish implementing sql_serializer for DuckDB
    - [ ] Modify sql_serializer_common
    - [ ] Implement sql_serializer_duckdb.c
    - [ ] Implement sql_serializer_duckdb.h
    - [ ] Modify assembleDuckDBPlugin in sql_serializer.c
- [ ] Implement new datatypes to convert binary geometries to an interpretable format
- [ ] Test different levels of granularity for spatial queries
    - [ ] Function level
    - [ ] Geometry as string
    - [ ] Geometry + Spatial Join Semantics
- [ ] Add docker support for DuckDB
- [ ] Create tests for DuckDB
- [ ] Create Examples for DuckDB
- [ ] Fix bugs

### Bugs to fix

- --with-duckdb-lib is not being correctly recognized
- All atributes are being capitalized
- CREATE TABLE does not work
- DELETE FROM does not delete permanentely

### Errors fixed

```bash
**../../include/model/list/list.h:33:9: error: 'LIST_EMPTY' macro redefined [-Werror,-Wmacro-redefined]**

33 | #define LIST_EMPTY(l) (LIST_LENGTH(l) == 0)

|         **^**

**/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/sys/queue.h:495:9: note:** previous definition is here

495 | #define LIST_EMPTY(head)        ((head)->lh_first == NULL)

|         **^**

1 error generated.
```

Solution: rename LIST_EMPTY → MY_LIST_EMPTY and modify it everywhere it appears.

---

At `configure.ac`:  

```bash
error: _AC_LANG_ABBREV: unknown language: _AC_LANG:
```

Solution: add “AC_LANG([C])” to configure.ac

