/* flex - tool to generate fast lexical analyzers
 *
 *
 * Copyright (c) 1987, the University of California
 * 
 * The United States Government has rights in this work pursuant to
 * contract no. DE-AC03-76SF00098 between the United States Department of
 * Energy and the University of California.
 * 
 * This program may be redistributed.  Enhancements and derivative works
 * may be created provided the new works, if made available to the general
 * public, are made available for use by anyone.
 *
 *
 * ver   date  who    remarks
 * ---   ----  ------ -------------------------------------------------------
 * 04b 30sep87 kg, vp .implemented (part of) Van Jacobson's fast scanner design
 * 04a 27jun86 vp     .translated from Ratfor into C
 * 01a 22aug83 vp     .written.  Original version by Jef Poskanzer.
 */

#include "flexdef.h"


/* these globals are all defined and commented in flexdef.h */
int printstats, syntaxerror, eofseen, ddebug, trace, spprdflt;
int interactive, caseins, useecs, fulltbl, usemecs, reject;
int fullspd, gen_line_dirs;
int datapos, dataline, linenum;
FILE *skelfile = NULL;
char *infilename = NULL;
int onestate[ONE_STACK_SIZE], onesym[ONE_STACK_SIZE];
int onenext[ONE_STACK_SIZE], onedef[ONE_STACK_SIZE], onesp;
int current_mns;
int accnum, *firstst, *lastst, *finalst, *transchar;
int *trans1, *trans2, *accptnum, lastnfa;
int numtemps, numprots, protprev[MSP], protnext[MSP], prottbl[MSP];
int protcomst[MSP], firstprot, lastprot, protsave[PROT_SAVE_SIZE];
int numecs, nextecm[CSIZE + 1], ecgroup[CSIZE + 1], nummecs, tecfwd[CSIZE + 1];
int tecbck[CSIZE + 1];
int lastsc, current_max_scs, *scset, *scbol, *scxclu, *actvsc;
int current_max_dfa_size, current_max_xpairs;
int current_max_template_xpairs, current_max_dfas;
int lastdfa, *nxt, *chk, *tnxt;
int *base, *def, tblend, firstfree, numtemps, **dss, *dfasiz;
union dfaacc_union *dfaacc;
int *accsiz, *dhash, *todo, todo_head, todo_next, numas;
int numsnpairs, jambase, jamstate;
int lastccl, current_maxccls, *cclmap, *ccllen, *cclng, cclreuse;
int current_max_ccl_tbl_size;
char *ccltbl;
char *starttime, *endtime, nmstr[MAXLINE];
int sectnum, nummt, hshcol, dfaeql, numeps, eps2, num_reallocs;
int tmpuses, totnst, peakpairs, numuniq, numdup, hshsave;
FILE *temp_action_file;
int end_of_buffer_state;
char *action_file_name = "/tmp/flexXXXXXX";


/* flex - main program
 *
 * synopsis (from the shell)
 *    flex [-v] [file ...]
 */

main( argc, argv )
int argc;
char **argv;

    {
    flexinit( argc, argv );

    readin();

    if ( ! syntaxerror )
	{
	/* convert the ndfa to a dfa */
	ntod();

	/* generate the C state transition tables from the DFA */
	make_tables();
	}

    /* note, flexend does not return.  It exits with its argument as status. */

    flexend( 0 );
    }


/* flexend - terminate flex
 *
 * synopsis
 *    int status;
 *    flexend( status );
 *
 *    status is exit status.
 *
 * note
 *    This routine does not return.
 */

