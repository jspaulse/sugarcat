#ifndef LIBSTR_H_INCLUDED
#define LIBSTR_H_INCLUDED
#include <stddef.h>

size_t strlen(const char *);
char *strrev(char *);
char *itox(unsigned int, char *);
char *itoa(int, char *);
int atoi(const char *);
int strcmp(const char *, const char *);
#endif
