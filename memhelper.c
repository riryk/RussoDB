#include "memhelper.h"
#include "common.h"
#include "unity_fixture.h"

#ifdef _WIN32

#include "windows.h"
#include "Psapi.h"

#pragma comment(lib, "Psapi.lib")

/* Note: The name "TotalPageFile" is a bit misleading here. 
 * In reality this parameter gives the "Virtual Memory Size", 
 * which is size of swap file plus installed RAM.
 */
ulong getTotalVirtualMemory()
{
    MEMORYSTATUSEX    memInfo;
    DWORDLONG         totalVirtualMem;

    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    GlobalMemoryStatusEx(&memInfo);
    totalVirtualMem = memInfo.ullTotalPageFile;

	return (ulong)totalVirtualMem;
}

ulong getCurrentVirtualMemory()
{
    MEMORYSTATUSEX    memInfo;
    DWORDLONG         virtualMemUsed;

    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    GlobalMemoryStatusEx(&memInfo);
    virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;

	return (ulong)virtualMemUsed;
}

ulong getCurrentProcessVirtualMemory()
{
    PROCESS_MEMORY_COUNTERS_EX  pmc;
	SIZE_T                      virtualMemUsedByMe;

	HANDLE currProcess = GetCurrentProcess();
	GetProcessMemoryInfo(currProcess, &pmc, sizeof(pmc));
	virtualMemUsedByMe = pmc.PrivateUsage;

    return (ulong)virtualMemUsedByMe;
}

ulong getTotalPhysicalMemory()
{
    MEMORYSTATUSEX    memInfo;
    DWORDLONG         totalPhysicalMem;

    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    GlobalMemoryStatusEx(&memInfo);
	totalPhysicalMem = memInfo.ullTotalPhys;

	return (ulong)totalPhysicalMem;
}

ulong getCurrentPhysicalMemory()
{
    MEMORYSTATUSEX    memInfo;
    DWORDLONG         physicalMemUsed;

    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    GlobalMemoryStatusEx(&memInfo);
	physicalMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;

	return (ulong)physicalMemUsed;
}

ulong getCurrentProcessPhysicalMemory()
{  
    PROCESS_MEMORY_COUNTERS_EX  pmc;
	SIZE_T                      physicalMemUsedByMe;

	HANDLE currProcess = GetCurrentProcess();
	GetProcessMemoryInfo(currProcess, &pmc, sizeof(pmc));
	physicalMemUsedByMe = pmc.WorkingSetSize;

    return (ulong)physicalMemUsedByMe;
}

#endif

Bool isInArray(int* arr, int count, int val)
{
    int i;

	for (i = 0; i < count; i++)
       if (arr[i] == val)
	      return True;

	return False;
}

void checkMallocFreeCalls()
{
	int    i;
    Bool   isInArr;

	TEST_ASSERT_EQUAL_UINT32(malloc_count, free_count);

	for (i = 0; i < malloc_count; i++)
	{
        isInArr = isInArray(free_mem_addrs, 
			                free_count, 
							malloc_mem_addrs[i]);

        TEST_ASSERT_TRUE(isInArr);
	}
}

size_t calculateTotMemSize()
{
	int     i;
	size_t  totSize = 0; 

    for (i = 0; i < malloc_count; i++)
	{
        totSize += malloc_mem_sizes[i];
	}

	return totSize;
}