flexend( status )
int status;

    {
    int tblsiz;
    char *gettime();

    if ( skelfile != NULL )
	(void) fclose( skelfile );

    if ( temp_action_file )
	{
	(void) fclose( temp_action_file );
	(void) unlink( action_file_name );
	}

    if ( printstats )
	{
	endtime = gettime();

	fprintf( stderr, "flex usage statistics:\n" );
	fprintf( stderr, "  started at %s, finished at %s\n",
		 starttime, endtime );

	fprintf( stderr, "  %d/%d NFA states\n", lastnfa, current_mns );
	fprintf( stderr, "  %d/%d DFA states (%d words)\n", lastdfa,
			 current_max_dfas, totnst );
	fprintf( stderr, "  %d rules\n", accnum );
	fprintf( stderr, "  %d/%d start conditions\n", lastsc,
			 current_max_scs );
	fprintf( stderr, "  %d epsilon states, %d double epsilon states\n",
		 numeps, eps2 );

	if ( lastccl == 0 )
	    fprintf( stderr, "  no character classes\n" );
	else
	    fprintf( stderr,
	"  %d/%d character classes needed %d/%d words of storage, %d reused\n",
		     lastccl, current_maxccls,
		     cclmap[lastccl] + ccllen[lastccl] - 1,
		     current_max_ccl_tbl_size, cclreuse );

	fprintf( stderr, "  %d state/nextstate pairs created\n", numsnpairs );
	fprintf( stderr, "  %d/%d unique/duplicate transitions\n",
		 numuniq, numdup );

	if ( fulltbl )
	    {
	    tblsiz = lastdfa * numecs;
	    fprintf( stderr, "  %d table entries\n", tblsiz );
	    }

	else
	    {
	    tblsiz = 2 * (lastdfa + numtemps) + 2 * tblend;

	    fprintf( stderr, "  %d/%d base/def entries created\n",
		     lastdfa + numtemps, current_max_dfas );
	    fprintf( stderr, "  %d/%d (peak %d) nxt/chk entries created\n",
		     tblend, current_max_xpairs, peakpairs );
	    fprintf( stderr,
		     "  %d/%d (peak %d) template nxt/chk entries created\n",
		     numtemps * nummecs, current_max_template_xpairs,
		     numtemps * numecs );
	    fprintf( stderr, "  %d empty table entries\n", nummt );
	    fprintf( stderr, "  %d protos created\n", numprots );
	    fprintf( stderr, "  %d templates created, %d uses\n",
		     numtemps, tmpuses );
	    }

	if ( useecs )
	    {
	    tblsiz = tblsiz + CSIZE;
	    fprintf( stderr, "  %d/%d equivalence classes created\n",
		     numecs, CSIZE );
	    }

	if ( usemecs )
	    {
	    tblsiz = tblsiz + numecs;
	    fprintf( stderr, "  %d/%d meta-equivalence classes created\n",
		     nummecs, CSIZE );
	    }

	fprintf( stderr, "  %d (%d saved) hash collisions, %d DFAs equal\n",
		 hshcol, hshsave, dfaeql );
	fprintf( stderr, "  %d sets of reallocations needed\n", num_reallocs );
	fprintf( stderr, "  %d total table entries needed\n", tblsiz );
	}

    exit( status );
    }


/* flexinit - initialize flex
 *
 * synopsis
 *    int argc;
 *    char **argv;
 *    flexinit( argc, argv );
 */

