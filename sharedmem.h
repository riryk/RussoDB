
#ifndef SHARED_MEM_H
#define SHARED_MEM_H

#ifndef PG_SHMEM_H
#define PG_SHMEM_H

typedef struct PGShmemHeader	/* standard header for all Postgres shmem */
{
	int32		magic;			/* magic # to identify Postgres segments */
#define PGShmemMagic  679834894
	pid_t		creatorPID;		/* PID of creating process */
	Size		totalsize;		/* total size of segment */
	Size		freeoffset;		/* offset to first free space */
	void	   *index;			/* pointer to ShmemIndex table */
#ifndef WIN32					/* Windows doesn't have useful inode#s */
	dev_t		device;			/* device data directory is on */
	ino_t		inode;			/* inode number of data directory */
#endif
} PGShmemHeader;
