#include "process_functions.h"
#include <string.h>

void process_func_request(char* funcName)
{
    if (strcmp(funcName, "proc_func_smciap") == 0) 
		proc_func_smciap(); 
}