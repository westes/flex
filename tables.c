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

/** Calculate (0-7) = number bytes needed to pad n to next 64-bit boundary. */
#define yypad64(n) ((8-((n)%8))%8)

/** Extract corresponding data size_t from td_flags */
#define TFLAGS2BYTES(td_flags)\
        (((td_flags) & YYT_DATA8)\
            ? sizeof(int8_t)\
            :(((td_flags) & YYT_DATA16)\
                ? sizeof(int16_t)\
                :sizeof(int32_t)))

/** Convert size_t to t_flag.
 *  @param n in {1,2,4}
 *  @return YYT_DATA*. 
 */
#define BYTES2TFLAG(n)\
    (((n) == sizeof(int8_t))\
        ? YYT_DATA8\
        :(((n)== sizeof(int16_t))\
            ? YYT_DATA16\
            : YYT_DATA32))

/** Clear YYT_DATA* bit flags
 * @return the flag with the YYT_DATA* bits cleared
 */
#define TFLAGS_CLRDATA(flg) ((flg) & ~(YYT_DATA8 | YYT_DATA16 | YYT_DATA32))

int     yytbl_fwrite32 (FILE * out, uint32_t v);
int     yytbl_fwrite16 (FILE * out, uint16_t v);
int     yytbl_fwrite8 (FILE * out, uint8_t v);

/** Initialize a table header.
 *  @param th  The uninitialized structure
 *  @param version_str the  version string
 *  @param name the name of this table set
 */
