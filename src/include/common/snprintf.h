#include <stdio.h>
#include <stdarg.h>
#include "error.h"

#ifndef SNPRINTF_H
#define SNPRINTF_H

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

#endif