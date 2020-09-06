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
#include "version.h"

/* Code specific to the C/C++ back end starts here */

static const char *cpp_suffix (void)
{
	char   *suffix;

	if (C_plus_plus)
	    suffix = "cc";
	else
	    suffix = "c";

	return suffix;
}

/* cpp_prolog - make rules prolog specific to cpp-using languages.
 *
 * If you don't ship this, you will effectively be assuming that your
 * parsers are always reentrant, always allow reject, always have a
 * yywrap() method, have a debug member in the wrapper class, and are
 * interactive.  This eliminates most of the boilerplate in the C/C++
 * scanner prolog.  It means such parsers will be a bit larger and
 * slower than C/C++ ones, but since we're not running on 1987's
 * hardware we officially do not care.
 *
 * A detail to beware of: If you're not issuing this prologue, you may
 * want to write your own definition of YY_CHAR in your skel
 * file. It's a typedef to an unsigned octet in C/C++, but if your
 * target language has a Unicode code-point type like Go's 'rune' is
 * may be appropriate.
 */

static void cpp_prolog (void)
{
	static char yy_stdinit[] = "FILE *yyin = stdin, *yyout = stdout;";
	static char yy_nostdinit[] =
		"FILE *yyin = NULL, *yyout = NULL;";

	if (reject){
        out_m4_define( "M4_YY_USES_REJECT", NULL);
		//outn ("\n#define YY_USES_REJECT");
	}

	if (!do_yywrap) {
		if (!C_plus_plus) {
			 if (reentrant)
				out_str ("\n#define %swrap(yyscanner) (/*CONSTCOND*/1)\n", prefix);
			 else
				out_str ("\n#define %swrap() (/*CONSTCOND*/1)\n", prefix);
		}
		outn ("#define YY_SKIP_YYWRAP");
	}

	if (ddebug)
		outn ("\n#define FLEX_DEBUG");

	OUT_BEGIN_CODE ();
	outn ("typedef flex_uint8_t YY_CHAR;");
	OUT_END_CODE ();

	if (C_plus_plus) {
		outn ("#define yytext_ptr yytext");

		if (interactive)
			outn ("#define YY_INTERACTIVE");
	}

	else {
		OUT_BEGIN_CODE ();
		/* In reentrant scanner, stdinit is handled in flex.skl. */
		if (do_stdinit) {
			if (reentrant){
                outn ("#ifdef VMS");
                outn ("#ifdef __VMS_POSIX");
                outn ("#define YY_STDINIT");
                outn ("#endif");
                outn ("#else");
                outn ("#define YY_STDINIT");
                outn ("#endif");
            }

			outn ("#ifdef VMS");
			outn ("#ifndef __VMS_POSIX");
			outn (yy_nostdinit);
			outn ("#else");
			outn (yy_stdinit);
			outn ("#endif");
			outn ("#else");
			outn (yy_stdinit);
			outn ("#endif");
		}

		else {
			if(!reentrant)
				outn (yy_nostdinit);
		}
		OUT_END_CODE ();
	}

	OUT_BEGIN_CODE ();
	if (fullspd)
		outn ("typedef const struct yy_trans_info *yy_state_type;");
	else if (!C_plus_plus)
		outn ("typedef int yy_state_type;");
	OUT_END_CODE ();

	if (lex_compat)
		outn ("#define YY_FLEX_LEX_COMPAT");

	if (!C_plus_plus && !reentrant) {
		outn ("extern int yylineno;");
		OUT_BEGIN_CODE ();
		outn ("int yylineno = 1;");
		OUT_END_CODE ();
	}

	if (C_plus_plus) {
		outn ("\n#include <FlexLexer.h>");

		if (!do_yywrap) {
			outn("\nint yyFlexLexer::yywrap() { return 1; }");
		}

		if (yyclass) {
			outn ("int yyFlexLexer::yylex()");
			outn ("\t{");
			outn ("\tLexerError( \"yyFlexLexer::yylex invoked but %option yyclass used\" );");
			outn ("\treturn 0;");
			outn ("\t}");

			out_str ("\n#define YY_DECL int %s::yylex()\n",
				 yyclass);
		}
	}

	else {

		/* Watch out: yytext_ptr is a variable when yytext is an array,
		 * but it's a macro when yytext is a pointer.
		 */
		if (yytext_is_array) {
			if (!reentrant)
				outn ("extern char yytext[];\n");
		}
		else {
			if (reentrant) {
				outn ("#define yytext_ptr yytext_r");
			}
			else {
				outn ("extern char *yytext;");

				outn("#ifdef yytext_ptr");
				outn("#undef yytext_ptr");
				outn("#endif");
				outn ("#define yytext_ptr yytext");
			}
		}

		if (yyclass)
			flexerror (_
				   ("%option yyclass only meaningful for C++ scanners"));
	}
}

