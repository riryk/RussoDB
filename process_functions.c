#include "process_functions.h"
#include <string.h>

void process_func_request(char* funcName)
{
    if (strcmp(funcName, "proc_func_smciap") == 0) 
		proc_func_smciap(); 

    if (strcmp(funcName, "proc_func_smaiap") == 0) 
		proc_func_smaiap(); 

    if (strcmp(funcName, "proc_func_slfstip") == 0) 
		proc_func_slfstip(); 

    if (strcmp(funcName, "proc_func_slaiaps") == 0) 
		proc_func_slaiaps(); 

    if (strcmp(funcName, "proc_func_sstap") == 0) 
		proc_func_sstap(); 
}