/*
 *  Definitions for flex.
 *
 * modification history
 * --------------------
 * 02b kg, vp   30sep87  .added definitions for fast scanner; misc. cleanup
 * 02a vp       27jun86  .translated into C/FTL
 */

/*
 * Copyright (c) 1987, the University of California
 * 
 * The United States Government has rights in this work pursuant to
 * contract no. DE-AC03-76SF00098 between the United States Department of
 * Energy and the University of California.
 * 
 * This program may be redistributed.  Enhancements and derivative works
 * may be created provided the new works, if made available to the general
 * public, are made available for use by anyone.
 */

#include <stdio.h>

#ifdef SV
#include <string.h>
#define bzero(s, n) memset((char *)(s), '\000', (unsigned)(n))
#else
#include <strings.h>
#endif

char *sprintf(); /* keep lint happy */


/* maximum line length we'll have to deal with */
#define MAXLINE BUFSIZ

/* maximum size of file name */
#define FILENAMESIZE 1024

#define min(x,y) (x < y ? x : y)
#define max(x,y) (x > y ? x : y)

#define true 1
#define false 0


#ifndef DEFAULT_SKELETON_FILE
#define DEFAULT_SKELETON_FILE "flex.skel"
#endif

#ifndef FAST_SKELETON_FILE
#define FAST_SKELETON_FILE "flex.fastskel"
#endif

/* special nxt[] action number for the "at the end of the input buffer" state */
/* note: -1 is already taken by YY_NEW_FILE */
#define END_OF_BUFFER_ACTION -3
/* action number for default action for fast scanners */
#define DEFAULT_ACTION -2

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

#define ALIST 'l'	/* points to list of rules accepted for a state */
#define ACCEPT 'a'	/* list of rules accepted for a state */
#define ECARRAY 'e'	/* maps input characters to equivalence classes */
#define MATCHARRAY 'm'	/* maps equivalence classes to meta-equivalence classes */
#define BASEARRAY 'b'	/* "base" array */
#define DEFARRAY 'd'	/* "default" array */
#define NEXTARRAY 'n'	/* "next" array */
#define CHECKARRAY 'c'	/* "check" array */

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

#define INITIAL_MAXCCLS 100	/* max number of unique character classes */
#define MAXCCLS_INCREMENT 100

/* size of table holding members of character classes */
#define INITIAL_MAX_CCL_TBL_SIZE 500
#define MAX_CCL_TBL_SIZE_INCREMENT 250

#define INITIAL_MNS 2000	/* default maximum number of nfa states */
#define MNS_INCREMENT 1000	/* amount to bump above by if it's not enough */

#define INITIAL_MAX_DFAS 1000	/* default maximum number of dfa states */
#define MAX_DFAS_INCREMENT 1000

#define JAMSTATE -32766	/* marks a reference to the state that always jams */

/* enough so that if it's subtracted from an NFA state number, the result
 * is guarenteed to be negative
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

/* the percentage the number of homogenous out-transitions of a state
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
#define MAX_XTIONS_FOR_FULL_INTERIOR_FIT 4

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
    char *val;
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
 * reject - if true (-r flag), generate tables for REJECT macro
 * fullspd - if true (-F flag), use Jacobson method of table representation
 * gen_line_dirs - if true (i.e., no -L flag), generate #line directives
 */

extern int printstats, syntaxerror, eofseen, ddebug, trace, spprdflt;
extern int interactive, caseins, useecs, fulltbl, usemecs, reject;
extern int fullspd, gen_line_dirs;


/* variables used in the flex input routines:
 * datapos - characters on current output line
 * dataline - number of contiguous lines of data in current data
 *    statement.  Used to generate readable -f output
 * skelfile - fd of the skeleton file
 * yyin - input file
 * temp_action_file - temporary file to hold actions
 * action_file_name - name of the temporary file
 * infilename - name of input file
 * linenum - current input line number
 */

extern int datapos, dataline, linenum;
extern FILE *skelfile, *yyin, *temp_action_file;
extern char *infilename;
extern char *action_file_name;


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
 * accnum - number of the last accepting state
 * firstst - physically the first state of a fragment
 * lastst - last physical state of fragment
 * finalst - last logical state of fragment
 * transchar - transition character
 * trans1 - transition state
 * trans2 - 2nd transition state for epsilons
 * accptnum - accepting number
 * lastnfa - last nfa state number created
 */

extern int current_mns;
extern int accnum, *firstst, *lastst, *finalst, *transchar;
extern int *trans1, *trans2, *accptnum, lastnfa;


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
 * nextecm - forward link of Equivalenc Class members
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
 * actvsc - stack of active start conditions for the current rule
 */

extern int lastsc, current_max_scs, *scset, *scbol, *scxclu, *actvsc;


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
 * todo - queue of DFAs still to be processed
 * todo_head - head of todo queue
 * todo_next - next available entry on todo queue
 * numas - number of DFA accepting states created; note that this
 *    is not necessarily the same value as accnum, which is the analogous
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
extern int *accsiz, *dhash, *todo, todo_head, todo_next, numas;
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
 */

extern char *starttime, *endtime, nmstr[MAXLINE];
extern int sectnum, nummt, hshcol, dfaeql, numeps, eps2, num_reallocs;
extern int tmpuses, totnst, peakpairs, numuniq, numdup, hshsave;

char *allocate_array(), *reallocate_array();

#define allocate_integer_array(size) \
	(int *) allocate_array( size, sizeof( int ) )

#define reallocate_integer_array(array,size) \
	(int *) reallocate_array( (char *) array, size, sizeof( int ) )

#define allocate_integer_pointer_array(size) \
	(int **) allocate_array( size, sizeof( int * ) )

#define allocate_dfaacc_union(size) \
	(union dfaacc_union *) \
		allocate_array( size, sizeof( union dfaacc_union ) )

#define reallocate_integer_pointer_array(array,size) \
	(int **) reallocate_array( (char *) array, size, sizeof( int * ) )

#define reallocate_dfaacc_union(array, size) \
	(union dfaacc_union *)  reallocate_array( (char *) array, size, sizeof( union dfaacc_union ) )

#define allocate_character_array(size) allocate_array( size, sizeof( char ) )

#define reallocate_character_array(array,size) \
	reallocate_array( array, size, sizeof( char ) )


/* used to communicate between scanner and parser.  The type should really
 * be YYSTYPE, but we can't easily get our hands on it.
 */
extern int yylval;
