/* flex - tool to generate fast lexical analyzers */

/*  Copyright (c) 1990 The Regents of the University of California. */
/*  All rights reserved. */

/*  This code is derived from software contributed to Berkeley by */
/*  Vern Paxson. */

/*  The United States Government has rights in this work pursuant */
/*  to contract no. DE-AC03-76SF00098 between the United States */
/*  Department of Energy and the University of California. */

/*  This file is part of flex. */

/*  Redistribution and use in source and binary forms, with or without */
/*  modification, are permitted provided that the following conditions */
/*  are met: */

/*  1. Redistributions of source code must retain the above copyright */
/*     notice, this list of conditions and the following disclaimer. */
/*  2. Redistributions in binary form must reproduce the above copyright */
/*     notice, this list of conditions and the following disclaimer in the */
/*     documentation and/or other materials provided with the distribution. */

/*  Neither the name of the University nor the names of its contributors */
/*  may be used to endorse or promote products derived from this software */
/*  without specific prior written permission. */

/*  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR */
/*  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED */
/*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR */
/*  PURPOSE. */


#include "flexdef.h"
#include "tables.h"

/* These typedefs are only used for computing footprint sizes,
 * You need to make sure they match reality in the skeleton file to 
 * get accurate numbers, but they don't otherwise matter.
 * FIXME" This shiould go away when tFkex ships only macros.
 */
typedef char YY_CHAR;
struct yy_trans_info {int32_t yy_verify; int32_t yy_nxt;};

/* Helper fubctions */

static const char *cpp_get_int16_decl (void)
{
	return (gentables)
		? "static const flex_int16_t %s[%d] =\n    {   0,\n"
		: "static const flex_int16_t * %s = 0;\n";
}


static const char *cpp_get_int32_decl (void)
{
	return (gentables)
		? "static const flex_int32_t %s[%d] =\n    {   0,\n"
		: "static const flex_int32_t * %s = 0;\n";
}

static const char *cpp_get_state_decl (void)
{
	return (gentables)
		? "static const yy_state_type %s[%d] =\n    {   0,\n"
		: "static const yy_state_type * %s = 0;\n";
}

static const char *cpp_get_yy_char_decl (void)
{
	return (gentables)
		? "static const YY_CHAR %s[%d] =\n    {   0,\n"
		: "static const YY_CHAR * %s = 0;\n";
}

/* Methods */

static const char *cpp_suffix (void)
{
	char   *suffix;

	if (ctrl.C_plus_plus)
	    suffix = "cc";
	else
	    suffix = "c";

	return suffix;
}

static void cpp_comment(const char *txt)
{
	char buf[MAXLINE];
	bool eol;

	strncpy(buf, txt, MAXLINE-1);
	eol = buf[strlen(buf)-1] == '\n';

	if (eol)
		buf[strlen(buf)-1] = '\0';
	out_str("/* [[%s]] */", buf);
	if (eol)
		outc ('\n');
}

static void cpp_ntod(size_t num_full_table_rows)
// Generate nxt table for ntod
{
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_NXT, (void**)&yy_nxt, sizeof(%s)},\n",
		    ctrl.long_align ? "flex_int32_t" : "flex_int16_t");

	/* Unless -Ca, declare it "short" because it's a real
	 * long-shot that that won't be large enough.
	 */
	if (gentables)
		out_str_dec
			("static const %s yy_nxt[][%d] =\n    {\n",
			 ctrl.long_align ? "flex_int32_t" : "flex_int16_t",
			 num_full_table_rows);
	else {
		out_dec ("#undef YY_NXT_LOLEN\n#define YY_NXT_LOLEN (%d)\n", num_full_table_rows);
		out_str ("static const %s *yy_nxt =0;\n",
			 ctrl.long_align ? "flex_int32_t" : "flex_int16_t");
	}
	/* It would be no good trying to return an allocation size here,
	 * as it's not known before table generation is finished.
	 */
}

static void cpp_mkctbl (size_t sz)
// Make full-speed compressed transition table
{
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_TRANSITION, (void**)&yy_transition, sizeof(%s)},\n",
		    (sz >= INT16_MAX
		     || ctrl.long_align) ? "flex_int32_t" : "flex_int16_t");
}

static size_t cpp_gen_yy_trans(size_t sz)
// Table of verify for transition and offset to next state. (sic)
{
	if (gentables)
		out_dec ("static const struct yy_trans_info yy_transition[%d] =\n    {\n", sz);
	else
		outn ("static const struct yy_trans_info *yy_transition = 0;");
	return sz * sizeof(struct yy_trans_info);
}

static size_t cpp_start_state_list(size_t sz)
// Start initializer for table of pointers to start state
{
	/* Table of pointers to start states. */
	if (gentables)
		out_dec ("static const struct yy_trans_info *yy_start_state_list[%d] =\n", sz);
	else
		outn ("static const struct yy_trans_info **yy_start_state_list =0;");
	return sz * sizeof(struct yy_trans_info *);
}

static void cpp_mkftbl(void)
// Make full table
{
	// FIXME: why are there two places this is emitted, here and in cpp_gentabs_accept()?
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_ACCEPT, (void**)&yy_accept, sizeof(%s)},\n",
		    ctrl.long_align ? "flex_int32_t" : "flex_int16_t");
}

static size_t cpp_gentabs_acclist(size_t sz)
// Generate accept list initializer
{
	out_str_dec (ctrl.long_align ? cpp_get_int32_decl () :
		     cpp_get_int16_decl (), "yy_acclist", sz);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_ACCLIST, (void**)&yy_acclist, sizeof(%s)},\n",
		    ctrl.long_align ? "flex_int32_t" : "flex_int16_t");
	return sz * (ctrl.long_align ? sizeof(int32_t) : sizeof(int16_t));
}