static void cpp_epilog (void)
{
#if 0
	fprintf (header_out,
		 "#ifdef YY_HEADER_EXPORT_START_CONDITIONS\n");
	fprintf (header_out,
		 "/* Beware! Start conditions are not prefixed. */\n");

	/* Special case for "INITIAL" */
	fprintf (header_out,
		 "#undef INITIAL\n#define INITIAL 0\n");
	for (i = 2; i <= lastsc; i++)
		fprintf (header_out, "#define %s %d\n", scname[i], i - 1);
	fprintf (header_out,
		 "#endif /* YY_HEADER_EXPORT_START_CONDITIONS */\n\n");

	/* Kill ALL flex-related macros. This is so the user
	 * can #include more than one generated header file. */
	fprintf (header_out, "#ifndef YY_HEADER_NO_UNDEFS\n");
	fprintf (header_out,
		 "/* Undefine all internal macros, etc., that do no belong in the header. */\n\n");

        {
		const char * undef_list[] = {

                "BEGIN",
                "ECHO",
                "EOB_ACT_CONTINUE_SCAN",
                "EOB_ACT_END_OF_FILE",
                "EOB_ACT_LAST_MATCH",
                "FLEX_SCANNER",
                "REJECT",
                "YYFARGS0",
                "YYFARGS1",
                "YYFARGS2",
                "YYFARGS3",
                "YYLMAX",
                "YYSTATE",
                "YY_AT_BOL",
                "YY_BREAK",
                "YY_BUFFER_EOF_PENDING",
                "YY_BUFFER_NEW",
                "YY_BUFFER_NORMAL",
                "YY_BUF_SIZE",
                "M4_YY_CALL_LAST_ARG",
                "M4_YY_CALL_ONLY_ARG",
                "YY_CURRENT_BUFFER",
                "YY_DECL",
                "M4_YY_DECL_LAST_ARG",
                "M4_YY_DEF_LAST_ARG",
                "M4_YY_DEF_ONLY_ARG",
                "YY_DO_BEFORE_ACTION",
                "YY_END_OF_BUFFER",
                "YY_END_OF_BUFFER_CHAR",
                "YY_EXIT_FAILURE",
                "YY_EXTRA_TYPE",
                "YY_FATAL_ERROR",
                "YY_FLEX_DEFINED_ECHO",
                "YY_FLEX_LEX_COMPAT",
                "YY_FLEX_MAJOR_VERSION",
                "YY_FLEX_MINOR_VERSION",
                "YY_FLEX_SUBMINOR_VERSION",
                "YY_FLUSH_BUFFER",
                "YY_G",
                "YY_INPUT",
                "YY_INTERACTIVE",
                "YY_INT_ALIGNED",
                "YY_LAST_ARG",
                "YY_LESS_LINENO",
                "YY_LEX_ARGS",
                "YY_LEX_DECLARATION",
                "YY_LEX_PROTO",
                "YY_MAIN",
                "YY_MORE_ADJ",
                "YY_NEED_STRLEN",
                "YY_NEW_FILE",
                "YY_NULL",
                "YY_NUM_RULES",
                "YY_ONLY_ARG",
                "YY_PARAMS",
                "YY_PROTO",
                "M4_YY_PROTO_LAST_ARG",
                "M4_YY_PROTO_ONLY_ARG void",
                "YY_READ_BUF_SIZE",
                "YY_REENTRANT",
                "YY_RESTORE_YY_MORE_OFFSET",
                "YY_RULE_SETUP",
                "YY_SC_TO_UI",
                "YY_SKIP_YYWRAP",
                "YY_START",
                "YY_START_STACK_INCR",
                "YY_STATE_EOF",
                "YY_STDINIT",
                "YY_TRAILING_HEAD_MASK",
                "YY_TRAILING_MASK",
                "YY_USER_ACTION",
                "YY_USE_CONST",
                "YY_USE_PROTOS",
                "unput",
                "yyTABLES_NAME",
                "yy_create_buffer",
                "yy_delete_buffer",
                "yy_flex_debug",
                "yy_flush_buffer",
                "yy_init_buffer",
                "yy_load_buffer_state",
                "yy_new_buffer",
                "yy_scan_buffer",
                "yy_scan_bytes",
                "yy_scan_string",
                "yy_set_bol",
                "yy_set_interactive",
                "yy_switch_to_buffer",
				"yypush_buffer_state",
				"yypop_buffer_state",
				"yyensure_buffer_stack",
                "yyalloc",
                "const",
                "yyextra",
                "yyfree",
                "yyget_debug",
                "yyget_extra",
                "yyget_in",
                "yyget_leng",
                "yyget_column",
                "yyget_lineno",
                "yyget_lloc",
                "yyget_lval",
                "yyget_out",
                "yyget_text",
                "yyin",
                "yyleng",
                "yyless",
                "yylex",
                "yylex_destroy",
                "yylex_init",
                "yylex_init_extra",
                "yylineno",
                "yylloc",
                "yylval",
                "yymore",
                "yyout",
                "yyrealloc",
                "yyrestart",
                "yyset_debug",
                "yyset_extra",
                "yyset_in",
                "yyset_column",
                "yyset_lineno",
                "yyset_lloc",
                "yyset_lval",
                "yyset_out",
                "yytables_destroy",
                "yytables_fload",
                "yyterminate",
                "yytext",
                "yytext_ptr",
                "yywrap",

                /* must be null-terminated */
                NULL};


                for (i=0; undef_list[i] != NULL; i++)
                    fprintf (header_out, "#undef %s\n", undef_list[i]);
        }

	/* undef any of the auto-generated symbols. */
	for (i = 0; i < defs_buf.nelts; i++) {

		/* don't undef start conditions */
		if (sclookup (((char **) defs_buf.elts)[i]) > 0)
			continue;
		fprintf (header_out, "#undef %s\n",
			 ((char **) defs_buf.elts)[i]);
	}

	fprintf (header_out,
		 "#endif /* !YY_HEADER_NO_UNDEFS */\n");
	fprintf (header_out, "\n");
	fprintf (header_out, "#undef %sIN_HEADER\n", prefix);
	fprintf (header_out, "#endif /* %sHEADER_H */\n", prefix);

	if (ferror (header_out))
		lerr (_("error creating header file %s"),
			headerfilename);
	fflush (header_out);
	fclose (header_out);
#endif
}

