/* misc - miscellaneous flex routines */

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

#include <ctype.h>
#include "flexdef.h"

char *malloc(), *realloc();


/* action_out - write the actions from the temporary file to lex.yy.c
 *
 * synopsis
 *     action_out();
 *
 *     Copies the action file up to %% (or end-of-file) to lex.yy.c
 */

action_out()

    {
    char buf[MAXLINE];

    while ( fgets( buf, MAXLINE, temp_action_file ) != NULL )
	if ( buf[0] == '%' && buf[1] == '%' )
	    break;
	else
	    fputs( buf, stdout );
    }


/* allocate_array - allocate memory for an integer array of the given size */

char *allocate_array( size, element_size )
int size, element_size;

    {
    register char *mem = malloc( (unsigned) (element_size * size) );

    if ( mem == NULL )
	flexfatal( "memory allocation failed in allocate_array()" );

    return ( mem );
    }


/* bubble - bubble sort an integer array in increasing order
 *
 * synopsis
 *   int v[n], n;
 *   bubble( v, n );
 *
 * description
 *   sorts the first n elements of array v and replaces them in
 *   increasing order.
 *
 * passed
 *   v - the array to be sorted
 *   n - the number of elements of 'v' to be sorted */

bubble( v, n )
int v[], n;

    {
    register int i, j, k;

    for ( i = n; i > 1; --i )
	for ( j = 1; j < i; ++j )
	    if ( v[j] > v[j + 1] )	/* compare */
		{
		k = v[j];	/* exchange */
		v[j] = v[j + 1];
		v[j + 1] = k;
		}
    }


/* clower - replace upper-case letter to lower-case
 *
 * synopsis:
 *    char clower(), c;
 *    c = clower( c );
 */

char clower( c )
register char c;

    {
    return ( isupper(c) ? tolower(c) : c );
    }


/* copy_string - returns a dynamically allocated copy of a string
 *
 * synopsis
 *    char *str, *copy, *copy_string();
 *    copy = copy_string( str );
 */

char *copy_string( str )
register char *str;

    {
    register char *c;
    char *copy;

    /* find length */
    for ( c = str; *c; ++c )
	;

    copy = malloc( (unsigned) ((c - str + 1) * sizeof( char )) );

    if ( copy == NULL )
	flexfatal( "dynamic memory failure in copy_string()" );

    for ( c = copy; (*c++ = *str++); )
	;
    
    return ( copy );
    }


/* cshell - shell sort a character array in increasing order
 *
 * synopsis
 *
 *   char v[n];
 *   int n;
 *   cshell( v, n );
 *
 * description
 *   does a shell sort of the first n elements of array v.
 *
 * passed
 *   v - array to be sorted
 *   n - number of elements of v to be sorted
 */
cshell( v, n )
char v[];
int n;

    {
    int gap, i, j, jg;
    char k;

    for ( gap = n / 2; gap > 0; gap = gap / 2 )
	for ( i = gap; i < n; ++i )
	    for ( j = i - gap; j >= 0; j = j - gap )
		{
		jg = j + gap;

		if ( v[j] <= v[jg] )
		    break;

		k = v[j];
		v[j] = v[jg];
		v[jg] = k;
		}
    }


/* dataend - finish up a block of data declarations
 *
 * synopsis
 *    dataend();
 */
dataend()

    {
    if ( datapos > 0 )
	dataflush();

    /* add terminator for initialization */
    puts( "    } ;\n" );

    dataline = 0;
    }



/* dataflush - flush generated data statements
 *
 * synopsis
 *    dataflush();
 */
dataflush()

    {
    putchar( '\n' );

    if ( ++dataline >= NUMDATALINES )
	{
	/* put out a blank line so that the table is grouped into
	 * large blocks that enable the user to find elements easily
	 */
	putchar( '\n' );
	dataline = 0;
	}

    /* reset the number of characters written on the current line */
    datapos = 0;
    }

/* gettime - return current time
 *
 * synopsis
 *    char *gettime(), *time_str;
 *    time_str = gettime();
 */

/* include sys/types.h to use time_t and make lint happy */

#include <sys/types.h>

char *gettime()

    {
    time_t t, time();
    char *result, *ctime(), *copy_string();

    t = time( (long *) 0 );

    result = copy_string( ctime( &t ) );

    /* get rid of trailing newline */
    result[24] = '\0';

    return ( result );
    }


/* lerrif - report an error message formatted with one integer argument
 *
 * synopsis
 *    char msg[];
 *    int arg;
 *    lerrif( msg, arg );
 */

lerrif( msg, arg )
char msg[];
int arg;

    {
    char errmsg[MAXLINE];
    (void) sprintf( errmsg, msg, arg );
    flexerror( errmsg );
    }


/* lerrsf - report an error message formatted with one string argument
 *
 * synopsis
 *    char msg[], arg[];
 *    lerrsf( msg, arg );
 */

lerrsf( msg, arg )
char msg[], arg[];

    {
    char errmsg[MAXLINE];

    (void) sprintf( errmsg, msg, arg );
    flexerror( errmsg );
    }


/* flexerror - report an error message and terminate
 *
 * synopsis
 *    char msg[];
 *    flexerror( msg );
 */

flexerror( msg )
char msg[];

    {
    fprintf( stderr, "flex: %s\n", msg );
    flexend( 1 );
    }


/* flexfatal - report a fatal error message and terminate
 *
 * synopsis
 *    char msg[];
 *    flexfatal( msg );
 */

