/*
 * metadata_lookup.h
 *
 *      Author: zephyr
 *      Get the metadata by table name.
 *
 *      catalogTableExists() checks if the given table exists
 *      getAttributes() gets all the attribute names of the given table
 *      Don't worry about the connection establishment. It will automatically connect to oracle server
 *      if not created.
 *      But you should call databaseConnectionClose() at the end of the whole program.
 *      Note that it need only be called once because it frees the list of attributes and all resources allocated
 *      for OCI connection.
 */

#ifndef METADATA_LOOKUP_H_
#define METADATA_LOOKUP_H_

#include "model/list/list.h"

#if HAVE_LIBOCILIB
#include <ocilib.h>
    extern OCI_Connection *getConnection();
#endif

extern boolean catalogTableExists (char * tableName);
extern List *getAttributes (char *tableName);
extern int databaseConnectionClose();

#endif /* METADATA_LOOKUP_H_ */
