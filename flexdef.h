/* flexdef - definitions file for flex */

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Vern Paxson.
 * 
 * The United States Government has rights in this work pursuant to
 * contract no. DE-AC03-76SF00098 between the United States Department of
 * Energy and the University of California.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* @(#) $Header$ (LBL) */

#ifndef FILE
#include <stdio.h>
#endif

#ifdef SYS_V
#include <string.h>

#ifdef AMIGA
#define bzero(s, n) setmem((char *)(s), (unsigned)(n), '\0')
#define abs(x) ((x) < 0 ? -(x) : (x))
#else
#define bzero(s, n) memset((char *)(s), '\0', (unsigned)(n))
#endif

#ifndef VMS
char *memset();
#else
/* memset is needed for old versions of the VMS C runtime library */
#define memset(s, c, n) \
	{ \
	register char *t = s; \
	register unsigned int m = n; \
	while ( m-- > 0 ) \
	    *t++ = c; \
	}
#define unlink delete
#define SHORT_FILE_NAMES
#endif
#endif

#ifndef SYS_V
#include <strings.h>
#ifdef lint
char *sprintf(); /* keep lint happy */
#endif
#endif


/* maximum line length we'll have to deal with */
#define MAXLINE BUFSIZ

/* maximum size of file name */
#define FILENAMESIZE 1024

#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))

#ifdef MS_DOS
#define abs(x) ((x) < 0 ? -(x) : (x))
#define SHORT_FILE_NAMES
#endif

#define true 1
#define false 0


#ifndef DEFAULT_SKELETON_FILE
#define DEFAULT_SKELETON_FILE "flex.skel"
#endif

/* special chk[] values marking the slots taking by end-of-buffer and action
 * numbers
 */
#define EOB_POSITION -1
#define ACTION_POSITION -2

/* number of data items per line for -f output */
#define NUMDATAITEMS 10

/* number of lines of data in -f output before inserting a blank line for
 * readability.
 */
#define NUMDATALINES 10

/* transition_struct_out() definitions */
#define TRANS_STRUCT_PRINT_LENGTH 15

/* returns true if an nfa state has an epsilon out-transition slot
 * that can be used.  This definition is currently not used.
 */
#define FREE_EPSILON(state) \
	(transchar[state] == SYM_EPSILON && \
	 trans2[state] == NO_TRANSITION && \
	 finalst[state] != state)

/* returns true if an nfa state has an epsilon out-transition character
 * and both slots are free
 */
#define SUPER_FREE_EPSILON(state) \
	(transchar[state] == SYM_EPSILON && \
	 trans1[state] == NO_TRANSITION) \

/* maximum number of NFA states that can comprise a DFA state.  It's real
 * big because if there's a lot of rules, the initial state will have a
 * huge epsilon closure.
 */
#define INITIAL_MAX_DFA_SIZE 750
#define MAX_DFA_SIZE_INCREMENT 750

/* array names to be used in generated machine.  They're short because
 * we write out one data statement (which names the array) for each element
 * in the array.
 */

/* points to list of rules accepted for a state */
#define ALIST "yy_accept"
#define ACCEPT "yy_acclist"	/* list of rules accepted for a state */
#define ECARRAY "yy_ec"	/* maps input characters to equivalence classes */
/* maps equivalence classes to meta-equivalence classes */
#define MATCHARRAY "yy_meta"
#define BASEARRAY "yy_base"	/* "base" array */
#define DEFARRAY "yy_def"	/* "default" array */
#define NEXTARRAY "yy_nxt"	/* "next" array */
#define CHECKARRAY "yy_chk"	/* "check" array */


/* a note on the following masks.  They are used to mark accepting numbers
 * as being special.  As such, they implicitly limit the number of accepting
 * numbers (i.e., rules) because if there are too many rules the rule numbers
 * will overload the mask bits.  Fortunately, this limit is \large/ (0x2000 ==
 * 8192) so unlikely to actually cause any problems.  A check is made in
 * new_rule() to ensure that this limit is not reached.
 */

/* mask to mark a trailing context accepting number */
#define YY_TRAILING_MASK 0x2000

/* mask to mark the accepting number of the "head" of a trailing context rule */
#define YY_TRAILING_HEAD_MASK 0x4000

/* maximum number of rules, as outlined in the above note */
#define MAX_RULE (YY_TRAILING_MASK - 1)


/* NIL must be 0.  If not, its special meaning when making equivalence classes
 * (it marks the representative of a given e.c.) will be unidentifiable
 */
#define NIL 0

#define JAM -1	/* to mark a missing DFA transition */
#define NO_TRANSITION NIL
#define UNIQUE -1	/* marks a symbol as an e.c. representative */
#define INFINITY -1	/* for x{5,} constructions */

/* size of input alphabet - should be size of ASCII set */
#define CSIZE 127

#define INITIAL_MAX_CCLS 100	/* max number of unique character classes */
#define MAX_CCLS_INCREMENT 100

/* size of table holding members of character classes */
#define INITIAL_MAX_CCL_TBL_SIZE 500
#define MAX_CCL_TBL_SIZE_INCREMENT 250

#define INITIAL_MAX_RULES 100	/* default maximum number of rules */
#define MAX_RULES_INCREMENT 100

#define INITIAL_MNS 2000	/* default maximum number of nfa states */
#define MNS_INCREMENT 1000	/* amount to bump above by if it's not enough */

#define INITIAL_MAX_DFAS 1000	/* default maximum number of dfa states */
#define MAX_DFAS_INCREMENT 1000

#define JAMSTATE -32766	/* marks a reference to the state that always jams */

/* enough so that if it's subtracted from an NFA state number, the result
 * is guaranteed to be negative
 */
#define MARKER_DIFFERENCE 32000
#define MAXIMUM_MNS 31999

/* maximum number of nxt/chk pairs for non-templates */
#define INITIAL_MAX_XPAIRS 2000
#define MAX_XPAIRS_INCREMENT 2000

/* maximum number of nxt/chk pairs needed for templates */
#define INITIAL_MAX_TEMPLATE_XPAIRS 2500
#define MAX_TEMPLATE_XPAIRS_INCREMENT 2500

#define SYM_EPSILON 0	/* to mark transitions on the symbol epsilon */

#define INITIAL_MAX_SCS 40	/* maximum number of start conditions */
#define MAX_SCS_INCREMENT 40	/* amount to bump by if it's not enough */

#define ONE_STACK_SIZE 500	/* stack of states with only one out-transition */
#define SAME_TRANS -1	/* transition is the same as "default" entry for state */

/* the following percentages are used to tune table compression:

 * the percentage the number of out-transitions a state must be of the
 * number of equivalence classes in order to be considered for table
 * compaction by using protos
 */
#define PROTO_SIZE_PERCENTAGE 15

/* the percentage the number of homogeneous out-transitions of a state
 * must be of the number of total out-transitions of the state in order
 * that the state's transition table is first compared with a potential 
 * template of the most common out-transition instead of with the first
 * proto in the proto queue
 */
#define CHECK_COM_PERCENTAGE 50

/* the percentage the number of differences between a state's transition
 * table and the proto it was first compared with must be of the total
 * number of out-transitions of the state in order to keep the first
 * proto as a good match and not search any further
 */
#define FIRST_MATCH_DIFF_PERCENTAGE 10

/* the percentage the number of differences between a state's transition
 * table and the most similar proto must be of the state's total number
 * of out-transitions to use the proto as an acceptable close match
 */
#define ACCEPTABLE_DIFF_PERCENTAGE 50

/* the percentage the number of homogeneous out-transitions of a state
 * must be of the number of total out-transitions of the state in order
 * to consider making a template from the state
 */
#define TEMPLATE_SAME_PERCENTAGE 60

/* the percentage the number of differences between a state's transition
 * table and the most similar proto must be of the state's total number
 * of out-transitions to create a new proto from the state
 */
#define NEW_PROTO_DIFF_PERCENTAGE 20

/* the percentage the total number of out-transitions of a state must be
 * of the number of equivalence classes in order to consider trying to
 * fit the transition table into "holes" inside the nxt/chk table.
 */
#define INTERIOR_FIT_PERCENTAGE 15

/* size of region set aside to cache the complete transition table of
 * protos on the proto queue to enable quick comparisons
 */
#define PROT_SAVE_SIZE 2000

#define MSP 50	/* maximum number of saved protos (protos on the proto queue) */

/* maximum number of out-transitions a state can have that we'll rummage
 * around through the interior of the internal fast table looking for a
 * spot for it
 */
#define MAX_XTIONS_FULL_INTERIOR_FIT 4

/* maximum number of rules which will be reported as being associated
 * with a DFA state
 */
#define MAX_ASSOC_RULES 100

/* number that, if used to subscript an array, has a good chance of producing
 * an error; should be small enough to fit into a short
 */
#define BAD_SUBSCRIPT -32767

/* absolute value of largest number that can be stored in a short, with a
 * bit of slop thrown in for general paranoia.
 */
#define MAX_SHORT 32766


/* Declarations for global variables. */

/* variables for symbol tables:
 * sctbl - start-condition symbol table
 * ndtbl - name-definition symbol table
 * ccltab - character class text symbol table
 */

struct hash_entry
    {
    struct hash_entry *prev, *next;
    char *name;
    char *str_val;
    int int_val;
    } ;

typedef struct hash_entry *hash_table[];

#define NAME_TABLE_HASH_SIZE 101
#define START_COND_HASH_SIZE 101
#define CCL_HASH_SIZE 101

extern struct hash_entry *ndtbl[NAME_TABLE_HASH_SIZE]; 
extern struct hash_entry *sctbl[START_COND_HASH_SIZE];
extern struct hash_entry *ccltab[CCL_HASH_SIZE];


/* variables for flags:
 * printstats - if true (-v), dump statistics
 * syntaxerror - true if a syntax error has been found
 * eofseen - true if we've seen an eof in the input file
 * ddebug - if true (-d), make a "debug" scanner
 * trace - if true (-T), trace processing
 * spprdflt - if true (-s), suppress the default rule
 * interactive - if true (-I), generate an interactive scanner
 * caseins - if true (-i), generate a case-insensitive scanner
 * useecs - if true (-ce flag), use equivalence classes
 * fulltbl - if true (-cf flag), don't compress the DFA state table
 * usemecs - if true (-cm flag), use meta-equivalence classes
 * fullspd - if true (-F flag), use Jacobson method of table representation
 * gen_line_dirs - if true (i.e., no -L flag), generate #line directives
 * performance_report - if true (i.e., -p flag), generate a report relating
 *   to scanner performance
 * backtrack_report - if true (i.e., -b flag), generate "lex.backtrack" file
 *   listing backtracking states
 * yymore_used - if true, yymore() is used in input rules
 * reject - if true, generate backtracking tables for REJECT macro
 * real_reject - if true, scanner really uses REJECT (as opposed to just
 *               having "reject" set for variable trailing context)
 * continued_action - true if this rule's action is to "fall through" to
 *                    the next rule's action (i.e., the '|' action)
 * yymore_really_used - has a REALLY_xxx value indicating whether a
 *                      %used or %notused was used with yymore()
 * reject_really_used - same for REJECT
 */

extern int printstats, syntaxerror, eofseen, ddebug, trace, spprdflt;
extern int interactive, caseins, useecs, fulltbl, usemecs;
extern int fullspd, gen_line_dirs, performance_report, backtrack_report;
extern int yymore_used, reject, real_reject, continued_action;

#define REALLY_NOT_DETERMINED 0
#define REALLY_USED 1
#define REALLY_NOT_USED 2
extern int yymore_really_used, reject_really_used;


/* variables used in the flex input routines:
 * datapos - characters on current output line
 * dataline - number of contiguous lines of data in current data
 *    statement.  Used to generate readable -f output
 * skelfile - the skeleton file
 * yyin - input file
 * temp_action_file - temporary file to hold actions
 * backtrack_file - file to summarize backtracking states to
 * action_file_name - name of the temporary file
 * infilename - name of input file
 * linenum - current input line number
 */

extern int datapos, dataline, linenum;
extern FILE *skelfile, *yyin, *temp_action_file, *backtrack_file;
extern char *infilename;
extern char action_file_name[];


/* variables for stack of states having only one out-transition:
 * onestate - state number
 * onesym - transition symbol
 * onenext - target state
 * onedef - default base entry
 * onesp - stack pointer
 */

extern int onestate[ONE_STACK_SIZE], onesym[ONE_STACK_SIZE];
extern int onenext[ONE_STACK_SIZE], onedef[ONE_STACK_SIZE], onesp;


/* variables for nfa machine data:
 * current_mns - current maximum on number of NFA states
 * num_rules - number of the last accepting state; also is number of
 *             rules created so far
 * current_max_rules - current maximum number of rules
 * lastnfa - last nfa state number created
 * firstst - physically the first state of a fragment
 * lastst - last physical state of fragment
 * finalst - last logical state of fragment
 * transchar - transition character
 * trans1 - transition state
 * trans2 - 2nd transition state for epsilons
 * accptnum - accepting number
 * assoc_rule - rule associated with this NFA state (or 0 if none)
 * state_type - a STATE_xxx type identifying whether the state is part
 *              of a normal rule, the leading state in a trailing context
 *              rule (i.e., the state which marks the transition from
 *              recognizing the text-to-be-matched to the beginning of
 *              the trailing context), or a subsequent state in a trailing
 *              context rule
 * rule_type - a RULE_xxx type identifying whether this a a ho-hum
 *             normal rule or one which has variable head & trailing
 *             context
 * rule_linenum - line number associated with rule
 */

extern int current_mns, num_rules, current_max_rules, lastnfa;
extern int *firstst, *lastst, *finalst, *transchar, *trans1, *trans2;
extern int *accptnum, *assoc_rule, *state_type, *rule_type, *rule_linenum;

/* different types of states; values are useful as masks, as well, for
 * routines like check_trailing_context()
 */
#define STATE_NORMAL 0x1
#define STATE_TRAILING_CONTEXT 0x2

/* global holding current type of state we're making */

extern int current_state_type;

/* different types of rules */
#define RULE_NORMAL 0
#define RULE_VARIABLE 1

/* true if the input rules include a rule with both variable-length head
 * and trailing context, false otherwise
 */
extern int variable_trailing_context_rules;


/* variables for protos:
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


/* variables for managing equivalence classes:
 * numecs - number of equivalence classes
 * nextecm - forward link of Equivalence Class members
 * ecgroup - class number or backward link of EC members
 * nummecs - number of meta-equivalence classes (used to compress
 *   templates)
 * tecfwd - forward link of meta-equivalence classes members
 * tecbck - backward link of MEC's
 */

extern int numecs, nextecm[CSIZE + 1], ecgroup[CSIZE + 1], nummecs;
extern int tecfwd[CSIZE + 1], tecbck[CSIZE + 1];


/* variables for start conditions:
 * lastsc - last start condition created
 * current_max_scs - current limit on number of start conditions
 * scset - set of rules active in start condition
 * scbol - set of rules active only at the beginning of line in a s.c.
 * scxclu - true if start condition is exclusive
 * sceof - true if start condition has EOF rule
 * scname - start condition name
 * actvsc - stack of active start conditions for the current rule
 */

extern int lastsc, current_max_scs, *scset, *scbol, *scxclu, *sceof, *actvsc;
extern char **scname;


/* variables for dfa machine data:
 * current_max_dfa_size - current maximum number of NFA states in DFA
 * current_max_xpairs - current maximum number of non-template xtion pairs
 * current_max_template_xpairs - current maximum number of template pairs
 * current_max_dfas - current maximum number DFA states
 * lastdfa - last dfa state number created
 * nxt - state to enter upon reading character
 * chk - check value to see if "nxt" applies
 * tnxt - internal nxt table for templates
 * base - offset into "nxt" for given state
 * def - where to go if "chk" disallows "nxt" entry
 * tblend - last "nxt/chk" table entry being used
 * firstfree - first empty entry in "nxt/chk" table
 * dss - nfa state set for each dfa
 * dfasiz - size of nfa state set for each dfa
 * dfaacc - accepting set for each dfa state (or accepting number, if
 *    -r is not given)
 * accsiz - size of accepting set for each dfa state
 * dhash - dfa state hash value
 * numas - number of DFA accepting states created; note that this
 *    is not necessarily the same value as num_rules, which is the analogous
 *    value for the NFA
 * numsnpairs - number of state/nextstate transition pairs
 * jambase - position in base/def where the default jam table starts
 * jamstate - state number corresponding to "jam" state
 * end_of_buffer_state - end-of-buffer dfa state number
 */

extern int current_max_dfa_size, current_max_xpairs;
extern int current_max_template_xpairs, current_max_dfas;
extern int lastdfa, lasttemp, *nxt, *chk, *tnxt;
extern int *base, *def, tblend, firstfree, **dss, *dfasiz;
extern union dfaacc_union
    {
    int *dfaacc_set;
    int dfaacc_state;
    } *dfaacc;
extern int *accsiz, *dhash, numas;
extern int numsnpairs, jambase, jamstate;
extern int end_of_buffer_state;

/* variables for ccl information:
 * lastccl - ccl index of the last created ccl
 * current_maxccls - current limit on the maximum number of unique ccl's
 * cclmap - maps a ccl index to its set pointer
 * ccllen - gives the length of a ccl
 * cclng - true for a given ccl if the ccl is negated
 * cclreuse - counts how many times a ccl is re-used
 * current_max_ccl_tbl_size - current limit on number of characters needed
 *	to represent the unique ccl's
 * ccltbl - holds the characters in each ccl - indexed by cclmap
 */

extern int lastccl, current_maxccls, *cclmap, *ccllen, *cclng, cclreuse;
extern int current_max_ccl_tbl_size;
extern char *ccltbl;


/* variables for miscellaneous information:
 * starttime - real-time when we started
 * endtime - real-time when we ended
 * nmstr - last NAME scanned by the scanner
 * sectnum - section number currently being parsed
 * nummt - number of empty nxt/chk table entries
 * hshcol - number of hash collisions detected by snstods
 * dfaeql - number of times a newly created dfa was equal to an old one
 * numeps - number of epsilon NFA states created
 * eps2 - number of epsilon states which have 2 out-transitions
 * num_reallocs - number of times it was necessary to realloc() a group
 *		  of arrays
 * tmpuses - number of DFA states that chain to templates
 * totnst - total number of NFA states used to make DFA states
 * peakpairs - peak number of transition pairs we had to store internally
 * numuniq - number of unique transitions
 * numdup - number of duplicate transitions
 * hshsave - number of hash collisions saved by checking number of states
 * num_backtracking - number of DFA states requiring back-tracking
 * bol_needed - whether scanner needs beginning-of-line recognition
 */

extern char *starttime, *endtime, nmstr[MAXLINE];
extern int sectnum, nummt, hshcol, dfaeql, numeps, eps2, num_reallocs;
extern int tmpuses, totnst, peakpairs, numuniq, numdup, hshsave;
extern int num_backtracking, bol_needed;

char *allocate_array(), *reallocate_array();

#define allocate_integer_array(size) \
	(int *) allocate_array( size, sizeof( int ) )

#define reallocate_integer_array(array,size) \
	(int *) reallocate_array( (char *) array, size, sizeof( int ) )

#define allocate_int_ptr_array(size) \
	(int **) allocate_array( size, sizeof( int * ) )

#define allocate_char_ptr_array(size) \
	(char **) allocate_array( size, sizeof( char * ) )

#define allocate_dfaacc_union(size) \
	(union dfaacc_union *) \
		allocate_array( size, sizeof( union dfaacc_union ) )

#define reallocate_int_ptr_array(array,size) \
	(int **) reallocate_array( (char *) array, size, sizeof( int * ) )

#define reallocate_char_ptr_array(array,size) \
	(char **) reallocate_array( (char *) array, size, sizeof( char * ) )

#define reallocate_dfaacc_union(array, size) \
	(union dfaacc_union *)  reallocate_array( (char *) array, size, sizeof( union dfaacc_union ) )

#define allocate_character_array(size) allocate_array( size, sizeof( char ) )

#define reallocate_character_array(array,size) \
	reallocate_array( array, size, sizeof( char ) )


/* used to communicate between scanner and parser.  The type should really
 * be YYSTYPE, but we can't easily get our hands on it.
 */
extern int yylval;
