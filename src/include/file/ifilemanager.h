
#ifndef IFILE_MANAGER_H
#define IFILE_MANAGER_H

typedef struct SIFileManager
{
	int (*openFile)(char* fname, int fileFlags, int fileMode);
	long (*restoreFilePos)(int fileId, long offset, int placeToPut);
} SIFileManager, *IFileManager;


#endif