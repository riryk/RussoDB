
MainSignalData MainSignalState = NULL;

void mainProcSignalInit()
{
	Bool	 found;

	MainSignalState = (MainSignalData)
		ShmemInitStruct("PMSignalState", PMSignalShmemSize(), &found);

	if (!found)
	{
		MemSet(PMSignalState, 0, PMSignalShmemSize());
		PMSignalState->num_child_flags = MaxLivePostmasterChildren();
	}
}