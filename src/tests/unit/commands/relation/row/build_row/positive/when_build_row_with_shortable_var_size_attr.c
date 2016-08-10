#include "hashtable.h"
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "relrowmanager.h"
#include "relrow_helper.h"
#include "stddef.h"

RelAttribute attrs;
size_t       rowSize, headerLen, dataLen;
uint16*      data;
RelRow       row;
RelRowHeader rowHd;
char*        dataP;
uint*        brwsvsa_values; 

uint         brwsvsa_val_headers[3] = 
{ 
	24,            // 0000 0000 0001 1000   6
	40,            // 0000 0000 0010 1000   10  
	56,            // 0000 0000 0011 1000   14   
};

TEST_GROUP(build_row_with_shortable_var_size_attr);

SETUP_DEPENDENCIES(build_row_with_shortable_var_size_attr) 
{
}

GIVEN(build_row_with_shortable_var_size_attr)
{
	attrs = (RelAttribute)sFakeMemManager.alloc(3 * sizeof(SRelAttribute));
	memset(attrs, 0, 3 * sizeof(SRelAttribute));

	attrs[0].len = -1;
	attrs[1].len = -1;
	attrs[2].len = -1;

	/* Allocate memory for values array. */
	brwsvsa_values = (uint*)sFakeMemManager.alloc(3 * sizeof(uint));

	/* Allocate memory for variable length data. */
    data = (uint16*)sFakeMemManager.alloc((3 + 5 + 7) * sizeof(uint16));
    data[0] = (uint16)brwsvsa_val_headers[0];
	data[1] = 0;
    data[2] = 123;

	data[3] = (uint16)brwsvsa_val_headers[1];
    data[4] = 0;
    data[5] = 57304;
    data[6] = 53314;
    data[7] = 11;

    data[8] = (uint16)brwsvsa_val_headers[2];
	data[9] = 0;
    data[10] = 17104;
    data[11] = 12;
    data[12] = 11222;
    data[13] = 1;
    data[14] = 3333;

	/* Filling in values array */
    brwsvsa_values[0] = (uint)(&data[0]);
    brwsvsa_values[1] = (uint)(&data[3]);
    brwsvsa_values[2] = (uint)(&data[8]);

	// Compute SRelRow structure size
	rowSize = AlignDefault(sizeof(SRelRow));

	// Compute header length
    headerLen = offsetof(SRelRowHeader, nullBits);
	headerLen = AlignDefault(headerLen);

	// Compute data length
    dataLen = computeRowSize(attrs, 3, brwsvsa_values, NULL);

	// Allocate memory for our row
	row = (RelRow)sFakeMemManager.alloc(rowSize + headerLen + dataLen);
    memset(row, 0, rowSize + headerLen + dataLen);

	// Compute a pointer to 
    rowHd = (RelRowHeader)((char*)row + rowSize);
}

WHEN(build_row_with_shortable_var_size_attr) 
{
	dataP = (char*)rowHd + headerLen;

	buildRelRow(attrs, 
		        3, 
				brwsvsa_values, 
				NULL,
                dataP,            
                dataLen,
				&rowHd->mask,
                NULL);
}

TEST_TEAR_DOWN(build_row_with_shortable_var_size_attr)
{
	sFakeMemManager.freeAll();
}

TEST(build_row_with_shortable_var_size_attr, 
	 then_data_must_be_written_to_memory)
{
    uint16* dataPInt16;

	TEST_ASSERT_EQUAL_UINT32((int)dataP[0], 3 * 2 + 1);
	dataP += 1; 
    dataPInt16 = (uint16*)dataP;
    TEST_ASSERT_EQUAL_UINT32(dataPInt16[0], 123);
    dataPInt16 += 1;
    
	dataP = dataPInt16;
	TEST_ASSERT_EQUAL_UINT32((int)dataP[0], 7 * 2 + 1);
    dataP += 1; 
    dataPInt16 = (uint16*)dataP; 
    TEST_ASSERT_EQUAL_UINT32(dataPInt16[0], 57304);
    TEST_ASSERT_EQUAL_UINT32(dataPInt16[1], 53314);
    TEST_ASSERT_EQUAL_UINT32(dataPInt16[2], 11);
    dataPInt16 += 3;

    dataP = dataPInt16;
    TEST_ASSERT_EQUAL_UINT32((int)dataP[0], 11 * 2 + 1);
    dataP += 1; 
    dataPInt16 = (uint16*)dataP; 
    TEST_ASSERT_EQUAL_UINT32(dataPInt16[0], 17104);
    TEST_ASSERT_EQUAL_UINT32(dataPInt16[1], 12);
    TEST_ASSERT_EQUAL_UINT32(dataPInt16[2], 11222);
    TEST_ASSERT_EQUAL_UINT32(dataPInt16[3], 1);
    TEST_ASSERT_EQUAL_UINT32(dataPInt16[4], 3333);
}

TEST_GROUP_RUNNER(build_row_with_shortable_var_size_attr)
{
    RUN_TEST_CASE(build_row_with_shortable_var_size_attr, 
		          then_data_must_be_written_to_memory);
}


