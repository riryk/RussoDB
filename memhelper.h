
#include "common.h"

ulong getTotalVirtualMemory();
ulong getCurrentVirtualMemory();
ulong getCurrentProcessVirtualMemory();
ulong getTotalPhysicalMemory();
ulong getCurrentPhysicalMemory();
ulong getCurrentProcessPhysicalMemory();

void checkMallocFreeCalls();
size_t calculateTotMemSize();