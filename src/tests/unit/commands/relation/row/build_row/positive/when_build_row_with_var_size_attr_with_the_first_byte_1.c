#include "hashtable.h"
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "relrowmanager.h"
#include "relrow_helper.h"
#include "stddef.h"

RelAttribute attrs;
size_t       rowSize, headerLen, dataLen;
uint*        data;
RelRow       row;
RelRowHeader rowHd;
char*        dataP;
uint*        brwvsawtfb1_values; 

uint         brwvsawtfb1_val_headers[3] = 
{ 
	2049,          // 0000 1000 0000 0001   8  = 4*2
	3073,          // 0000 1100 0000 0001   12 = 4*3
	8193,          // 0010 0000 0000 0001   32 = 4*8
};

TEST_GROUP(build_row_with_var_size_attr_with_the_first_byte_1);

SETUP_DEPENDENCIES(build_row_with_var_size_attr_with_the_first_byte_1) 
{
}

GIVEN(build_row_with_var_size_attr_with_the_first_byte_1)
{
	attrs = (RelAttribute)sFakeMemManager.alloc(3 * sizeof(SRelAttribute));
	memset(attrs, 0, 3 * sizeof(SRelAttribute));

	attrs[0].len = -1;
	attrs[1].len = -1;
	attrs[2].len = -1;

	/* Allocate memory for values array. */
	brwvsawtfb1_values = (uint*)sFakeMemManager.alloc(3 * sizeof(uint));

	/* Allocate memory for variable length data. */
    data = (uint*)sFakeMemManager.alloc((2 + 3 + 8) * sizeof(uint));
    data[0] = brwvsawtfb1_val_headers[0];
	data[1] = 122;

	data[2] = brwvsawtfb1_val_headers[1];
    data[3] = 111;
    data[4] = 573047305;

    data[5] = brwvsawtfb1_val_headers[2];
	data[6] = 13444;
    data[7] = 171047305;
    data[8] = 12;
    data[9] = 11222;
    data[10] = 1;
    data[11] = 5;
    data[12] = 77777;

	/* Filling in values array */
    brwvsawtfb1_values[0] = (uint)(&data[0]);
    brwvsawtfb1_values[1] = (uint)(&data[2]);
    brwvsawtfb1_values[2] = (uint)(&data[5]);

	// Compute SRelRow structure size
	rowSize = AlignDefault(sizeof(SRelRow));

	// Compute header length
    headerLen = offsetof(SRelRowHeader, nullBits);
	headerLen = AlignDefault(headerLen);

	// Compute data length
    dataLen = computeRowSize(attrs, 3, brwvsawtfb1_values, NULL);

	// Allocate memory for our row
	row = (RelRow)sFakeMemManager.alloc(rowSize + headerLen + dataLen);
    memset(row, 0, rowSize + headerLen + dataLen);

	// Compute a pointer to 
    rowHd = (RelRowHeader)((char*)row + rowSize);
}

WHEN(build_row_with_var_size_attr_with_the_first_byte_1) 
{
	dataP = (char*)rowHd + headerLen;

	buildRelRow(attrs, 
		        3, 
				brwvsawtfb1_values, 
				NULL,
                dataP,            
                dataLen,
				&rowHd->mask,
                NULL);
}

TEST_TEAR_DOWN(build_row_with_var_size_attr_with_the_first_byte_1)
{
	sFakeMemManager.freeAll();
}

TEST(build_row_with_var_size_attr_with_the_first_byte_1, 
	 then_data_must_be_written_to_memory)
{
    uint* dataPInt = (uint*)dataP;

	TEST_ASSERT_EQUAL_UINT32(dataPInt[0], brwvsawtfb1_val_headers[0]);
    TEST_ASSERT_EQUAL_UINT32(dataPInt[1], 122);
	TEST_ASSERT_EQUAL_UINT32(dataPInt[2], brwvsawtfb1_val_headers[1]);
    TEST_ASSERT_EQUAL_UINT32(dataPInt[3], 111);
    TEST_ASSERT_EQUAL_UINT32(dataPInt[4], 573047305);
    TEST_ASSERT_EQUAL_UINT32(dataPInt[5], brwvsawtfb1_val_headers[2]);
    TEST_ASSERT_EQUAL_UINT32(dataPInt[6], 13444);
    TEST_ASSERT_EQUAL_UINT32(dataPInt[7], 171047305);
    TEST_ASSERT_EQUAL_UINT32(dataPInt[8], 12);
    TEST_ASSERT_EQUAL_UINT32(dataPInt[9], 11222);
    TEST_ASSERT_EQUAL_UINT32(dataPInt[10], 1);
    TEST_ASSERT_EQUAL_UINT32(dataPInt[11], 5);
    TEST_ASSERT_EQUAL_UINT32(dataPInt[12], 77777);
}

TEST_GROUP_RUNNER(build_row_with_var_size_attr_with_the_first_byte_1)
{
    RUN_TEST_CASE(build_row_with_var_size_attr_with_the_first_byte_1, 
		          then_data_must_be_written_to_memory);
}


