#ifndef STRING_UTIL_H_
#define STRING_UTIL_H_


#define atoi(s)     string_to_decimal(s)
#define strtoul(s,c,r)      string_to_hex(s)


int isdigit(char c);
int ishexletter(char c);
int isxdigit(char c);
int iscrlf(char c);
int CompareStringsNoCase(const char *s1, const char *s2);
char toupper(char c);





#endif /* STRING_UTIL_H_ */
