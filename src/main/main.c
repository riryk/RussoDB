#include "unittestsrunner.h"
#include "common.h"
#include "buffermanager.h"
#include "processhelper.h"
#include "common.h"

int main(int argc, char* argv[]) 
{    
	commonHelper->fillCommonParams(argc, argv);

    if (argc >= 3 && strcmp(argv[1], "subproc") == 0)
	{
        processManager->subProcessMain(processManager, argc, argv);
		return 0;
	}

	runAllUnitTests();
    return 0;
}

