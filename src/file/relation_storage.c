
#include "relation_storage.h"

int	   SimultaneousProcessMode = 0;
int    FlushToDiskProcess      = 0;
int    StartupProcess          = 0;

static int SyncNumber          = 0;
static int SyncInProgress      = 0;

static int EnableFsync         = 1;

typedef struct StorageRelation
{
} StorageRelation;

typedef struct RelationFileInfo
{
	unsigned int		tableSpaceId;		/* tablespace */
	unsigned int 		databaseId;		    /* database */
	unsigned int		relationId;		    /* relation */
} RelationFileInfo;

typedef struct RelFileNodeBackend
{
	RelationFileInfo   node;
	int	               backend;
} RelFileNodeBackend;

typedef struct
{
	RelationFileInfo    Key;	                      /* hash table key */
	int             	SyncNumber;		
	Set*    FSyncRequests[MAX_REL_PARTS_NUMBER + 1];  /* fsync requests */
} FSyncRequestItem;

typedef struct StorageRelationData
{
	RelationFileInfo    Key;
	/* hash table key */
} StorageRelationData;

static HashTable *requestsTable = NULL;
static HashTable *storageRelationTable = NULL;

void RelationStorageInit()
{
	if (!SimultaneousProcessMode || FlushToDiskProcess || StartupProcess)
	{
		HashTableSettings		hashTableSett;

		MemSet(&hashTableSett, 0, sizeof(hashTableSett));

		hashTableSett.KeyLength = sizeof(RelationFileInfo);
		hashTableSett.ValueLength = sizeof(FSyncRequestItem);
        hashTableSett.HashFunc = HashSimple;

		requestsTable = HashTableCreate("Requests Table",
									    100L,
									    &hashTableSett,
								        HASH_FUNCTION);
	}
}

StorageRelation RelationOpen(RelationFileInfo rnode, int backend)
{
	RelFileNodeBackend  brnode;
	StorageRelation     reln;
	int		            found;

	if (storageRelationTable == NULL)
	{
		HashTableSettings		hashTableSett;

		MemSet(&hashTableSett, 0, sizeof(hashTableSett));

		hashTableSett.KeyLength = sizeof(RelationFileInfo);
		hashTableSett.ValueLength = sizeof(StorageRelationData);
        hashTableSett.HashFunc = TagHash;

		storageRelationTable = HashTableCreate("Storage Relation Table",
									    400,
									    &hashTableSett,
								        HASH_FUNCTION);
	}
	
	brnode.node = rnode;
	brnode.backend = backend;

	reln = (StorageRelation) hash_search(SMgrRelationHash,
									  (void *) &brnode,
									  HASH_ENTER, &found);

	/* Initialize it if not present before */
	if (!found)
	{
		int			forknum;

		/* hash_search already filled in the lookup key */
		reln->smgr_owner = NULL;
		reln->smgr_targblock = InvalidBlockNumber;
		reln->smgr_fsm_nblocks = InvalidBlockNumber;
		reln->smgr_vm_nblocks = InvalidBlockNumber;
		reln->smgr_which = 0;	/* we only have md.c at present */

		/* mark it not open */
		for (forknum = 0; forknum <= MAX_FORKNUM; forknum++)
			reln->md_fd[forknum] = NULL;

		/* place it at head of unowned list (to make smgrsetowner cheap) */
		reln->next_unowned_reln = first_unowned_reln;
		first_unowned_reln = reln;
	}

	return reln;
}

