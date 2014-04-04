#include "unittestsrunner.h"
#include "common.h"
#include "buffermanager.h"
#include "processhelper.h"
#include "common.h"

int main(int argc, char* argv[]) 
{
    char            str[80];

    printf("Enter a string: ");
    fgets(str, 10, stdin);

	commonHelper->fillCommonParams(argc, argv);

    if (argc >= 3 && strcmp(argv[3], "subproc") == 0)
	{
		IProcessManager pm;
        pm->subProcessMain(pm, argc, argv);
		return 0;
	}

	runAllUnitTests();
    return 0;
}

