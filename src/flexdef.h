/* flexdef - definitions file for flex */

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

#pragma once

#include <config.h>

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flexint.h"

#include "common.h"

/* We use gettext. So, when we write strings which should be translated, we mark them with _() */
#ifdef ENABLE_NLS
#include "gettext.h"
#include <locale.h>
#define _(String) gettext(String)
#else
#define _(STRING) STRING
#endif /* ENABLE_NLS */

/* Always be prepared to generate an 8-bit scanner. */
#define CSIZE 256

/* Size of input alphabet - should be size of ASCII set. */
#ifndef DEFAULT_CSIZE
#define DEFAULT_CSIZE 128
#endif

/* Maximum line length we'll have to deal with. */
#define MAXLINE 2048

#define unspecified -1

/* Special chk[] values marking the slots taking by end-of-buffer and action
 * numbers.
 */
#define EOB_POSITION -1
#define ACTION_POSITION -2

/* Number of data items per line for -f output. */
#define NUMDATAITEMS 10

/* Number of lines of data in -f output before inserting a blank line for
 * readability.
 */
#define NUMDATALINES 10

/* transition_struct_out() definitions. */
#define TRANS_STRUCT_PRINT_LENGTH 14

/* Returns true if an nfa state has an epsilon out-transition slot
 * that can be used.  This definition is currently not used.
 */
#define FREE_EPSILON(state)             \
    (transchar[state] == SYM_EPSILON && \
     trans2[state] == NO_TRANSITION &&  \
     finalst[state] != state)

/* Returns true if an nfa state has an epsilon out-transition character
 * and both slots are free
 */
#define SUPER_FREE_EPSILON(state)       \
    (nfas[state].transchar == SYM_EPSILON && \
     nfas[state].trans1 == NO_TRANSITION)

/* A note on the following masks.  They are used to mark accepting numbers
 * as being special.  As such, they implicitly limit the number of accepting
 * numbers (i.e., rules) because if there are too many rules the rule numbers
 * will overload the mask bits.  Fortunately, this limit is \large/ (0x2000 ==
 * 8192) so unlikely to actually cause any problems.  A check is made in
 * new_rule() to ensure that this limit is not reached.
 */

/* Mask to mark a trailing context accepting number. */
#define YY_TRAILING_MASK 0x2000

/* Mask to mark the accepting number of the "head" of a trailing context
 * rule.
 */
#define YY_TRAILING_HEAD_MASK 0x4000

/* NIL must be 0.  If not, its special meaning when making equivalence classes
 * (it marks the representative of a given e.c.) will be unidentifiable.
 */
#define NIL 0

#define JAM -1 /* to mark a missing DFA transition */
#define NO_TRANSITION NIL
#define UNIQUE -1          /* marks a symbol as an e.c. representative */
#define INFINITE_REPEAT -1 /* for x{5,} constructions */

#define JAMSTATE -32766 /* marks a reference to the state that always jams */

#define SYM_EPSILON (CSIZE + 1) /* to mark transitions on the symbol epsilon */

#define ONE_STACK_SIZE 500 /* stack of states with only one out-transition */
#define SAME_TRANS -1      /* transition is the same as "default" entry for state */

/* The following percentages are used to tune table compression:

 * The percentage the number of out-transitions a state must be of the
 * number of equivalence classes in order to be considered for table
 * compaction by using protos.
 */
#define PROTO_SIZE_PERCENTAGE 15

/* The percentage the number of homogeneous out-transitions of a state
 * must be of the number of total out-transitions of the state in order
 * that the state's transition table is first compared with a potential
 * template of the most common out-transition instead of with the first
 * proto in the proto queue.
 */
#define CHECK_COM_PERCENTAGE 50

/* The percentage the number of differences between a state's transition
 * table and the proto it was first compared with must be of the total
 * number of out-transitions of the state in order to keep the first
 * proto as a good match and not search any further.
 */
#define FIRST_MATCH_DIFF_PERCENTAGE 10

/* The percentage the number of differences between a state's transition
 * table and the most similar proto must be of the state's total number
 * of out-transitions to use the proto as an acceptable close match.
 */
#define ACCEPTABLE_DIFF_PERCENTAGE 50

/* The percentage the number of homogeneous out-transitions of a state
 * must be of the number of total out-transitions of the state in order
 * to consider making a template from the state.
 */
