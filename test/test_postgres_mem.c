#include "test_main.h"
#include "model/node/nodetype.h"
#include "mem_manager/mem_mgr.h"
#include "model/bitset/bitset.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"
#include "metadata_lookup/metadata_lookup_postgres.h"

static rc t1(void);
static rc t2(void);
rc
testPostgres(void)
{
	t1();
	t2();
	return PASS;
}

static rc
t1(void)
{
   StringInfo ctx = makeStringInfo();
	appendStringInfo(ctx, "name1");

	MemContext *context = NEW_MEM_CONTEXT(ctx->data);
	ACQUIRE_MEM_CONTEXT(context);
	postgresExecuteQuery("select 1;");

	CLEAR_CUR_MEM_CONTEXT();
	ASSERT_EQUALS_INT(0, memContextSize(context), "MEM: 0");
	FREE_CUR_MEM_CONTEXT();
	RELEASE_MEM_CONTEXT();
	return PASS;
}

static rc
t2(void)
{
    StringInfo ctx = makeStringInfo();
	appendStringInfo(ctx, "name2");

	MemContext *context = NEW_MEM_CONTEXT(ctx->data);
	ACQUIRE_MEM_CONTEXT(context);
	postgresExecuteQuery("select 1;");
	postgresExecuteQuery("select 2;");
	CLEAR_CUR_MEM_CONTEXT();
	ASSERT_EQUALS_INT(0, memContextSize(context), "MEM: 0");
	FREE_CUR_MEM_CONTEXT();
	RELEASE_MEM_CONTEXT();
	return PASS;
}





