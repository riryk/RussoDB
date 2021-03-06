
#include "hashtablehelper.h"


const SIHashtableHelper sHashtableHelper = 
{ 
   calcSegmsNum,
   calcLowMask,
   calcHighMask
};

const IHashtableHelper hashtableHelper = &sHashtableHelper;

int calcSegmsNum(int numHashLists, ulong segmSize)
{
   return (numHashLists - 1) / segmSize + 1;
}

uint calcLowMask(int numHashLists)
{
   return numHashLists - 1;
}

uint calcHighMask(int numHashLists)
{
   return (numHashLists << 1) - 1;
}

