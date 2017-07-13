
#ifndef RelAttribute_h
#define RelAttribute_h

#include "common.h"

typedef struct SRelAttribute
{
   uint		 id;
   Name      name;
   uint      typeId;
   uint      typeParam;   //For varchar type it is the max value

   /*
	 * For a fixed-size type, typlen is the number of bytes we use to
	 * represent a value of this type, e.g. 4 for an int4.	But for a
	 * variable-length type, typlen is negative.  We use -1 to indicate a
	 * "varlena" type (one that has a length word), -2 to indicate a
	 * null-terminated C string.
	 */

   /* If len > 0 we deal with a fixed-size attribute type.
    * For example for int2 we have len = 2 bytes,
	*             for int4 we have len = 4 bytes.   
	* If len == -1 this is a variable-length attribute.
	* If len == -2 this is a null terminated string.
    */
   int16     len;

   Bool      notNull;
   Bool      hasDefault;
   Bool      isInvisible;
   uint      collation;
   /* storageStrategy tells what to do with a row, when it does not fit 
    * into a page. The possible strategies are:
	*       'o', 'ordinary': value must be stored in common, ordinary, plain way.
	*                        It means that one part of the row, which fits into a table,
	*                        goes into one page, whereas another part goes to another part.
	*                        It can cause a large perfomance gap because we need to load 
	*                        two pages into memory.
	*       's': 'secondary' value will be stored in "secondary" table provided the table has one
    *       'c': 'compressed' value can be stored compressed inline
	*       'm': 'mixed' value can be stored compressed inline or in "secondary"
	*/
   char		 storageStrategy;
    /* 'c' = CHAR alignment, ie no alignment needed.
	 * 's' = SHORT alignment (2 bytes on most machines).
	 * 'i' = INT alignment (4 bytes on most machines).
	 * 'd' = DOUBLE alignment (8 bytes on many machines, but by no means all).
	 */
   char      align;
   /* The are attribute types with variable length. These attributes
    * are always passed by reference. The attributes with a constant length
	* are passed by value. */
   Bool      byVal;

   /* Is the number of dimensions for an array domian type */
   int       dimCount;
} SRelAttribute, *RelAttribute;

#endif