#define TEMPLATE_SAME_PERCENTAGE 60

/* The percentage the number of differences between a state's transition
 * table and the most similar proto must be of the state's total number
 * of out-transitions to create a new proto from the state.
 */
#define NEW_PROTO_DIFF_PERCENTAGE 20

/* The percentage the total number of out-transitions of a state must be
 * of the number of equivalence classes in order to consider trying to
 * fit the transition table into "holes" inside the nxt/chk table.
 */
#define INTERIOR_FIT_PERCENTAGE 15

/* Size of region set aside to cache the complete transition table of
 * protos on the proto queue to enable quick comparisons.
 */
#define PROT_SAVE_SIZE 2000

#define MSP 50 /* maximum number of saved protos (protos on the proto queue) */

/* Maximum number of out-transitions a state can have that we'll rummage
 * around through the interior of the internal fast table looking for a
 * spot for it.
 */
#define MAX_XTIONS_FULL_INTERIOR_FIT 4

/* Maximum number of rules which will be reported as being associated
 * with a DFA state.
 */
#define MAX_ASSOC_RULES 100

/* Number that, if used to subscript an array, has a good chance of producing
 * an error; should be small enough to fit into a short.
 */
#define BAD_SUBSCRIPT -32767

/* Absolute value of largest number that can be stored in a short, with a
 * bit of slop thrown in for general paranoia.
 */
#define MAX_SHORT 32700

/* Declarations for global variables. */

/* Variables for flags:
 * printstats - if true (-v), dump statistics
 * syntaxerror - true if a syntax error has been found
 * eofseen - true if we've seen an eof in the input file
 * ddebug - if true (-d), make a "debug" scanner
 * trace - if true (-T), trace processing
 * nowarn - if true (-w), do not generate warnings
 * spprdflt - if true (-s), suppress the default rule
 * interactive - if true (-I), generate an interactive scanner
 * lex_compat - if true (-l), maximize compatibility with AT&T lex
 * posix_compat - if true (-X), maximize compatibility with POSIX lex
 * do_yylineno - if true, generate code to maintain yylineno
 * useecs - if true (-Ce flag), use equivalence classes
 * fulltbl - if true (-Cf flag), don't compress the DFA state table
 * usemecs - if true (-Cm flag), use meta-equivalence classes
 * fullspd - if true (-F flag), use Jacobson method of table representation
 * gen_line_dirs - if true (i.e., no -L flag), generate #line directives
 * performance_report - if > 0 (i.e., -p flag), generate a report relating
 *   to scanner performance; if > 1 (-p -p), report on minor performance
 *   problems, too
 * backing_up_report - if true (i.e., -b flag), generate "lex.backup" file
 *   listing backing-up states
 * C_plus_plus - if true (i.e., -+ flag), generate a C++ scanner class;
 *   otherwise, a standard C scanner
 * reentrant - if true (-R), generate a reentrant C scanner.
 * bison_bridge_lval - if true (--bison-bridge), bison pure calling convention.
 * bison_bridge_lloc - if true (--bison-locations), bison yylloc.
 * long_align - if true (-Ca flag), favor long-word alignment.
 * use_read - if true (-f, -F, or -Cr) then use read() for scanner input;
 *   otherwise, use fread().
 * yytext_is_array - if true (i.e., %array directive), then declare
 *   yytext as a array instead of a character pointer.  Nice and inefficient.
 * do_yywrap - do yywrap() processing on EOF.  If false, EOF treated as
 *   "no more files".
 * csize - size of character set for the scanner we're generating;
 *   128 for 7-bit chars and 256 for 8-bit
 * yymore_used - if true, yymore() is used in input rules
 * reject - if true, generate back-up tables for REJECT macro
 * real_reject - if true, scanner really uses REJECT (as opposed to just
 *   having "reject" set for variable trailing context)
 * continued_action - true if this rule's action is to "fall through" to
 *   the next rule's action (i.e., the '|' action)
 * in_rule - true if we're inside an individual rule, false if not.
 * yymore_really_used - whether to treat yymore() as really used, regardless
 *   of what we think based on references to it in the user's actions.
 * reject_really_used - same for REJECT
 * trace_hex - use hexadecimal numbers in trace/debug outputs instead of octals
 */