flexinit( argc, argv )
int argc;
char **argv;

    {
    int i, sawcmpflag, use_stdout;
    char *arg, *skelname = NULL, *gettime(), clower(), *mktemp();

    printstats = syntaxerror = trace = spprdflt = interactive = caseins = false;
    ddebug = fulltbl = reject = fullspd = false;
    gen_line_dirs = usemecs = useecs = true;

    sawcmpflag = false;
    use_stdout = false;

    /* read flags */
    for ( --argc, ++argv; argc ; --argc, ++argv )
	{
	if ( argv[0][0] != '-' || argv[0][1] == '\0' )
	    break;

	arg = argv[0];

	for ( i = 1; arg[i] != '\0'; ++i )
	    switch ( arg[i] )
		{
		case 'c':
		    if ( i != 1 )
			flexerror( "-c flag must be given separately" );

		    if ( ! sawcmpflag )
			{
			useecs = false;
			usemecs = false;
			fulltbl = false;
			sawcmpflag = true;
			}

		    for ( ++i; arg[i] != '\0'; ++i )
			switch ( clower( arg[i] ) )
			    {
			    case 'e':
				useecs = true;
				break;

			    case 'F':
				fullspd = true;
				break;

			    case 'f':
				fulltbl = true;
				break;

			    case 'm':
				usemecs = true;
				break;

			    default:
				lerrif( "unknown -c option %c",
					(int) arg[i] );
				break;
			    }
		    
		    goto get_next_arg;

		case 'd':
		    ddebug = true;
		    break;

		case 'f':
		    useecs = usemecs = false;
		    fulltbl = true;
		    break;

		case 'I':
		    interactive = true;
		    break;

		case 'i':
		    caseins = true;
		    break;

		case 'L':
		    gen_line_dirs = false;
		    break;

		case 'r':
		    reject = true;
		    break;

		case 'F':
		    useecs = usemecs = false;
		    fullspd = true;
		    break;

		case 'S':
		    if ( i != 1 )
			flexerror( "-S flag must be given separately" );

		    skelname = arg + i + 1;
		    goto get_next_arg;

		case 's':
		    spprdflt = true;
		    break;

		case 't':
		    use_stdout = true;
		    break;

		case 'T':
		    trace = true;
		    break;

		case 'v':
		    printstats = true;
		    break;

		default:
		    lerrif( "unknown flag %c", (int) arg[i] );
		    break;
		}

get_next_arg: /* used by -c and -S flags in lieu of a "continue 2" control */
	;
	}

    if ( (fulltbl || fullspd) && usemecs )
	flexerror( "full table and -cm don't make sense together" );

    if ( (fulltbl || fullspd) && interactive )
	flexerror( "full table and -I are (currently) incompatible" );

    if ( (fulltbl || fullspd) && reject )
	flexerror( "reject (-r) cannot be used with -f or -F" );

    if ( fulltbl && fullspd )
	flexerror( "full table and -F are mutually exclusive" );

    if ( ! skelname )
	{
	static char skeleton_name_storage[400];

	skelname = skeleton_name_storage;

	if ( fullspd || fulltbl )
	    (void) strcpy( skelname, FAST_SKELETON_FILE );
	else
	    (void) strcpy( skelname, DEFAULT_SKELETON_FILE );
	}

    if ( ! use_stdout )
	{
	FILE *prev_stdout = freopen( "lex.yy.c", "w", stdout );

	if ( prev_stdout == NULL )
	    flexerror( "could not create lex.yy.c" );
	}

    if ( argc )
	{
	if ( argc > 1 )
	    flexerror( "extraneous argument(s) given" );

	yyin = fopen( infilename = argv[0], "r" );

	if ( yyin == NULL )
	    lerrsf( "can't open %s", argv[0] );
	}

    else
	yyin = stdin;

    lastccl = 0;
    lastsc = 0;

    /* initialize the statistics */
    starttime = gettime();

    if ( (skelfile = fopen( skelname, "r" )) == NULL )
	lerrsf( "can't open skeleton file %s", skelname );

    (void) mktemp( action_file_name );

    if ( (temp_action_file = fopen( action_file_name, "w" )) == NULL )
	lerrsf( "can't open temporary action file %s", action_file_name );

    lastdfa = lastnfa = accnum = numas = numsnpairs = tmpuses = 0;
    numecs = numeps = eps2 = num_reallocs = hshcol = dfaeql = totnst = 0;
    numuniq = numdup = hshsave = eofseen = datapos = dataline = 0;
    onesp = numprots = 0;

    linenum = sectnum = 1;
    firstprot = NIL;

    /* used in mkprot() so that the first proto goes in slot 1
     * of the proto queue
     */
    lastprot = 1;

    if ( useecs )
	{
	/* set up doubly-linked equivalence classes */
	ecgroup[1] = NIL;

	for ( i = 2; i <= CSIZE; ++i )
	    {
	    ecgroup[i] = i - 1;
	    nextecm[i - 1] = i;
	    }

	nextecm[CSIZE] = NIL;
	}

    else
	{ /* put everything in its own equivalence class */
	for ( i = 1; i <= CSIZE; ++i )
	    {
	    ecgroup[i] = i;
	    nextecm[i] = BAD_SUBSCRIPT;	/* to catch errors */
	    }
	}

    set_up_initial_allocations();
    }


