/*-----------------------------------------------------------------------------
 *
 * test_exception.c
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "exception/exception.h"
#include "test_main.h"
#include "model/integrity_constraints/integrity_constraints.h"

static rc testAttributeClosure(void);
static rc testNormalizeFDs(void);

rc
testIntegrityConstraints(void)
{
    RUN_TEST(testAttributeClosure(), "test computing attribtue closures");
    RUN_TEST(testNormalizeFDs(), "test normalizing FDs");

    return PASS;
}


static rc
testAttributeClosure(void)
{
	FD *f1 = createFD(NULL, MAKE_STR_SET("a", "b"), MAKE_STR_SET("c"));
	FD *f2 = createFD(NULL, MAKE_STR_SET("c"), MAKE_STR_SET("d"));
	FD *f3 = createFD(NULL, MAKE_STR_SET("b", "d"), MAKE_STR_SET("e"));
	List *fds = LIST_MAKE(f1,f2,f3);

	DEBUG_NODE_BEATIFY_LOG("fds: ", fds);

	ASSERT_EQUALS_NODE(MAKE_STR_SET("a", "b", "c", "d", "e"),
					   attributeClosure(fds, MAKE_STR_SET("a", "b"), NULL),
					   "AC(a,b) = a,b,c,d FOR a,b->c, c->d, b,d->e");

	ASSERT_EQUALS_NODE(MAKE_STR_SET("a"),
					   attributeClosure(fds, MAKE_STR_SET("a"), NULL),
					   "AC(a) = a FOR a,b->c, c->d, b,d->e");

	ASSERT_EQUALS_NODE(MAKE_STR_SET("a", "c", "d"),
					   attributeClosure(fds, MAKE_STR_SET("a", "c"), NULL),
					   "AC(a,c) = a,c,d FOR a,b->c, c->d, b,d->e");

    return PASS;
}

static rc
testNormalizeFDs(void)
{

	FD *f1 = createFD(NULL, MAKE_STR_SET("a", "b", "c"), MAKE_STR_SET("c", "d"));
	FD *f2 = createFD(NULL, MAKE_STR_SET("a", "b"), MAKE_STR_SET("a"));
	FD *f3 = createFD(NULL, MAKE_STR_SET("a", "b"), MAKE_STR_SET("c"));
	List *fds = LIST_MAKE(f1,f2,f3);

	List *expected = LIST_MAKE(
		createFD(NULL, MAKE_STR_SET("a", "b", "c"), MAKE_STR_SET("d")),
		createFD(NULL, MAKE_STR_SET("a", "b"), MAKE_STR_SET("c"))
		);

	ASSERT_EQUALS_NODE(expected,
					   normalizeFDs(fds),
					   "normalize FDs a,b,c->c,d ; a,b->a; a,b,->c into a,b,c->d and a,b->c");

	return PASS;
}
