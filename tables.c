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
#define TFLAGS2BYTES(flags)\
        (((flags) & YYT_DATA8)\
            ? sizeof(int8_t)\
            :(((flags) & YYT_DATA16)\
                ? sizeof(int16_t)\
                :sizeof(int32_t)))

int     yytbl_fwrite32 (FILE * out, uint32_t v);
int     yytbl_fwrite16 (FILE * out, uint16_t v);
int     yytbl_fwrite8 (FILE * out, uint8_t v);

void    yytbl_hdr_init (struct yytbl_hdr *th, const char *version_str,
			const char *name)
{
	memset (th, 0, sizeof (struct yytbl_hdr));

	th->th_magic = 0xF13C57B1;
	th->th_hsize =
		yypad64 (20 + strlen (version_str) + 1 + strlen (name) +
			 1);
	th->th_ssize = 0;	// Not known at this point.
	th->th_flags = 0;
	th->th_version = copy_string (version_str);
	th->th_name = copy_string (name);
}

struct yytbl_data *yytbl_data_create (enum yytbl_id id)
{
	struct yytbl_data *td;

	td = (struct yytbl_data *) flex_alloc (sizeof (struct yytbl_data));
	memset (td, 0, sizeof (struct yytbl_data));

	td->t_id = id;
	return td;
}

int     yytbl_hdr_fwrite (FILE * out, struct yytbl_hdr *th)
{
	size_t  sz, rv;
	int     pad, bwritten = 0;

	if (yytbl_fwrite32 (out, th->th_magic) < 0
	    || yytbl_fwrite32 (out, th->th_hsize) < 0
	    || yytbl_fwrite32 (out, th->th_ssize) < 0
	    || yytbl_fwrite16 (out, th->th_flags) < 0)
		return -1;
	else
		bwritten += 3 * 4 + 2;

	sz = strlen (th->th_version) + 1;
	if ((rv = fwrite (th->th_version, 1, sz, out)) != sz)
		return -1;
	bwritten += rv;

	sz = strlen (th->th_name) + 1;
	if ((rv = fwrite (th->th_name, 1, sz, out)) != sz)
		return 1;
	bwritten += rv;

	/* add padding */
	pad = yypad64 (bwritten) - bwritten;
	while (pad-- > 0)
		if (yytbl_fwrite8 (out, 0) < 0)
			return -1;
		else
			bwritten++;

	/* Sanity check */
	if (bwritten != th->th_hsize) {
		/* Oops. */
		return -1;
	}

	return bwritten;
}

int     yytbl_fwrite32 (FILE * out, uint32_t v)
{
	uint32_t vnet;
	size_t  bytes, rv;

	vnet = htonl (v);
	bytes = sizeof (uint32_t);
	rv = fwrite (&vnet, bytes, 1, out);
	if (rv != bytes)
		return -1;
	return bytes;
}

int     yytbl_fwrite16 (FILE * out, uint16_t v)
{
	uint16_t vnet;
	size_t  bytes, rv;

	vnet = htons (v);
	bytes = sizeof (uint16_t);
	rv = fwrite (&vnet, bytes, 1, out);
	if (rv != bytes)
		return -1;
	return bytes;
}

int     yytbl_fwrite8 (FILE * out, uint8_t v)
{
	size_t  bytes, rv;

	bytes = sizeof (uint8_t);
	rv = fwrite (&v, bytes, 1, out);
	if (rv != bytes)
		return -1;
	return bytes;
}

/* calculate the number of bytes (1,2,4) needed to hold the largest absolute value
 * in this array. */
static int min_int_size (void *arr, int32_t len, int sz)
{
	int32_t curr, max = 0, i;

	for (i = 0; i < len; i++) {
		switch (sz) {
		case 1:
			curr = abs (((int8_t *) arr)[i]);
			break;
		case 2:
			curr = abs (((int16_t *) arr)[i]);
			break;
		case 4:
			curr = abs (((int32_t *) arr)[i]);
			break;
		default:
			fprintf (stderr,
				 "Illegal size (%d) in min_int_size\n",
				 sz);
			return 32;
		}
		if (curr > max)
			max = curr;
	}
	if (max < INT8_MAX)
		return sizeof (int8_t);
	else if (max < INT16_MAX)
		return sizeof (int16_t);
	else
		return sizeof (int32_t);
}


/* extract data element [i][j] from int array data tables. */
static int32_t yytbl_data_geti (const struct yytbl_data *tbl, int i, int j)
{
	/* TODO */
	return 0;
}

/* Transform data to smallest possible of (int32, int16, int8) */
void    yytbl_data_compress (struct yytbl_data *tbl)
{
	int32_t i, sz;
	void   *newdata = 0;

	if (tbl->t_id != YYT_ID_TRANSITION
	    && tbl->t_id != YYT_ID_START_STATE_LIST) {
		if (tbl->t_hilen == 0) {
			/* Data is a single-dimensional array of ints */
			sz =
				min_int_size (tbl->t_data, tbl->t_lolen,
					      TFLAGS2BYTES (tbl->t_flags));
			if (sz == TFLAGS2BYTES (tbl->t_flags))
				/* No change in this table needed. */
				return;

			if (sz > TFLAGS2BYTES (tbl->t_flags)) {
				/* TODO: ERROR. The code is wrong somewhere. */
				return;
			}

			newdata = flex_alloc (sz * tbl->t_lolen);
			for (i = 0; i < tbl->t_lolen; i++) {
				int32_t n;

				n = yytbl_data_geti (tbl, 0, i);
				switch (sz) {
				case sizeof (int8_t):
					((int8_t *) newdata)[i] = (int8_t) n;
					break;
				case sizeof (int16_t):
					
						((int16_t *) newdata)[i] =
						(int16_t) n;
					break;
				case sizeof (int32_t):
					
						((int32_t *) newdata)[i] =
						(int32_t) n;
					break;
				default:	/* TODO: ERROR: unknown 'sz' */
					break;
				}
			}
			free (tbl->t_data);
			tbl->t_data = newdata;

		}
		else {
			/* Data is a two-dimensional array of ints */
		}
	}
	else if (tbl->t_id == YYT_ID_TRANSITION) {
		/* Data is an array of structs */
	}
	else if (tbl->t_id == YYT_ID_START_STATE_LIST) {
		/* Data is an array of pointers */
	}
}

/* vim:set expandtab cindent tabstop=4 softtabstop=4 shiftwidth=4 textwidth=0: */
