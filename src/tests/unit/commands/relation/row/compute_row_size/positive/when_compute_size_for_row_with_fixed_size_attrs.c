#include "hashtable.h"
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "relrowmanager.h"
#include "relrow_helper.h"

RelAttribute attrs;
size_t       size;
uint         values[6] = 
{ 
	115,        // 0111 0011
	29555,      // 0111 0011 0111 0011
	1936946035, // 0111 0011 0111 0011 0111 0011 0111 0011
	5,          // 101
	5,          // 101
	5           // 101
};

TEST_GROUP(compute_size_for_row_with_fixed_size_attrs);

SETUP_DEPENDENCIES(compute_size_for_row_with_fixed_size_attrs) 
{
}

GIVEN(compute_size_for_row_with_fixed_size_attrs)
{
	attrs = (RelAttribute)sFakeMemManager.alloc(6 * sizeof(SRelAttribute));
	memset(attrs, 0, 6 * sizeof(SRelAttribute));

    DoSetFixedSize(&attrs[0], 1, 'c');
	DoSetFixedSize(&attrs[1], 2, 's');
	DoSetFixedSize(&attrs[2], 4, 'i');
	DoSetFixedSize(&attrs[3], 1, 'c');
	DoSetFixedSize(&attrs[4], 2, 's');
    DoSetFixedSize(&attrs[5], 4, 'i');
}

WHEN(compute_size_for_row_with_fixed_size_attrs) 
{
	size = computeRowSize(attrs, 6, values, NULL);
}

TEST_TEAR_DOWN(compute_size_for_row_with_fixed_size_attrs)
{
	sFakeMemManager.freeAll();
}

TEST(compute_size_for_row_with_fixed_size_attrs, then_alignment_must_be_applied)
{
   /* Let's compute the expected size with alignment.
    * From the start suppose that no alignment is applied
	* |-|--|----|-|--|----|     expected size is 14.
	* 0 1  3    7 8  10   14
	* Alignment is needed for the operation syztem virtual memory manager.
	* When data is not type-aligned it needs to load two blocks into memory
	* to retrieve only one item.
	* When data is aligned it looks like:
	* |-|-|--|----|-|-|--|----|  expected size is 16.
	* 0 1 2  4    8 910 12   16 
    */
	TEST_ASSERT_EQUAL_UINT32(size, 16);
}

TEST_GROUP_RUNNER(compute_size_for_row_with_fixed_size_attrs)
{
    RUN_TEST_CASE(compute_size_for_row_with_fixed_size_attrs, then_alignment_must_be_applied);
}