static size_t cpp_gentabs_accept(size_t sz)
// Generate accept table initializer
{
	out_str_dec (ctrl.long_align ? cpp_get_int32_decl () : cpp_get_int16_decl (),
		     "yy_accept", sz);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_ACCEPT, (void**)&yy_accept, sizeof(%s)},\n",
		    ctrl.long_align ? "flex_int32_t" : "flex_int16_t");
	return sz * (ctrl.long_align ? sizeof(int32_t) : sizeof(int16_t));
}

static size_t cpp_gentabs_yy_meta(size_t sz)
// Generate yy_meta table initializer
{
	out_str_dec (cpp_get_yy_char_decl (), "yy_meta", sz);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_META, (void**)&yy_meta, sizeof(%s)},\n",
		    "YY_CHAR");
	return sz * sizeof(YY_CHAR);
}

static size_t cpp_gentabs_yy_base(size_t sz)
// Generate yy_meta base initializer
{
	out_str_dec ((tblend >= INT16_MAX || ctrl.long_align) ?
		     cpp_get_int32_decl () : cpp_get_int16_decl (),
		     "yy_base", sz);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_BASE, (void**)&yy_base, sizeof(%s)},\n",
		    (sz >= INT16_MAX
		     || ctrl.long_align) ? "flex_int32_t" : "flex_int16_t");
	return sz * ((sz >= INT16_MAX || ctrl.long_align) ? sizeof(int32_t) : sizeof(int16_t)); 
}

static size_t cpp_gentabs_yy_def(size_t sz)
// Generate yy_def initializer
{
	out_str_dec ((sz >= INT16_MAX || ctrl.long_align) ?
		     cpp_get_int32_decl () : cpp_get_int16_decl (),
		     "yy_def", sz);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_DEF, (void**)&yy_def, sizeof(%s)},\n",
		    (sz >= INT16_MAX
		     || ctrl.long_align) ? "flex_int32_t" : "flex_int16_t");
	return sz * ((sz >= INT16_MAX || ctrl.long_align) ? sizeof(int32_t) : sizeof(int16_t));
}

static size_t cpp_gentabs_yy_nxt(size_t tblafter)
// Generate yy_nxt initializer
{
	/* Begin generating yy_nxt */
	out_str_dec ((tblafter >= INT16_MAX || ctrl.long_align) ?
		     cpp_get_int32_decl () : cpp_get_int16_decl (), "yy_nxt",
		     tblafter);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_NXT, (void**)&yy_nxt, sizeof(%s)},\n",
		    (tblafter >= INT16_MAX
		     || ctrl.long_align) ? "flex_int32_t" : "flex_int16_t");
	return tblafter * ((tblafter >= INT16_MAX || ctrl.long_align) ? sizeof(int32_t) : sizeof(int16_t));
}

static size_t cpp_gentabs_yy_chk(size_t tblafter)
// Generate yy_chk initializer
{
	out_str_dec ((tblafter >= INT16_MAX || ctrl.long_align) ?
		     cpp_get_int32_decl () : cpp_get_int16_decl (), "yy_chk",
		     tblafter);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_CHK, (void**)&yy_chk, sizeof(%s)},\n",
		    (tblafter >= INT16_MAX
		     || ctrl.long_align) ? "flex_int32_t" : "flex_int16_t");
	return tblafter * ((tblafter >= INT16_MAX || ctrl.long_align) ? sizeof(int32_t) : sizeof(int16_t));
}

static size_t cpp_nultrans(int fullspd, size_t afterdfa)
// Generate nulltrans initializer
{
	// Making this a backend method may be overzealous.
	// How many other languages have to special-case NUL
	// because it's a string terminator?
	out_str_dec (cpp_get_state_decl (), "yy_NUL_trans", afterdfa);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_NUL_TRANS, (void**)&yy_NUL_trans, sizeof(%s)},\n",
		    (fullspd) ? "struct yy_trans_info*" : "flex_int32_t");
	return afterdfa * (fullspd ? sizeof(struct yy_trans_info *) : sizeof(int32_t));
}

static const char *cpp_trans_offset_type(int total_table_size)
{
	return (total_table_size >= INT16_MAX || ctrl.long_align) ?
			"flex_int32_t" : "flex_int16_t";
}

const char *cpp_skel[] = {
#include "cpp-skel.h"
    0,
};

/* This backend is only accessed through this method table */
struct flex_backend_t cpp_backend = {
	.suffix = cpp_suffix,
	.skel = cpp_skel,
	.comment = cpp_comment,
	.ntod = cpp_ntod,
	.mkctbl = cpp_mkctbl,
	.gen_yy_trans = cpp_gen_yy_trans,
	.start_state_list = cpp_start_state_list,
	.mkftbl = cpp_mkftbl,
	.gentabs_acclist = cpp_gentabs_acclist,
	.gentabs_accept = cpp_gentabs_accept,
	.gentabs_yy_meta = cpp_gentabs_yy_meta,
	.gentabs_yy_base = cpp_gentabs_yy_base,
	.gentabs_yy_def = cpp_gentabs_yy_def,
	.gentabs_yy_nxt = cpp_gentabs_yy_nxt,
	.gentabs_yy_chk = cpp_gentabs_yy_chk,
	.nultrans = cpp_nultrans,
	.trans_offset_type = cpp_trans_offset_type,
	.c_like = true,
};