static void cpp_linecomment(char *text)
{
    out("/* ");
    out(text);
    outn(" */");
}

static void cpp_assign(char *left, char *right)
{
    do_indent ();
    out(left);
    out(" = ");
    out(right);
    outn(";");
}

static void cpp_cond(char *guard)
{
    do_indent ();
    out("if (");
    out(guard);
    outn(" ) {");
}

static void cpp_statement(char *text)
{
    do_indent ();
    out(text);
    outn(";");
}

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

static void cpp_mkeoltbl()
// Make end-of-line-table - only used when yylinemo tracking is on
{
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_RULE_CAN_MATCH_EOL, (void**)&yy_rule_can_match_eol, sizeof(%s)},\n",
		    "flex_int32_t");
}

static void cpp_geneoltbl(size_t sz)
// Generate end-of-line-transitions - only used when yylineno tracking is on
{
	outn ("/* Table of booleans, true if rule could match eol. */");
	out_str_dec (backend->get_int32_decl (), "yy_rule_can_match_eol", sz);
}

static void cpp_gen_backing_up (void)
// Generate code to keep backup information.
{
	if (fullspd)
		indent_puts ("if ( yy_current_state[-1].yy_nxt )");
	else
		indent_puts ("if ( yy_accept[yy_current_state] )");

	++indent_level;
	indent_puts ("{");
	indent_puts ("YY_G(yy_last_accepting_state) = yy_current_state;");
	indent_puts ("YY_G(yy_last_accepting_cpos) = yy_cp;");
	indent_puts ("}");
	--indent_level;
}

