/*  tables.h - tables serialization code
 *
 *  Copyright (c) 1990 The Regents of the University of California.
 *  All rights reserved.
 *
 *  This code is derived from software contributed to Berkeley by
 *  Vern Paxson.
 *
 *  The United States Government has rights in this work pursuant
 *  to contract no. DE-AC03-76SF00098 between the United States
 *  Department of Energy and the University of California.
 *
 *  This file is part of flex.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  Neither the name of the University nor the names of its contributors
 *  may be used to endorse or promote products derived from this software
 *  without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE.
 */


#include "flexdef.h"

#define yypad64(n) ((8-((n)%8))%8)

int yytbl_fwrite32 (FILE *out, uint32_t v);
int yytbl_fwrite16 (FILE *out, uint16_t v);
int yytbl_fwrite8  (FILE *out, uint8_t  v);

void yytbl_hdr_init(struct yytbl_hdr *th, const char * version_str, const char *name)
{
    memset(th, 0, sizeof(struct yytbl_hdr));
    
    th->th_magic = 0xF13C57B1;
    th->th_hsize = yypad64(20 + strlen(version_str) + 1 + strlen(name)+ 1);
    th->th_ssize = 0; // Not known at this point.
    th->th_flags = 0;
    th->th_version = copy_string(version_str);
    th->th_name = copy_string(name);
}

int yytbl_hdr_fwrite(FILE* out, struct yytbl_hdr * th)
{
    size_t sz,rv;
    int pad,bwritten=0;

    if ( yytbl_fwrite32(out, th->th_magic) < 0
        || yytbl_fwrite32(out, th->th_hsize) < 0
        || yytbl_fwrite32(out, th->th_ssize) < 0
        || yytbl_fwrite16(out, th->th_flags) < 0)
        return -1;
    else
        bwritten += 3*4 + 2;

    sz = strlen(th->th_version)+1;
    if ((rv=fwrite(th->th_version,1,sz,out)) != sz)
        return -1;
    bwritten += rv;

    sz = strlen(th->th_name)+1;
    if ((rv=fwrite(th->th_name,1,sz,out)) != sz)
        return 1;
    bwritten += rv;

    /* add padding */
    pad = yypad64(bwritten) - bwritten;
    while(pad-- > 0)
        if (yytbl_fwrite8(out, 0) < 0)
            return -1;
        else
            bwritten++;

    /* Sanity check */
    if (bwritten != th->th_hsize){
        /* Oops. */
        return -1;
    }

    return bwritten;
}

int yytbl_fwrite32(FILE *out, uint32_t v)
{
    uint32_t vnet;
    size_t bytes, rv;
    
    vnet = htonl(v);
    bytes = sizeof(uint32_t);
    rv = fwrite(&vnet,bytes,1,out);
    if( rv != bytes)
        return -1;
    return bytes;
}

int yytbl_fwrite16(FILE *out, uint16_t v)
{
    uint16_t vnet;
    size_t bytes, rv;
    
    vnet = htons(v);
    bytes = sizeof(uint16_t);
    rv = fwrite(&vnet,bytes,1,out);
    if( rv != bytes)
        return -1;
    return bytes;
}

int yytbl_fwrite8(FILE *out, uint8_t v)
{
    size_t bytes, rv;
    
    bytes = sizeof(uint8_t);
    rv = fwrite(&v,bytes,1,out);
    if( rv != bytes)
        return -1;
    return bytes;
}


/* vim:set expandtab cindent tabstop=4 softtabstop=4 shiftwidth=4 textwidth=0: */