extern int printstats, syntaxerror, eofseen, ddebug, trace, nowarn, spprdflt;
extern int interactive, lex_compat, posix_compat, do_yylineno;
extern int useecs, fulltbl, usemecs, fullspd;
extern int gen_line_dirs, performance_report, backing_up_report;
extern int reentrant, bison_bridge_lval, bison_bridge_lloc;
extern bool ansi_func_defs, ansi_func_protos;
extern int C_plus_plus, long_align, use_read, yytext_is_array, do_yywrap;
extern int csize;
extern int yymore_used, reject, real_reject, continued_action, in_rule;
extern int yymore_really_used, reject_really_used;
extern int trace_hex;

/* Variables used in the flex input routines:
 * datapos - characters on current output line
 * dataline - number of contiguous lines of data in current data
 * 	statement.  Used to generate readable -f output
 * linenum - current input line number
 * skelfile - the skeleton file
 * skel - compiled-in skeleton array
 * skel_ind - index into "skel" array, if skelfile is nil
 * backing_up_file - file to summarize backing-up states to
 * infilename - name of input file
 * outfilename - name of output file
 * headerfilename - name of the .h file to generate
 * did_outfilename - whether outfilename was explicitly set
 * prefix - the prefix used for externally visible names ("yy" by default)
 * yyclass - yyFlexLexer subclass to use for YY_DECL
 * do_stdinit - whether to initialize yyin/yyout to stdin/stdout
 * use_stdout - the -t flag
 * input_files - array holding names of input files
 * program_name - name with which program was invoked
 * action_array - array to hold the rule actions
 * defs1_offset - index where the user's section 1 definitions start
 *	in action_array
 * prolog_offset - index where the prolog starts in action_array
 * action_offset - index where the non-prolog starts in action_array
 * action_index - index where the next action should go, with respect
 * 	to "action_array"
 */

extern int datapos, dataline, linenum;
extern FILE *backing_up_file;
extern const char *skel[];
extern int skel_ind;
extern String infilename, headerfilename;
extern std::string outfilename;
extern int did_outfilename;
extern String prefix, yyclass, extra_type;
extern int do_stdinit, use_stdout;
extern InputFiles input_files;
extern String program_name;
extern String action_array, defs1_array, prolog_array;

/* Variables for stack of states having only one out-transition:
 * onestate - state number
 * onesym - transition symbol
 * onenext - target state
 * onedef - default base entry
 * onesp - stack pointer
 */

extern int onestate[ONE_STACK_SIZE], onesym[ONE_STACK_SIZE];
extern int onenext[ONE_STACK_SIZE], onedef[ONE_STACK_SIZE], onesp;








/* True if the input rules include a rule with both variable-length head
 * and trailing context, false otherwise.
 */
extern int variable_trailing_context_rules;

/* Variables for protos:
 * numtemps - number of templates created
 * numprots - number of protos created
 * protprev - backlink to a more-recently used proto
 * protnext - forward link to a less-recently used proto
 * prottbl - base/def table entry for proto
 * protcomst - common state of proto
 * firstprot - number of the most recently used proto
 * lastprot - number of the least recently used proto
 * protsave contains the entire state array for protos
 */

extern int numtemps, numprots, protprev[MSP], protnext[MSP], prottbl[MSP];
extern int protcomst[MSP], firstprot, lastprot, protsave[PROT_SAVE_SIZE];

/* Variables for managing equivalence classes:
 * numecs - number of equivalence classes
 * nextecm - forward link of Equivalence Class members
 * ecgroup - class number or backward link of EC members
 * nummecs - number of meta-equivalence classes (used to compress
 *   templates)
 * tecfwd - forward link of meta-equivalence classes members
 * tecbck - backward link of MEC's
 */

/* Reserve enough room in the equivalence class arrays so that we
 * can use the CSIZE'th element to hold equivalence class information
 * for the NUL character.  Later we'll move this information into
 * the 0th element.
 */
extern int numecs, nextecm[CSIZE + 1], ecgroup[CSIZE + 1], nummecs;

/* Meta-equivalence classes are indexed starting at 1, so it's possible
 * that they will require positions from 1 .. CSIZE, i.e., CSIZE + 1
 * slots total (since the arrays are 0-based).  nextecm[] and ecgroup[]
 * don't require the extra position since they're indexed from 1 .. CSIZE - 1.
 */
extern int tecfwd[CSIZE + 1], tecbck[CSIZE + 1];


