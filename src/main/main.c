#include "unittestsrunner.h"
#include "common.h"
#include "buffermanager.h"
#include "processhelper.h"
#include "common.h"
#include "process_functions.h"

int main(int argc, char* argv[]) 
{   
	commonHelper->fillCommonParams(argc, argv);

    if (argc >= 3 && strcmp(argv[1], "subproc") == 0)
	{
        processManager->subProcessMain(processManager, argc, argv);
		return 0;
	}

	if (argc >= 3 && strcmp(argv[1], "func") == 0)
	{
        int      c;
        char*    funcName;
        
		printf("write a symbol");
		//c        = fgetc(stdin);
		funcName = argv[3];

		process_func_request(funcName);
		return 0;
	}

	runAllUnitTests();
    return 0;
}





