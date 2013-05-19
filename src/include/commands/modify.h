
#define MAX_ATTRIBUTES_COUNT 1664

#define TYPE_REC_OID		 2249

#define TRAN_MASK			 0xFFE0	

#define ATTR_MASK			 0x07FF

#define TRAN_MAX_INVALID	 0x0800

#define MIN_FILLFACTOR			10

#define DEFAULT_FILLFACTOR		100

#define TABLE_INSERT_SKIP_FREE_SPACE_MANAGER 0x0002

#define ROW_UNUSED		0		/* unused (should always have lp_len=0) */
#define ROW_NORMAL		1	

#define BUFFER_DIRTY    1 << 0