/* Variables for dfa machine data:
 * nxt - state to enter upon reading character
 * chk - check value to see if "nxt" applies
 * tnxt - internal nxt table for templates
 * NUL_ec - equivalence class of the NUL character
 * tblend - last "nxt/chk" table entry being used
 * firstfree - first empty entry in "nxt/chk" table
 * numas - number of DFA accepting states created; note that this
 *	is not necessarily the same value as num_rules, which is the analogous
 *	value for the NFA
 * numsnpairs - number of state/nextstate transition pairs
 * jambase - position in base/def where the default jam table starts
 * jamstate - state number corresponding to "jam" state
 * end_of_buffer_state - end-of-buffer dfa state number
 */

extern int NUL_ec, firstfree, tblend;
extern std::vector<int> nxt, chk, tnxt;
extern int numas;
extern int numsnpairs, jambase, jamstate;
extern int end_of_buffer_state;
extern bool nultrans;
extern Dfas dfas;



/* Variables for miscellaneous information:
 * nmstr - last NAME scanned by the scanner
 * sectnum - section number currently being parsed
 * nummt - number of empty nxt/chk table entries
 * hshcol - number of hash collisions detected by snstods
 * dfaeql - number of times a newly created dfa was equal to an old one
 * numeps - number of epsilon NFA states created
 * eps2 - number of epsilon states which have 2 out-transitions
 * num_reallocs - number of times it was necessary to realloc() a group
 *	  of arrays
 * tmpuses - number of DFA states that chain to templates
 * totnst - total number of NFA states used to make DFA states
 * peakpairs - peak number of transition pairs we had to store internally
 * numuniq - number of unique transitions
 * numdup - number of duplicate transitions
 * hshsave - number of hash collisions saved by checking number of states
 * num_backing_up - number of DFA states requiring backing up
 * bol_needed - whether scanner needs beginning-of-line recognition
 */

extern String nmstr;
extern int sectnum, nummt, hshcol, dfaeql, numeps, eps2, num_reallocs;
extern int tmpuses, totnst, peakpairs, numuniq, numdup, hshsave;
extern int num_backing_up, bol_needed;

extern int nlch; // nlch - default eol char







/* External functions that are cross-referenced among the flex source files. */



/* from file main.c */

void check_options(void);
void flexend(int);




/* from file parse.y */

/* Write out a message formatted with one string, pinpointing its location. */
void format_pinpoint_message(const String &, const String &);

/* Write out a message, pinpointing its location. */
void pinpoint_message(const char *);

/* Write out a warning, pinpointing it at the given line. */
void line_warning(const char *, int);

/* Write out a message, pinpointing it at the given line. */
void line_pinpoint(const char *, int);

/* Report a formatted syntax error. */
void format_synerr(const char *, const char *);
void synerr(const char *); /* report a syntax error */
void format_warn(const char *, const char *);
void warn(const char *);    /* report a warning */
void yyerror(const char *); /* report a parse error */




/* from file scan.l */

/* The Flex-generated scanner for flex. */
int flexscan(void);









/* For blocking out code from the header file. */
#define OUT_BEGIN_CODE() outn("m4_ifdef( [[M4_YY_IN_HEADER]],,[[")
#define OUT_END_CODE() outn("]])")

void flex_exit(int code);

/* ctype functions forced to return boolean */
#define b_islower(c) (islower(c) ? true : false)
#define b_isupper(c) (isupper(c) ? true : false)

/* return true if char is uppercase or lowercase. */
bool has_case(int c);

/* Change case of character if possible. */
int reverse_case(int c);

/* return false if [c1-c2] is ambiguous for a caseless scanner. */
bool range_covers_case(int c1, int c2);


extern bool tablesext, tablesverify;






template <typename T>
bool replace_all(T &str, const T &from, const T &to)
{
    bool replaced = false;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != T::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
        replaced = true;
    }
    return replaced;
}

inline bool replace_all(String &str, const String &from, const String &to)
{
    return replace_all<String>(str, from, to);
}

#define outn(x) processed_file << (x) << Context::eol
#define indent_puts outn



extern Rules rules;
extern StartConditions start_conditions;
extern CharacterClasses ccls;
extern Nfas nfas;

#define EOB_ACTION rules.size()

extern int num_eof_rules; // number of <<EOF>> rules
extern int default_rule; // number of the default rule

/* Global holding current type of state we're making. */
extern StateType current_state_type;