/* readin - read in the rules section of the input file(s)
 *
 * synopsis
 *    readin();
 */

readin()

    {
    fputs( "#define YY_DEFAULT_ACTION ", stdout );

    if ( spprdflt )
	fputs( "YY_FATAL_ERROR( \"flex scanner jammed\" )", stdout );
    else
	fputs( "ECHO", stdout );

    fputs( ";\n", stdout );

    if ( ddebug )
	puts( "#define FLEX_DEBUG" );
    if ( useecs )
	puts( "#define FLEX_USE_ECS" );
    if ( usemecs )
	puts( "#define FLEX_USE_MECS" );
    if ( interactive )
	puts( "#define FLEX_INTERACTIVE_SCANNER" );
    if ( reject )
	puts( "#define FLEX_REJECT_ENABLED" );
    if ( fulltbl )
	puts( "#define FLEX_FULL_TABLE" );

    skelout();

    line_directive_out( stdout );

    if ( yyparse() )
	lerrif( "fatal parse error at line %d", linenum );

    if ( useecs )
	{
	numecs = cre8ecs( nextecm, ecgroup, CSIZE );
	ccl2ecl();
	}

    else
	numecs = CSIZE;

    }



/* set_up_initial_allocations - allocate memory for internal tables */

set_up_initial_allocations()

    {
    current_mns = INITIAL_MNS;
    firstst = allocate_integer_array( current_mns );
    lastst = allocate_integer_array( current_mns );
    finalst = allocate_integer_array( current_mns );
    transchar = allocate_integer_array( current_mns );
    trans1 = allocate_integer_array( current_mns );
    trans2 = allocate_integer_array( current_mns );
    accptnum = allocate_integer_array( current_mns );

    current_max_scs = INITIAL_MAX_SCS;
    scset = allocate_integer_array( current_max_scs );
    scbol = allocate_integer_array( current_max_scs );
    scxclu = allocate_integer_array( current_max_scs );
    actvsc = allocate_integer_array( current_max_scs );

    current_maxccls = INITIAL_MAXCCLS;
    cclmap = allocate_integer_array( current_maxccls );
    ccllen = allocate_integer_array( current_maxccls );
    cclng = allocate_integer_array( current_maxccls );

    current_max_ccl_tbl_size = INITIAL_MAX_CCL_TBL_SIZE;
    ccltbl = allocate_character_array( current_max_ccl_tbl_size );

    current_max_dfa_size = INITIAL_MAX_DFA_SIZE;

    current_max_xpairs = INITIAL_MAX_XPAIRS;
    nxt = allocate_integer_array( current_max_xpairs );
    chk = allocate_integer_array( current_max_xpairs );

    current_max_template_xpairs = INITIAL_MAX_TEMPLATE_XPAIRS;
    tnxt = allocate_integer_array( current_max_template_xpairs );

    current_max_dfas = INITIAL_MAX_DFAS;
    base = allocate_integer_array( current_max_dfas );
    def = allocate_integer_array( current_max_dfas );
    dfasiz = allocate_integer_array( current_max_dfas );
    accsiz = allocate_integer_array( current_max_dfas );
    dhash = allocate_integer_array( current_max_dfas );
    todo = allocate_integer_array( current_max_dfas );
    dss = allocate_integer_pointer_array( current_max_dfas );
    dfaacc = allocate_dfaacc_union( current_max_dfas );
    }
