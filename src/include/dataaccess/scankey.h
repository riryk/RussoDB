#include "common.h"

#ifndef Scan_key_h
#define Scan_key_h

typedef int16 AttributeNumber;
typedef uint16 StrategyNumber;

typedef struct FunctionData
{
	int Id;
} FunctionData;

typedef struct ScanKeyData
{
	int			     Properties;
	AttributeNumber  ColumnNumber;
	StrategyNumber   Strategy;
	ObjectId		 StrategySubType;
	ObjectId		 Collation;	
	FunctionData	 Function;
	DataPointer		 DataToCompare;
} ScanKeyData;

typedef ScanKeyData *ScanKey;

#endif