flexfatal( msg )
char msg[];

    {
    fprintf( stderr, "flex: fatal internal error %s\n", msg );
    flexend( 1 );
    }


/* line_directive_out - spit out a "# line" statement */

line_directive_out( output_file_name )
FILE *output_file_name;

    {
    if ( infilename && gen_line_dirs ) 
        fprintf( output_file_name, "# line %d \"%s\"\n", linenum, infilename );
    }


/* mk2data - generate a data statement for a two-dimensional array
 *
 * synopsis
 *    int value;
 *    mk2data( value );
 *
 *  generates a data statement initializing the current 2-D array to "value"
 */
mk2data( value )
int value;

    {
    if ( datapos >= NUMDATAITEMS )
	{
	putchar( ',' );
	dataflush();
	}

    if ( datapos == 0 )
	/* indent */
	fputs( "    ", stdout );

    else
	putchar( ',' );

    ++datapos;

    printf( "%5d", value );
    }


/* mkdata - generate a data statement
 *
 * synopsis
 *    int value;
 *    mkdata( value );
 *
 *  generates a data statement initializing the current array element to
 *  "value"
 */
mkdata( value )
int value;

    {
    if ( datapos >= NUMDATAITEMS )
	{
	putchar( ',' );
	dataflush();
	}

    if ( datapos == 0 )
	/* indent */
	fputs( "    ", stdout );

    else
	putchar( ',' );

    ++datapos;

    printf( "%5d", value );
    }


/* myctoi - return the integer represented by a string of digits
 *
 * synopsis
 *    char array[];
 *    int val, myctoi();
 *    val = myctoi( array );
 *
 */

int myctoi( array )
char array[];

    {
    int val = 0;

    (void) sscanf( array, "%d", &val );

    return ( val );
    }


/* myesc - return character corresponding to escape sequence
 *
 * synopsis
 *    char array[], c, myesc();
 *    c = myesc( array );
 *
 */

char myesc( array )
char array[];

    {
    switch ( array[1] )
	{
	case 'n': return ( '\n' );
	case 't': return ( '\t' );
	case 'f': return ( '\f' );
	case 'r': return ( '\r' );
	case 'b': return ( '\b' );

	case '0':
	    if ( isdigit(array[2]) )
		{ /* \0<octal> */
		char c, esc_char;
		register int sptr = 2;

		while ( isdigit(array[sptr]) )
		    /* don't increment inside loop control because the
		     * macro will expand it to two increments!  (Not a
		     * problem with the C version of the macro)
		     */
		    ++sptr;

		c = array[sptr];
		array[sptr] = '\0';

		esc_char = otoi( array + 2 );
		array[sptr] = c;

		if ( esc_char == '\0' )
		    {
		    synerr( "escape sequence for null not allowed" );
		    return ( 1 );
		    }

		return ( esc_char );
		}

	    else
		{
		synerr( "escape sequence for null not allowed" );
		return ( 1 );
		}

#ifdef NOTDEF
	case '^':
	    {
	    register char next_char = array[2];

	    if ( next_char == '?' )
		return ( 0x7f );
	    
	    else if ( next_char >= 'A' && next_char <= 'Z' )
		return ( next_char - 'A' + 1 );
    
	    else if ( next_char >= 'a' && next_char <= 'z' )
		return ( next_char - 'z' + 1 );
    
	    synerr( "illegal \\^ escape sequence" );

	    return ( 1 );
	    }
#endif
	}
    
    return ( array[1] );
    }


/* otoi - convert an octal digit string to an integer value
 *
 * synopsis:
 *    int val, otoi();
 *    char str[];
 *    val = otoi( str );
 */

int otoi( str )
char str[];

    {
#ifdef FTLSOURCE
    fortran int gctoi()
    int dummy = 1;

    return ( gctoi( str, dummy, 8 ) );
#else
    int result;

    (void) sscanf( str, "%o", &result );

    return ( result );
#endif
    }




/* reallocate_array - increase the size of a dynamic array */

char *reallocate_array( array, size, element_size )
char *array;
int size, element_size;

    {
    register char *new_array = realloc( array,
					(unsigned) (size * element_size ));

    if ( new_array == NULL )
	flexfatal( "attempt to increase array size failed" );
    
    return ( new_array );
    }


/* skelout - write out one section of the skeleton file
 *
 * synopsis
 *    skelout();
 *
 * DESCRIPTION
 *    Copies from skelfile to stdout until a line beginning with "%%" or
 *    EOF is found.
 */
skelout()

    {
    char buf[MAXLINE];

    while ( fgets( buf, MAXLINE, skelfile ) != NULL )
	if ( buf[0] == '%' && buf[1] == '%' )
	    break;
	else
	    fputs( buf, stdout );
    }


/* transition_struct_out - output a yy_trans_info structure
 *
 * synopsis
 *     int element_v, element_n;
 *     transition_struct_out( element_v, element_n );
 *
 * outputs the yy_trans_info structure with the two elements, element_v and
 * element_n.  Formats the output with spaces and carriage returns.
 */

transition_struct_out( element_v, element_n )
int element_v, element_n;

    {
    printf( "%7d, %5d,", element_v, element_n );

    datapos += TRANS_STRUCT_PRINT_LENGTH;

    if ( datapos >= 75 )
	{
	printf( "\n" );

	if ( ++dataline % 10 == 0 )
	    printf( "\n" );

	datapos = 0;
	}
    }
