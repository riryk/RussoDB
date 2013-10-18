//
//#ifndef IRELROW_H
//#define IRELROW_H
//
//#include "common.h"
//
//
//typedef struct SRowPointer
//{
//	SBlockId     block;
//	uint16       pos;
//} SRowPointer, *RowPointer;
//
//typedef struct SRowFields
//{
//	uint          tranMin;		
//	uint          tranMax;
//	union
//	{
//		uint	  cmdId;		
//		uint      tranVac;	
//	}             field3;
//} SRowFields, *RowFields;
//
//
//typedef struct SDataFields
//{
//	uint		  len;
//	uint		  typeMod;
//	uint		  typeId;
//} SDataFields, *DataFields;
//
//
//typedef struct SRelRowHeader
//{
//	union
//	{
//		SRowFields   fiels;
//		SDataFields  data;
//	}          typeData;
//
//	SRowPointer      curr;		
//	uint16		     mask2;	
//	uint16		     mask;	
//	uint8		     offset;
//    uint8            nullBits;
//} SRelRowHeader, *RelRowHeader;
//
//typedef struct SRelRow
//{ 
//    uint             len;
//    SRowPointer      self;
//    uint		 	 tblId;
//    SRelRowHeader    data;
//} SRelRow, *RelRow;
//
//#endif