void RelationWritesSync()
{
	HashSequenceItem  hashSequence;
	FSyncRequestItem* request;

	HashSequenceInit(&hashSequence, &requestsTable);
	while ((request = (FSyncRequestItem*)HashSequenceSearch(&hashSequence)) != NULL)
	{
		request->SyncNumber = SyncNumber;
	}

	SyncNumber++;
	SyncInProgress = 1;

	/* Now scan the hashtable for fsync requests to process */
	//absorb_counter = FSYNCS_PER_ABSORB;
	
	HashSequenceInit(&hashSequence, &requestsTable);
	while ((request = (FSyncRequestItem*)HashSequenceSearch(&hashSequence)) != NULL)
	{
		Fork	fork;

		if (request->SyncNumber == SyncNumber)
			continue;

		for (fork = 0; fork <= INIT_FORK; fork++)
		{
			Set*  requests = request->FSyncRequests[fork];
			int			segno;

			request->FSyncRequestsv[fork] = NULL;
			//entry->canceled[forknum] = false;

			while ((segno = Set_GetFirstMember(requests)) >= 0)
			{
				int			failures;

				if (!EnableFsync)
					continue;

				for (failures = 0;;failures++) /* loop exits at "break" */
				{
					//StorageRelation  storageRel;
					//MdfdVec    *seg;
					//char	   *path;
					//int			save_errno;

					
					reln = smgropen(entry->rnode, InvalidBackendId);

					/* Attempt to open and fsync the target segment */
					seg = _mdfd_getseg(reln, forknum,
							 (BlockNumber) segno * (BlockNumber) RELSEG_SIZE,
									   false, EXTENSION_RETURN_NULL);

					INSTR_TIME_SET_CURRENT(sync_start);

					if (seg != NULL &&
						FileSync(seg->mdfd_vfd) >= 0)
					{
						/* Success; update statistics about sync timing */
						INSTR_TIME_SET_CURRENT(sync_end);
						sync_diff = sync_end;
						INSTR_TIME_SUBTRACT(sync_diff, sync_start);
						elapsed = INSTR_TIME_GET_MICROSEC(sync_diff);
						if (elapsed > longest)
							longest = elapsed;
						total_elapsed += elapsed;
						processed++;
						if (log_checkpoints)
							elog(DEBUG1, "checkpoint sync: number=%d file=%s time=%.3f msec",
								 processed,
								 FilePathName(seg->mdfd_vfd),
								 (double) elapsed / 1000);

						break;	/* out of retry loop */
					}

					/* Compute file name for use in message */
					save_errno = errno;
					path = _mdfd_segpath(reln, forknum, (BlockNumber) segno);
					errno = save_errno;

					/*
					 * It is possible that the relation has been dropped or
					 * truncated since the fsync request was entered.
					 * Therefore, allow ENOENT, but only if we didn't fail
					 * already on this file.  This applies both for
					 * _mdfd_getseg() and for FileSync, since fd.c might have
					 * closed the file behind our back.
					 *
					 * XXX is there any point in allowing more than one retry?
					 * Don't see one at the moment, but easy to change the
					 * test here if so.
					 */
					if (!FILE_POSSIBLY_DELETED(errno) ||
						failures > 0)
						ereport(ERROR,
								(errcode_for_file_access(),
								 errmsg("could not fsync file \"%s\": %m",
										path)));
					else
						ereport(DEBUG1,
								(errcode_for_file_access(),
						errmsg("could not fsync file \"%s\" but retrying: %m",
							   path)));
					pfree(path);

					/*
					 * Absorb incoming requests and check to see if a cancel
					 * arrived for this relation fork.
					 */
					AbsorbFsyncRequests();
					absorb_counter = FSYNCS_PER_ABSORB; /* might as well... */

					if (entry->canceled[forknum])
						break;
				}				/* end retry loop */
			}
			bms_free(requests);
		}

		/*
		 * We've finished everything that was requested before we started to
		 * scan the entry.	If no new requests have been inserted meanwhile,
		 * remove the entry.  Otherwise, update its cycle counter, as all the
		 * requests now in it must have arrived during this cycle.
		 */
		for (forknum = 0; forknum <= MAX_FORKNUM; forknum++)
		{
			if (entry->requests[forknum] != NULL)
				break;
		}
		if (forknum <= MAX_FORKNUM)
			entry->cycle_ctr = mdsync_cycle_ctr;
		else
		{
			/* Okay to remove it */
			if (hash_search(pendingOpsTable, &entry->rnode,
							HASH_REMOVE, NULL) == NULL)
				elog(ERROR, "pendingOpsTable corrupted");
		}
	}							/* end loop over hashtable entries */

	/* Return sync performance metrics for report at checkpoint end */
	CheckpointStats.ckpt_sync_rels = processed;
	CheckpointStats.ckpt_longest_sync = longest;
	CheckpointStats.ckpt_agg_sync_time = total_elapsed;

	/* Flag successful completion of mdsync */
	mdsync_in_progress = false;
}