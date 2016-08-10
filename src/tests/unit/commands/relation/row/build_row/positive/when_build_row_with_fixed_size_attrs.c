#include "hashtable.h"
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "relrowmanager.h"
#include "relrow_helper.h"
#include "stddef.h"

RelAttribute attrs;
size_t       rowSize, headerLen, dataLen;
RelRow       row;
RelRowHeader rowHd;
char*        dataP;

uint         brwfsa_values[6] = 
{ 
	115,        // 0111 0011
	29555,      // 0111 0011 0111 0011
	1936946035, // 0111 0011 0111 0011 0111 0011 0111 0011
	5,          // 101
	5,          // 101
	5           // 101
};

TEST_GROUP(build_row_with_fixed_size_attrs);

SETUP_DEPENDENCIES(build_row_with_fixed_size_attrs) 
{
}

GIVEN(build_row_with_fixed_size_attrs)
{
    attrs = (RelAttribute)sFakeMemManager.alloc(6 * sizeof(SRelAttribute));
    memset(attrs, 0, 6 * sizeof(SRelAttribute));

    DoSetFixedSize(&attrs[0], 1, 'c');
	DoSetFixedSize(&attrs[1], 2, 's');
	DoSetFixedSize(&attrs[2], 4, 'i');
	DoSetFixedSize(&attrs[3], 1, 'c');
	DoSetFixedSize(&attrs[4], 2, 's');
	DoSetFixedSize(&attrs[5], 4, 'i');

	// Compute SRelRow structure size
	rowSize = AlignDefault(sizeof(SRelRow));

	// Compute header length
    headerLen = offsetof(SRelRowHeader, nullBits);
	headerLen = AlignDefault(headerLen);

	// Compute data length
    dataLen = computeRowSize(attrs, 6, brwfsa_values, NULL);

	// Allocate memory for our row
	row = (RelRow)sFakeMemManager.alloc(rowSize + headerLen + dataLen);
    memset(row, 0, rowSize + headerLen + dataLen);

	// Compute a pointer to 
    rowHd = (RelRowHeader)((char*)row + rowSize);
}

WHEN(build_row_with_fixed_size_attrs) 
{
	dataP = (char*)rowHd + headerLen;

	buildRelRow(attrs, 
		        6, 
				brwfsa_values, 
				NULL,
                dataP,            
                dataLen,
				&rowHd->mask,
                NULL);
}

TEST_TEAR_DOWN(build_row_with_fixed_size_attrs)
{
}

TEST(build_row_with_fixed_size_attrs, then_data_must_be_written_to_memory)
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
    char   savedVal1 = *(char*)dataP;
    uint16 savedVal2 = *(uint16*)(dataP + 2);
    uint   savedVal3 = *(uint*)(dataP + 4);

    char   savedVal4 = *(char*)(dataP + 8);
    uint16 savedVal5 = *(uint16*)(dataP + 10);
    uint   savedVal6 = *(uint*)(dataP + 12);

	TEST_ASSERT_EQUAL_UINT32(savedVal1, brwfsa_values[0]);
    TEST_ASSERT_EQUAL_UINT32(savedVal2, brwfsa_values[1]);
    TEST_ASSERT_EQUAL_UINT32(savedVal3, brwfsa_values[2]);

    TEST_ASSERT_EQUAL_UINT32(savedVal4, brwfsa_values[3]);
    TEST_ASSERT_EQUAL_UINT32(savedVal5, brwfsa_values[4]);
    TEST_ASSERT_EQUAL_UINT32(savedVal6, brwfsa_values[5]);
}

TEST_GROUP_RUNNER(build_row_with_fixed_size_attrs)
{
    RUN_TEST_CASE(build_row_with_fixed_size_attrs, then_data_must_be_written_to_memory);
}