void yytbl_hdr_init (struct yytbl_hdr *th, const char *version_str,
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

/** Allocate and initialize a table data structure.
 *  @param tbl a pointer to an uninitialized table
 *  @param id  the table identifier
 *  @return 0 on success
 */
int yytbl_data_init (struct yytbl_data *td, enum yytbl_id id)
{

	memset (td, 0, sizeof (struct yytbl_data));
	td->td_id = id;
	td->td_flags = YYT_DATA32;
	return 0;
}

/** write the header.
 *  @param out the output stream
 *  @param th table header to be written
 *  @return -1 on error, or bytes written on success.
 */
int yytbl_hdr_fwrite (FILE * out, const struct yytbl_hdr *th)
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

/** Write four bytes in network byte order
 *  @param  out  the output stream
 *  @param  v    a dword in host byte order
 *  @return  -1 on error. number of bytes written on success.
 */
int yytbl_fwrite32 (FILE * out, uint32_t v)
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

/** Write two bytes in network byte order.
 *  @param  out  the output stream
 *  @param  v    a word in host byte order
 *  @return  -1 on error. number of bytes written on success.
 */
int yytbl_fwrite16 (FILE * out, uint16_t v)
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

/** Write a byte.
 *  @param  out  the output stream
 *  @param  v    the value to be written
 *  @return  -1 on error. number of bytes written on success.
 */
int yytbl_fwrite8 (FILE * out, uint8_t v)
{
	size_t  bytes, rv;

	bytes = sizeof (uint8_t);
	rv = fwrite (&v, bytes, 1, out);
	if (rv != bytes)
		return -1;
	return bytes;
}

/** Get the number of integers in this table. This is NOT the
 *  same thing as the number of elements.
 *  @param td the table 
 *  @return the number of integers in the table
 */
static int32_t tbl_get_total_len (struct yytbl_data *tbl)
{

	int32_t n;

	/* total number of ints */
	n = tbl->td_lolen;
	if (tbl->td_hilen > 0)
		n *= tbl->td_hilen;

	if (tbl->td_id == YYT_ID_TRANSITION)
		n *= 2;
	return n;
}


/** Extract data element [i][j] from array data tables. 
 * @param tbl data table
 * @param i index into higher dimension array. i should be zero for one-dimensional arrays.
 * @param j index into lower dimension array.
 * @param k index into struct, must be 0 or 1. Only valid for YYT_ID_TRANSITION table
 * @return data[i][j + k]
 */
int32_t yytbl_data_getijk (const struct yytbl_data * tbl, int i, int j,
			   int k)
{
	int32_t lo;

	k %= 2;
	lo = tbl->td_lolen;

	switch (TFLAGS2BYTES (tbl->td_flags)) {
	case sizeof (int8_t):
		return ((int8_t *) (tbl->td_data))[(i * lo + j) * (k + 1) +
						   k];
	case sizeof (int16_t):
		return ((int16_t *) (tbl->td_data))[(i * lo + j) * (k +
								    1) +
						    k];
	case sizeof (int32_t):
		return ((int32_t *) (tbl->td_data))[(i * lo + j) * (k +
								    1) +
						    k];
	default:		/* TODO: error. major foobar somewhere. */
		break;
	}

	return 0;
}

/** Extract data element [i] from array data tables treated as a single flat array of integers.
 * Be careful for 2-dimensional arrays or for YYT_ID_TRANSITION, which is an array
 * of structs. 
 * @param tbl data table
 * @param i index into array.
 * @return data[i]
 */
static int32_t yytbl_data_geti (const struct yytbl_data *tbl, int i)
{

	switch (TFLAGS2BYTES (tbl->td_flags)) {
	case sizeof (int8_t):
		return ((int8_t *) (tbl->td_data))[i];
	case sizeof (int16_t):
		return ((int16_t *) (tbl->td_data))[i];
	case sizeof (int32_t):
		return ((int32_t *) (tbl->td_data))[i];
	default:		/* TODO: error. major foobar somewhere. */
		break;
	}
	return 0;
}

/** Set data element [i] in array data tables treated as a single flat array of integers.
 * Be careful for 2-dimensional arrays or for YYT_ID_TRANSITION, which is an array
 * of structs. 
 * @param tbl data table
 * @param i index into array.
 * @param newval new value for data[i]
 */
static void yytbl_data_seti (const struct yytbl_data *tbl, int i,
			     int newval)
{

	switch (TFLAGS2BYTES (tbl->td_flags)) {
	case sizeof (int8_t):
		((int8_t *) (tbl->td_data))[i] = (int8_t) newval;
	case sizeof (int16_t):
		((int16_t *) (tbl->td_data))[i] = (int16_t) newval;
	case sizeof (int32_t):
		((int32_t *) (tbl->td_data))[i] = (int32_t) newval;
	default:		/* TODO: error. major foobar somewhere. */
		break;
	}
}

/** Calculate the number of bytes  needed to hold the largest
 *  absolute value in this data array.
 *  @param tbl  the data table
 *  @return sizeof(n) where n in {int8_t, int16_t, int32_t}
 */
static size_t min_int_size (struct yytbl_data *tbl)
{
	uint32_t i, total_len;
	int32_t max = 0;

	total_len = tbl_get_total_len (tbl);

	for (i = 0; i < total_len; i++) {
		int32_t n;

		n = abs (yytbl_data_geti (tbl, i));

		if (n > max)
			max = n;
	}

	if (max <= INT8_MAX)
		return sizeof (int8_t);
	else if (max <= INT16_MAX)
		return sizeof (int16_t);
	else
		return sizeof (int32_t);
}

/** Transform data to smallest possible of (int32, int16, int8).
 * For example, we may have generated an int32 array due to user options
 * (e.g., %option align), but if the maximum value in that array
 * is 80 (for example), then we can serialize it with only 1 byte per int.
 * This is NOT the same as compressed DFA tables. We're just trying
 * to save storage space here.
 *
 * @param tbl the table to be compressed
 */
void yytbl_data_compress (struct yytbl_data *tbl)
{
	int32_t i, newsz, total_len;
	struct yytbl_data newtbl;

	yytbl_data_init (&newtbl, tbl->td_id);
	newtbl.td_hilen = tbl->td_hilen;
	newtbl.td_lolen = tbl->td_lolen;
	newtbl.td_flags = tbl->td_flags;

	newsz = min_int_size (tbl);



	if (newsz == TFLAGS2BYTES (tbl->td_flags))
		/* No change in this table needed. */
		return;

	if (newsz > TFLAGS2BYTES (tbl->td_flags)) {
		/* TODO: ERROR. The code is wrong somewhere. */
		return;
	}

	total_len = tbl_get_total_len (tbl);
	newtbl.td_data = flex_alloc (newsz * total_len);
	newtbl.td_flags =
		TFLAGS_CLRDATA (newtbl.td_flags) & BYTES2TFLAG (newsz);

	for (i = 0; i < total_len; i++)
		yytbl_data_seti (&newtbl, i, yytbl_data_geti (tbl, i));


	/* Now copy over the old table */
	free (tbl->td_data);
	*tbl = newtbl;
}

/* vim:set noexpandtab cindent tabstop=8 softtabstop=0 shiftwidth=8 textwidth=0: */
