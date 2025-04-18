#include "test_main.h"
#include "model/node/nodetype.h"
#include "mem_manager/mem_mgr.h"
#include "model/bitset/bitset.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"


static rc memOfSketchSize(int size);
static rc memOfRangesSize(int size);

rc
testSketchRanges(void)
{
    INFO_LOG("sizeof int: %d\n", sizeof(int));
    int sizes[] = {100, 200, 500, 1000, 2000, 5000, 10000, 20000, 100000};
    for (int i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        memOfSketchSize(sizes[i]);
        memOfRangesSize(sizes[i]);
    }
    return PASS;
}

// Y = X + sizeof(bitsizeof(m))
// Y' = X + 2 sizeof (bitsizeof(m))
// y' - y = more accurate

static rc
memOfSketchSize(int size)
{
	// set name for mem
    StringInfo sketchSize = makeStringInfo();
    appendStringInfo(sketchSize, "sketch_size_%d", size);

	// create the mem
    MemContext *context = NEW_MEM_CONTEXT(sketchSize->data);
    ACQUIRE_MEM_CONTEXT(context);

    BitSet *bitset = newBitSet(size);
    for (int i = 0; i < size; i++) {
        setBit(bitset, i, TRUE);
    }

	// clear mem
    CLEAR_CUR_MEM_CONTEXT();
    ASSERT_EQUALS_INT(0, memContextSize(context), "MEM SHOULD BE 0");

    FREE_CUR_MEM_CONTEXT();
    RELEASE_MEM_CONTEXT();


    StringInfo sketchSize2 = makeStringInfo();
    appendStringInfo(sketchSize2, "sketch_size_double_%d", size);

	// create the mem
    MemContext *context2 = NEW_MEM_CONTEXT(sketchSize2->data);
    ACQUIRE_MEM_CONTEXT(context2);

    BitSet *bitset2 = newBitSet(size);
    for (int i = 0; i < size; i++) {
        setBit(bitset2, i, TRUE);
    }

    BitSet *bitset3 = newBitSet(size);
    for (int i = 0; i < size; i++) {
        setBit(bitset3, i, TRUE);
    }
	// clear mem
    CLEAR_CUR_MEM_CONTEXT();
    ASSERT_EQUALS_INT(0, memContextSize(context2), "MEM SHOULD BE 0");

    FREE_CUR_MEM_CONTEXT();
    RELEASE_MEM_CONTEXT();


    return PASS;

}

static rc
memOfRangesSize(int size)
{
    StringInfo rangesSize = makeStringInfo();
    appendStringInfo(rangesSize, "ranges_size_%d", size);

    MemContext *context = NEW_MEM_CONTEXT(rangesSize->data);
    ACQUIRE_MEM_CONTEXT(context);
    int val = 1;
    List *ranges = NIL;
    for (int i = 0; i < size + 1; i++) {
        ranges = appendToTailOfList(ranges, createConstInt(val + 10));
        val += 10;
    }

    psAttrInfo *attrInfo = createPSAttrInfo(ranges, "tbl");
    attrInfo->attrName = "col";

    CLEAR_CUR_MEM_CONTEXT();
    ASSERT_EQUALS_INT(0, memContextSize(context), "MEM SHOULD BE 0");

    FREE_CUR_MEM_CONTEXT();
    RELEASE_MEM_CONTEXT();


    StringInfo rangesSize2 = makeStringInfo();
    appendStringInfo(rangesSize2, "ranges_size_double_%d", size);

    MemContext *context2 = NEW_MEM_CONTEXT(rangesSize2->data);
    ACQUIRE_MEM_CONTEXT(context2);
    val = 1;
    List *ranges2 = NIL;
    List *ranges3 = NIL;
    for (int i = 0; i < size + 1; i++) {
        ranges2 = appendToTailOfList(ranges2, createConstInt(val + 10));
        ranges3 = appendToTailOfList(ranges3, createConstInt(val + 10));
        val += 10;
    }

    psAttrInfo *attrInfo2 = createPSAttrInfo(ranges2, "tbl");
    attrInfo2->attrName = "col";

    psAttrInfo *attrInfo3 = createPSAttrInfo(ranges3, "tbl");
    attrInfo3->attrName = "col";

    CLEAR_CUR_MEM_CONTEXT();
    ASSERT_EQUALS_INT(0, memContextSize(context2), "MEM SHOULD BE 0");

    FREE_CUR_MEM_CONTEXT();
    RELEASE_MEM_CONTEXT();
    return PASS;
}
