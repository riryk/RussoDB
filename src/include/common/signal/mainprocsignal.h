
#include "common.h"

#ifndef MAINPROCSIGNAL_H
#define MAINPROCSIGNAL_H

typedef enum
{
	SIGNAL_TYPE1,
	SIGNAL_TYPE2,
	SIGNAL_TYPE3,
	SIGNAL_TYPE4,
	SIGNAL_TYPE5,
	SIGNAL_TYPE6,
	SIGNAL_TYPE7,
	SIGNAL_TYPE8,

	SIGNALS_NUM
} PMSignalReason;

typedef struct SSignalData
{
	/* array of flags */
	int    flags[SIGNALS_NUM];

	int	   flagsNum;	 
	int	   nextFlag;	  
	int    childFlags[1];
} SSignalData, *SignalData;

#endif
