#include "unittestsrunner.h"
#include "common.h"
#include "buffermanager.h"

int main() 
{
	BufferInfo buffer = allocateBuffer(
       NULL,
       NULL,
       'a', 
       22,
       12);

	runAllUnitTests();
    return 0;
}

