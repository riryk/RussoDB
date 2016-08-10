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
uint*        brwlvsa_values; 

uint         brwlvsa_val_headers[3] = 
{ 
	536,            // 0000 0010 0001 1000  536 / 4 = 134
	592,            // 0000 0010 0101 0000  592 / 4 = 148
	536,            // 0000 0010 0001 1000  536 / 4 = 134
};

TEST_GROUP(build_row_with_long_var_size_attr);

SETUP_DEPENDENCIES(build_row_with_long_var_size_attr) 
{
}

GIVEN(build_row_with_long_var_size_attr)
{
	attrs = (RelAttribute)sFakeMemManager.alloc(3 * sizeof(SRelAttribute));
	memset(attrs, 0, 3 * sizeof(SRelAttribute));

	attrs[0].len = -1;
	attrs[1].len = -1;
	attrs[2].len = -1;

	/* Allocate memory for values array. */
	brwlvsa_values = (uint*)sFakeMemManager.alloc(3 * sizeof(uint));

	/* Allocate memory for variable length data. */
    data = (uint*)sFakeMemManager.alloc((134 + 148 + 134) * sizeof(uint));
    data[0] = (uint)brwlvsa_val_headers[0];
    memset(data + 1, 1, (134 - 1) * sizeof(uint));

	data[134] = (uint)brwlvsa_val_headers[1];
    memset(data + 135, 2, (148 - 1) * sizeof(uint));

	data[134 + 148] = (uint)brwlvsa_val_headers[2];
    memset(data + 134 + 149, 2, (134 - 1) * sizeof(uint));

	/* Filling in values array */
    brwlvsa_values[0] = (uint)(&data[0]);
    brwlvsa_values[1] = (uint)(&data[134]);
    brwlvsa_values[2] = (uint)(&data[134 + 148]);

	// Compute SRelRow structure size
	rowSize = AlignDefault(sizeof(SRelRow));

	// Compute header length
    headerLen = offsetof(SRelRowHeader, nullBits);
	headerLen = AlignDefault(headerLen);

	// Compute data length
    dataLen = computeRowSize(attrs, 3, brwlvsa_values, NULL);

	// Allocate memory for our row
	row = (RelRow)sFakeMemManager.alloc(rowSize + headerLen + dataLen);
    memset(row, 0, rowSize + headerLen + dataLen);

	// Compute a pointer to 
    rowHd = (RelRowHeader)((char*)row + rowSize);
}

WHEN(build_row_with_long_var_size_attr) 
{
	dataP = (char*)rowHd + headerLen;

	buildRelRow(attrs, 
		        3, 
				brwlvsa_values, 
				NULL,
                dataP,            
                dataLen,
				&rowHd->mask,
                NULL);
}

TEST_TEAR_DOWN(build_row_with_long_var_size_attr)
{
	sFakeMemManager.freeAll();
}

TEST(build_row_with_long_var_size_attr, 
	 then_data_must_be_written_to_memory)
{
	TEST_ASSERT_EQUAL_UINT32(data[0], (uint)brwlvsa_val_headers[0]);
	TEST_ASSERT_EQUAL_UINT32(data[134], (uint)brwlvsa_val_headers[1]);
	TEST_ASSERT_EQUAL_UINT32(data[134 + 148], (uint)brwlvsa_val_headers[2]);
}

TEST_GROUP_RUNNER(build_row_with_long_var_size_attr)
{
    RUN_TEST_CASE(build_row_with_long_var_size_attr, 
		          then_data_must_be_written_to_memory);
}


