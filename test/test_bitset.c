/*-----------------------------------------------------------------------------
 *
 * test_list.c
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "test_main.h"
#include "common.h"
#include "model/bitset/bitset.h"
#include "utility/string_utils.h"

static rc testCreateBitset(void);
static rc testBitsetAndGet(void);
static rc testBitsetOps(void);
static rc testBitsetSerialization(void);
static rc testBitsetEquals(void);
static rc testLargeBitset(void);

rc
testBitset(void)
{
	RUN_TEST(testCreateBitset(), "test create bitset");
	RUN_TEST(testBitsetAndGet(), "test set and get on bitset");
	RUN_TEST(testBitsetOps(), "test bitwise logical operations on bitsets");
	RUN_TEST(testBitsetSerialization(), "test bitset serialization");
	RUN_TEST(testBitsetEquals(), "test bitset equals");
	RUN_TEST(testLargeBitset(), "test larger bitset");

	return PASS;
}

static rc
testCreateBitset(void)
{
	BitSet *b = newBitSet(10);

	ASSERT_EQUALS_INT(b->length, 10, "length is 10");
	ASSERT_EQUALS_INT(b->numWords, 1, "num words is 1");

	setBit(b, 4, TRUE);
	setBit(b, 5, FALSE);

	ASSERT_EQUALS_INT(b->length, 10, "length is 10");
	ASSERT_EQUALS_INT(b->numWords, 1, "num words is 1");

	return PASS;
}

static rc
testBitsetAndGet(void)
{
	BitSet *b = newBitSet(3);

	setBit(b, 1, TRUE);
	ASSERT_TRUE(isBitSet(b, 1), "get 1 = TRUE");
	ASSERT_FALSE(isBitSet(b, 0), "get 0 = FALSE");
	ASSERT_FALSE(isBitSet(b, 2), "get 2 = FALSE");
	ASSERT_FALSE(isBitSet(b, 15), "get 15 = FALSE");

	return PASS;
}

static rc
testBitsetOps(void)
{
	BitSet *b1 = stringToBitset("00101");
	BitSet *b2 = stringToBitset("11100");

	ASSERT_EQUALS_NODE(stringToBitset("00100"), bitAnd(b1,b2), "and bitset");
	ASSERT_EQUALS_NODE(stringToBitset("11101"), bitOr(b1,b2), "or bitset");
	ASSERT_EQUALS_NODE(stringToBitset("11010"), bitNot(b1), "not bitset");

	return PASS;
}

static rc
testBitsetSerialization(void)
{
	BitSet *b = stringToBitset("11000111");

	ASSERT_EQUALS_STRING("11000111", bitSetToString(b), "test bitset to string");
	ASSERT_EQUALS_STRING("11000111", bitSetToString(copyBitSet(b)), "copy preserves bitset");

	return PASS;
}

static rc
testBitsetEquals(void)
{
	BitSet *b1 = stringToBitset("11000011110");
	BitSet *b2 = stringToBitset("11000011110");

	ASSERT_EQUALS_NODE(b1, b2, "equal bitset");

	return PASS;
}

static rc
testLargeBitset(void)
{
	char *cb = "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
	char *cb2 = "1000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001";
	BitSet *b1 = stringToBitset(cb);
	BitSet *b2 = stringToBitset(cb2);

	ASSERT_EQUALS_STRING(cb, bitSetToString(b1), "all 1s");
	ASSERT_EQUALS_STRING(cb2, bitSetToString(b2), "1 many zeros 1");
	ASSERT_EQUALS_STRING(cb, bitSetToString(bitOr(b1,b2)), "bitor b1 + b2");
	ASSERT_EQUALS_STRING(cb2, bitSetToString(bitAnd(b1,b2)), "bitand b1 + b2");

	return PASS;
}
