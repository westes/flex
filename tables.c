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

int yytbl_hdr_write(FILE* fp, struct yytbl_hdr * th)
{
    uint32_t i32; /* temp variables. */
    uint16_t i16;
    size_t sz,rv;
    int pad,bwritten=0;

#define WRITEORERR(val,Bsz) \
    do {  i ##Bsz = htonl(val);\
          if ((rv=fwrite(&i ##Bsz, Bsz/8, 1, fp)) != (Bsz)/8)\
                return 1;\
          bwritten += rv;\
    }while(0)


    WRITEORERR(th->th_magic, 32);
    WRITEORERR(th->th_hsize, 32);
    WRITEORERR(th->th_ssize, 32);
    WRITEORERR(th->th_flags, 16);

    sz = strlen(th->th_version)+1;
    if ((rv=fwrite(th->th_version,1,sz,fp)) != sz)
        return 1;
    bwritten += rv;

    sz = strlen(th->th_name)+1;
    if ((rv=fwrite(th->th_name,1,sz,fp)) != sz)
        return 1;
    bwritten += rv;

    /* add padding */
    pad = yypad64(bwritten) - bwritten;
    while(pad-- > 0) {
        uint8_t c =0;
        if ((rv=fwrite(&c,sizeof(uint8_t),1,fp)) != 1)
            return 1;
        bwritten++;
    }

    /* Sanity check */
    if (bwritten != th->th_hsize){
        /* Oops. */
        return 1;
    }

    return 0;
}

/* vim:set expandtab cindent tabstop=4 softtabstop=4 shiftwidth=4 textwidth=0: */
