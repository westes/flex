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

/* This typedef is only used for computing footprint sizes,
 * You need to make sure they match reality in the skeleton file to 
 * get accurate numbers, but they don't otherwise matter.
 * FIXME: This should go away when Flex ships only macros.
 */
struct yy_trans_info {int32_t yy_verify; int32_t yy_nxt;};

/* Helper functions */

static const char *cpp_get_int16_decl (void)
{
	return (gentables)
		? "static const flex_int16_t %s[%d] = {   0,\n"
		: "static const flex_int16_t * %s = 0;\n";
}


static const char *cpp_get_int32_decl (void)
{
	return (gentables)
		? "static const flex_int32_t %s[%d] = {   0,\n"
		: "static const flex_int32_t * %s = 0;\n";
}

static const char *cpp_get_state_decl (void)
{
	return (gentables)
		? "static const yy_state_type %s[%d] = {   0,\n"
		: "static const yy_state_type * %s = 0;\n";
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

static void cpp_ntod(size_t num_full_table_rows)
// Generate nxt table for ntod
{
	struct packtype_t *ptype = optimize_pack(0);
	out_str ("m4_define([[M4_HOOK_NXT_TYPE]], [[%s]])", ptype->name);
	buf_strappend (&yydmap_buf,
		    "\t{YYTD_ID_NXT, (void**)&yy_nxt, sizeof(M4_HOOK_NXT_TYPE)},\n");

	/* Unless -Ca, declare it "short" because it's a real
	 * long-shot that that won't be large enough.
	 */
	if (gentables)
		out_dec
		    ("static const M4_HOOK_NXT_TYPE yy_nxt[][%d] =\n    {\n", num_full_table_rows);
	else {
		out_dec ("#undef YY_NXT_LOLEN\n#define YY_NXT_LOLEN (%d)\n", num_full_table_rows);
		out ("static const M4_HOOK_NXT_TYPE *yy_nxt =0;\n");
	}
	/* It would be no good trying to return an allocation size here,
	 * as it's not known before table generation is finished.
	 */
}

static size_t cpp_gentabs_acclist(size_t sz)
// Generate accept list initializer
{
	struct packtype_t *ptype = optimize_pack(0);
	out_str ("m4_define([[M4_HOOK_ACCLIST_TYPE]], [[%s]])", ptype->name);
	out_str_dec (ctrl.long_align ? cpp_get_int32_decl () :
		     cpp_get_int16_decl (), "yy_acclist", sz);
	buf_strappend (&yydmap_buf,
		    "\t{YYTD_ID_ACCLIST, (void**)&yy_acclist, sizeof(M4_HOOK_ACCLIST_TYPE)},\n");
	return sz * ptype->width;
}

static size_t cpp_gentabs_accept(size_t sz)
// Generate accept table initializer
{
	/* FIXME: Could pack tighter by passing the size limit to optimize_pack()_ */
	struct packtype_t *ptype = optimize_pack(0);
	out_str ("m4_define([[M4_HOOK_ACCEPT_TYPE]], [[%s]])", ptype->name);
	out_str_dec (ctrl.long_align ? cpp_get_int32_decl () : cpp_get_int16_decl (),
		     "yy_accept", sz);
	buf_strappend (&yydmap_buf,
		       "\t{YYTD_ID_ACCEPT, (void**)&yy_accept, sizeof(M4_HOOK_ACCEPT_TYPE)},\n");
	return sz * ptype->width;
}

static size_t cpp_gentabs_yy_base(size_t sz)
// Generate yy_meta base initializer
{
	struct packtype_t *ptype = optimize_pack(sz);
	out_str ("m4_define([[M4_HOOK_BASE_TYPE]], [[%s]])", ptype->name);
	out_str_dec ((sz >= INT16_MAX || ctrl.long_align) ?
		     cpp_get_int32_decl () : cpp_get_int16_decl (),
		     "yy_base", sz);
	buf_strappend (&yydmap_buf,
		    "\t{YYTD_ID_BASE, (void**)&yy_base, sizeof(M4_HOOK_BASE_TYPE)},\n");
	return sz * ptype->width;
}

static size_t cpp_gentabs_yy_def(size_t sz)
// Generate yy_def initializer
{
	struct packtype_t *ptype = optimize_pack(sz);
	out_str ("m4_define([[M4_HOOK_DEF_BASE_TYPE]], [[%s]])", ptype->name);
	out_str_dec ((sz >= INT16_MAX || ctrl.long_align) ?
		     cpp_get_int32_decl () : cpp_get_int16_decl (),
		     "yy_def", sz);
	buf_strappend (&yydmap_buf,
		    "\t{YYTD_ID_DEF, (void**)&yy_def, sizeof(M4_HOOK_DEF_BASE_TYPE)},\n");
	return sz * ptype->width;
}

static size_t cpp_gentabs_yy_nxt(size_t tblafter)
// Generate yy_nxt initializer
{
	struct packtype_t *ptype = optimize_pack(tblafter);
	out_str ("m4_define([[M4_HOOK_NXT_BASE_TYPE]], [[%s]])", ptype->name);
	out_str_dec ((tblafter >= INT16_MAX || ctrl.long_align) ?
		     cpp_get_int32_decl () : cpp_get_int16_decl (), "yy_nxt",
		     tblafter);
	buf_strappend (&yydmap_buf,
		    "\t{YYTD_ID_NXT, (void**)&yy_nxt, sizeof(M4_HOOK_NXT_BASE_TYPE)},\n");
	return tblafter * ptype->width;
}

static size_t cpp_gentabs_yy_chk(size_t tblafter)
// Generate yy_chk initializer
{
	struct packtype_t *ptype = optimize_pack(tblafter);
	out_str ("m4_define([[M4_HOOK_IDCHK_BASE_TYPE]], [[%s]])", ptype->name);
	out_str_dec ((tblafter >= INT16_MAX || ctrl.long_align) ?
		     cpp_get_int32_decl () : cpp_get_int16_decl (), "yy_chk",
		     tblafter);
	buf_strappend (&yydmap_buf,
		    "\t{YYTD_ID_CHK, (void**)&yy_chk, sizeof(M4_HOOK_IDCHK_BASE_TYPE)},\n");
	return tblafter * ptype->width;
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

const char *cpp_skel[] = {
#include "cpp-skel.h"
    0,
};

/* This backend is only accessed through this method table */
struct flex_backend_t cpp_backend = {
	.suffix = cpp_suffix,
	.skel = cpp_skel,
	.ntod = cpp_ntod,
	.gentabs_acclist = cpp_gentabs_acclist,
	.gentabs_accept = cpp_gentabs_accept,
	.gentabs_yy_base = cpp_gentabs_yy_base,
	.gentabs_yy_def = cpp_gentabs_yy_def,
	.gentabs_yy_nxt = cpp_gentabs_yy_nxt,
	.gentabs_yy_chk = cpp_gentabs_yy_chk,
	.nultrans = cpp_nultrans,
	.c_like = true,
};
