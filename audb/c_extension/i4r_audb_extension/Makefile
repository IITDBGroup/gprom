# PostgreSQL PGXS build for i4r_audb_extension (PG 16+)
MODULE_big = i4r_audb_extension
OBJS = \
	i4r_audb_extension.o \
	arithmetic.o \
	logicalOperators.o \
	helperFunctions.o \
	serialization.o \
	prune.o

EXTENSION = i4r_audb_extension
DATA = i4r_audb_extension--1.1.sql

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
