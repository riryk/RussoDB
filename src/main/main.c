#include "unittestsrunner.h"
#include "common.h"
#include "buffermanager.h"

int main(int argc, char* argv[]) 
{
	strcpy(ExecPath, argv[0]);

	runAllUnitTests();
    return 0;
}

