#include <stdio.h>
#include <stdarg.h>

int snprintf(
    char*        s, 
	size_t       count, 
	char*        fmt,
	...);

int snprintf_args(
	char*        s, 
	size_t       count, 
	char*        fmt, 
	va_list      args);