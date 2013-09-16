#ifndef STRING_UTIL_H_
#define STRING_UTIL_H_

#define USE_STRING_H            1
#define USE_CTYPE_H             1
#define STRCMPI_UNDEFINED       1

#ifdef USE_STRING_H
#include <string.h>
#endif

#ifdef USE_CTYPE_H
#include <ctype.h>
#endif

#if !USE_STRING_H
#define atoi(s)     string_to_decimal(s)
#define strtoul(s,c,r)      string_to_hex(s)
#endif


#if !USE_STRING_H || STRCMPI_UNDEFINED
int strcmpi(const char *s1, const char *s2);
#define stricmp(a,b)  strcmpi((a), (b))
#endif


#if !USE_CTYPE_H
int isdigit(char c);
int isxdigit(char c);
char toupper(char c);
#endif // USE_CTYPE_H

int ishexletter(char c);
int iscrlf(char c);

unsigned long string_to_hex(char *str);

#endif /* STRING_UTIL_H_ */
