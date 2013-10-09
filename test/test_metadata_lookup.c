/*
 * test_metadata_lookup.c
 *
 *      Author: zephyr
 */

#include <string.h>

#include "test_main.h"
#include "mem_manager/mem_mgr.h"
#include "common.h"
#include "configuration/option.h"
#include "metadata_lookup/metadata_lookup.h"
#include "mem_manager/mem_mgr.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "log/logger.h"

/* internal tests */
static rc testCatalogTableExists();
static rc testGetAttributes();

rc
testMetadataLookup(void)
{
	RUN_TEST(testCatalogTableExists(), "test catalog table exists");
	RUN_TEST(testGetAttributes(), "test get attributes");

	return PASS;
}

static rc
testCatalogTableExists()
{
	boolean hasR = catalogTableExists("r");
	boolean hasS = catalogTableExists("s");

	ASSERT_EQUALS_INT(hasR, TRUE, "test Has table r");
	ASSERT_EQUALS_INT(hasS, FALSE, "test Doesn't have table s");

	return PASS;
}

static rc
testGetAttributes()
{
	List* attrs = getAttributes("r");
	ListCell* temp = attrs->head;
	while(temp->next != NULL)
	{
		printf("Attribute %s\n",attrs->head->data);
		temp = temp->next;
	}
	return PASS;
}
