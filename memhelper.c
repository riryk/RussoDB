#include "memhelper.h"
#include "common.h"

#ifdef _WIN32

#include "windows.h"
#include "psapi.h"

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

ulong getCurrentProcessMemory()
{
    PROCESS_MEMORY_COUNTERS_EX  pmc;
	SIZE_T                      virtualMemUsedByMe;

	HANDLE currProcess = GetCurrentProcess();
	GetProcessMemoryInfo(currProcess, &pmc, sizeof(pmc));
    virtualMemUsedByMe = pmc.PrivateUsage;

    return (ulong)virtualMemUsedByMe;
}

http://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process

 
#endif