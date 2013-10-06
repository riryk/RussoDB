#include "ihashtablehelper.h"

#ifndef HASHTABLE_HELPER_H
#define HASHTABLE_HELPER_H

extern const SIHashtableHelper sHashtableHelper;
extern const IHashtableHelper hashtableHelper;

int calcSegmsNum(int numHashLists, ulong segmSize);
uint calcLowMask(int numHashLists);
uint calcHighMask(int numHashLists);

#endif