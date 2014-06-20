/* This file is part of flex.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the University nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE
 */

/*
 * Those are utility functions that are reimplementations of some standard
 * C functions for strings manipulation, but using YY_CHAR macro instead of
 * raw char type.
 *
 * These functions are only used in some test suites, not in the flex itself.
 */

#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "strutils.h"

size_t yy_utils_strlen(YY_CHAR* str)
{
    YY_CHAR *ptr = str;
    while(*ptr != 0) ptr++;
    return ptr-str;
}

YY_CHAR* yy_utils_strcpy(YY_CHAR *destination, const YY_CHAR *source)
{
    YY_CHAR *ptr = destination;
    while((*destination++ = *source++));
    return ptr;
}

int yy_utils_strcmp(YY_CHAR *str1, YY_CHAR *str2)
{
    while(*str1 == *str2++)
        if(*str1++ == 0)
            return 0;
    return *str1 - *(str2-1);
}

YY_CHAR* yy_utils_strdup(YY_CHAR* s1)
{
    YY_CHAR* s2 = (YY_CHAR*)malloc((yy_utils_strlen(s1)+1)*sizeof(YY_CHAR));
    yy_utils_strcpy(s2, s1);
    return s2;
}

bool yy_utils_isany(YY_CHAR ch, const YY_CHAR *set, size_t set_size) {
    size_t i;
    for(i = 0; i < set_size; ++i)
        if(set[i] == ch)
            return true;
    return false;
}

bool yy_utils_isspace(YY_CHAR ch) {
    static const YY_CHAR set[] = {' ', '\r', '\n', '\t'};
    return yy_utils_isany(ch, set, sizeof(set)/sizeof(set[0]));
}

bool yy_utils_isupper(YY_CHAR ch) {
    static const YY_CHAR set[] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
    return yy_utils_isany(ch, set, sizeof(set)/sizeof(set[0]));
}

bool yy_utils_islower(YY_CHAR ch) {
    static const YY_CHAR set[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
    return yy_utils_isany(ch, set, sizeof(set)/sizeof(set[0]));
}

bool yy_utils_isdigit(YY_CHAR ch) {
    static const YY_CHAR set[] = {'0','1','2','3','4','5','6','7','8','9'};
    return yy_utils_isany(ch, set, sizeof(set)/sizeof(set[0]));
}

long int yy_utils_strtol(const YY_CHAR *str, YY_CHAR **endptr, int base)
{
    while(yy_utils_isspace(*str)) str++;
    bool neg = false;
    if(*str == '-') neg = true, str++;
    else if(*str == '+') str++;
    YY_CHAR ch;
    long int n = 0;
    while((ch = *str++)) {
        long int v = -1;
        if(yy_utils_isdigit(ch))
            v = ch-'0';
        else if(yy_utils_isupper(ch))
            v = ch-'A'+10;
        else if(yy_utils_islower(ch))
            v = ch-'a'+10;
        else
            break;
        if(v >= base)
            break;
        n *= base;
        n += v;
    }
    if(endptr)
        *endptr = (YY_CHAR*)str-1;
    if(neg)
        n = -n;
    return n;
}