static void cpp_gen_bu_action (void)
// Generate the code to perform the backing up.
{
	indent_puts ("case 0: /* must back up */");
	indent_puts ("/* undo the effects of YY_DO_BEFORE_ACTION */");
	indent_puts ("*yy_cp = YY_G(yy_hold_char);");

	if (fullspd || fulltbl)
		indent_puts ("yy_cp = YY_G(yy_last_accepting_cpos) + 1;");
	else
		/* Backing-up info for compressed tables is taken \after/
		 * yy_cp has been incremented for the next state.
		 */
		indent_puts ("yy_cp = YY_G(yy_last_accepting_cpos);");

	indent_puts ("yy_current_state = YY_G(yy_last_accepting_state);");
	indent_puts ("goto yy_find_action;");
	outc ('\n');
}

static void cpp_mkctbl (size_t sz)
// Make full-speed compressed transition table
{
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_TRANSITION, (void**)&yy_transition, sizeof(%s)},\n",
		    (sz >= INT16_MAX
		     || long_align) ? "flex_int32_t" : "flex_int16_t");
}

static void cpp_mkssltbl (void)
// Make start_state_list table
{
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_START_STATE_LIST, (void**)&yy_start_state_list, sizeof(%s)},\n",
		    "struct yy_trans_info*");
}

static void cpp_gen_yy_trans(size_t sz)
// Table of verify for transition and offset to next state. (sic)
{
	if (gentables)
		out_dec ("static const struct yy_trans_info yy_transition[%d] =\n    {\n", sz);
	else
		outn ("static const struct yy_trans_info *yy_transition = 0;");
}

static void cpp_start_state_list(size_t sz)
// Start initializer for table of pointers to start state
{
	/* Table of pointers to start states. */
	if (gentables)
		out_dec ("static const struct yy_trans_info *yy_start_state_list[%d] =\n", sz);
	else
		outn ("static const struct yy_trans_info **yy_start_state_list =0;");
}

static void cpp_mkecstbl(void)
// Make equivalence-class tables
{
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_EC, (void**)&yy_ec, sizeof(%s)},\n",
		    "YY_CHAR");
}

const char *cpp_skel[] = {
#include "cpp-skel.h"
    0,
};

/* This backend is only accessed through this method table */
struct flex_backend_t cpp_backend = {
	.suffix = cpp_suffix,
	.prolog = cpp_prolog,
	.skel = cpp_skel,
	.epilog = cpp_epilog,
	.trace_fmt = "#line %d \"%s\"\n",
	.int_define_fmt = "#define %s %d\n",
	.string_define_fmt = "#define %s %s\n",
	.open_block = "{",
	.close_block = "}",
	.table_opener = "    {",
	.table_closer = "    };\n",
	.linecomment = cpp_linecomment,
	.assign = cpp_assign,
	.cond = cpp_cond,
	.statement = cpp_statement,
	.forever = "for ( ; ; ) {",
	.get_int16_decl = cpp_get_int16_decl,
	.get_int32_decl = cpp_get_int32_decl,
	.get_state_decl = cpp_get_state_decl,
	.get_yy_char_decl = cpp_get_yy_char_decl,
	.mkeoltbl = cpp_mkeoltbl,
	.geneoltbl = cpp_geneoltbl,
	.gen_backing_up = cpp_gen_backing_up,
	.gen_bu_action = cpp_gen_bu_action,
	.mkctbl = cpp_mkctbl,
	.mkssltbl = cpp_mkssltbl,
	.gen_yy_trans = cpp_gen_yy_trans,
	.start_state_list = cpp_start_state_list,
	.state_entry_fmt = "    &yy_transition[%d],\n",
	.mkecstbl = cpp_mkecstbl,
};
