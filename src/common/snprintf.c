#include "snprintf.h"
#include "common.h"
#include "error.h"

typedef struct SPrintfInfo
{
	char*       curr;			
	char*       start;
	char*       end; 
} SPrintfInfo, *PrintfInfo;

char*  digits = "0123456789abcdef";

void dostr(
    char*        val, 
	PrintfInfo   info)
{
    int  len = strlen(val);

	while (len > 0)
	{
		int avail = (info->end != NULL)
			        ? info->end - info->curr 
					: len;             

		if (avail <= 0)
			continue;

		avail = Min(avail, len);
		memmove(info->curr, val, avail);
		info->curr += avail;
		val += avail;
		len -= avail;
	}
}

void doint(
	int          val, 
	PrintfInfo   info)
{
	uint		base    = 10;
	char		str[64];
	int			len     = 0;

	do
	{
		str[len++] = digits[val % base];
		val = val / base;
	} while (val);

	while (len > 0) 
		*(info->curr++) = str[--len];
}

Bool doprintf(
	PrintfInfo   info, 
	char*        fmt, 
	va_list      args)
{
    int			 ch;
	char*        str;
	int          num;

    while ((ch = *fmt++) != '\0')
	{
        if (ch != '%')
		{
			*(info->curr++) = ch;
			continue;
		}

        ch = *fmt++;
		if (ch == '\0')
			break;	

		switch (ch)
		{
		    case 's':
				str = va_arg(args, char*);
			    dostr(str, info);
				break;
			case 'u':
				num = (uint)va_arg(args, int);
				doint(num, info);
     			break;
		}

        return False;
	}

	return True;
}

int snprintf(
    char*        s, 
	size_t       count, 
	char*        fmt,
	...)
{
	int			 len;
	va_list		 args;
    SPrintfInfo  info;

	if (s == NULL || count == 0)
		return 0;

	va_start(args, fmt);

	info.start  = s;
	info.curr   = s;
	info.end    = s + count - 1;

	doprintf(&info, fmt, args);

	va_end(args);

	*(info.curr) = '\0';

	return info.curr - info.start;
}

int snprintf_args(
	char*        s, 
	size_t       count, 
	char*        fmt, 
	va_list      args)
{
	SPrintfInfo  info;

	if (s == NULL || count == 0)
		return 0;

	info.start = s;
	info.curr  = s;
	info.end   = s + count - 1;

	if (!doprintf(&info, fmt, args))
	{
		*(info.curr) = '\0';
		errno = ERROR_CODE_BAD_FORMAT;
        return -1;
	}

	*(info.curr) = '\0';
	return info.curr - info.start;
}

