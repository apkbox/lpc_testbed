#include "string_util.h"


#if !USE_CTYPE_H
int isdigit(char c)
{
    return (c >= '0' && c <= '9');
}


int ishexletter(char c)
{
    return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}


int isxdigit(char c)
{
    return isdigit(c) || ishexletter(c);
}


char toupper(char c)
{
    if (c >= 'a' && c <= 'z')
        return c - 0x20;
    else
        return c;
}
#endif

int iscrlf(char c)
{
    return (c == '\r' || c == '\n');
}


#if !USE_STRING_H || STRCMPI_UNDEFINED
int strcmpi(const char *s1, const char *s2)
{
    while (*s1 != 0 && *s2 != 0) {
        if (*s1++ != *s2++)
            return 1;
    }

    return !(*s1 == 0 && *s2 == 0);
}
#endif


unsigned int string_to_decimal( char *str )
{
    unsigned int v = 0;
    char *p = str;

    while( !isdigit( *p ) && *p )
        p ++;

    while( isdigit( *p ) )
        v = v * 10 + ( *p ++ - '0' );

    return v;
}


unsigned long string_to_hex(char *str)
{
    unsigned long v = 0;
    char *p = str;

    while( !isxdigit( *p ) && *p )
        p ++;

    while( isxdigit( *p ) ) {
        char c = *p ++;
        if( c >= 'A' ) {
            if( c >= 'a' )
                c -= 0x20;      // to lower case
            c -= 'A' - 0x0A;
        }
        else {
            c -= '0';
        }
        v = ( v << 4 ) | c;
    }

    return v;
}
