#ifndef STRING_H
#define STRING_H

#include "types.h"

// memory
void*  memchr(const void *ptr, i16t value, u32t num);
i16t   memcmp(const void *ptr1, const void *ptr2, u32t num);
void*  memcpy(void *dest, const void *src, u32t num);
void*  memmove(void *dest, const void *src, u32t num);
void*  memset(void *ptr, i16t value, u32t num);

// string operations
string strcat(string restrict dest, string restrict src);
string strchrnul(string str, i16t character);
string strchr(string str, i16t character);
i16t strcmp(cstring str1, cstring str2);
i16t strcoll(string str1, string str2); //TODO: locale-aware strcmp
string strcpy(string dest, cstring src);
u32t strcspn(cstring str, cstring reject); // length before any char in reject
string strerror(i16t errnum); //TODO: no error table
u32t strlen(cstring str);
string strncat(string restrict dest, string restrict src, u32t num);
i16t strncmp(string str1, string str2, u32t num);
string strncpy(string dest, cstring src, u32t num);
string strpbrk(cstring str, cstring accept); // first char in str that's in accept
string strrchr(string str, i16t character);
u32t strspn(cstring str, cstring accept);  // length of leading chars all in accept
string strstr(string str, string substr);
string strtok(string restrict s, string restrict sep);
u32t strxfrm(string dest, string src, u32t num); //TODO: relies on locale
string stpncpy(string restrict dest, cstring restrict src, u32t num);
string stpcpy(string restrict dest, cstring restrict src);

#include "../src/string.c"

#endif /* STRING_H */