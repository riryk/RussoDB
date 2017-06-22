
#include "file_manager_helper.h"

char*        fnames[3000];
int          fds[3000];
void*        mem;
int          fcount = 0;

void doCreateTestFiles(int filesCount)
{
    int   i;
    int   expSize = sizeof("test_result/test.txt") + sizeof(int);
    char* pattern = "test_result/test%u.txt";
    
    fcount = filesCount;
	mem    = malloc(expSize * fcount);

    for (i = 0; i < fcount; i++)
	{
        fnames[i] = (char*)mem + i * expSize;
		//_snprintf_s(fnames[i], expSize, 100, pattern, i);
	}
}

void doFreeTestFiles()
{
	int i;

    for (i = 0; i < fcount; i++)
	{
       if (FILE_EXISTS(fnames[i]))
	   {
	   	   close(fds[i]);
	   	   remove(fnames[i]);
	   }
	}

	free(mem);
}

Bool existsInArray(int index, int* arr, int arrcount)
{
    int  i;
    for (i = 0; i < arrcount; i++)
	    if (arr[i] == index)
		   return True;
    return False;
}

void doFreeNotClosedTestFiles(int* alreadyClosed, int count)
{
    int i;

    for (i = 0; i < fcount; i++)
	{
       if (FILE_EXISTS(fnames[i]))
	   {
		   if (!existsInArray(fds[i], alreadyClosed, count))
	   	      close(fds[i]);
	   	   remove(fnames[i]);
	   }
	}

	free(mem);
}

    