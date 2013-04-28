#include "error.h"

void Fatal(long Code, char* Message,...)
{
    va_list arglist;

    va_start(arglist, Message);

    Log(Code, Message, arglist);

    va_end(arglist); 

    abort();
}

void Log(long Code, char* Message,...)
{
    va_list arglist;
   
    va_start(arglist, Message);
    
	printf("Code: %d, Message: ", Code);
	printf(Message, arglist);
   
    va_end(arglist); 
}






