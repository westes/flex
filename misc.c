/* misc - miscellaneous flex routines */

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

#ifndef lint

static char copyright[] =
    "@(#) Copyright (c) 1989 The Regents of the University of California.\n";
static char CR_continuation[] = "@(#) All rights reserved.\n";

static char rcsid[] =
    "@(#) $Header$ (LBL)";

#endif

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
    register char *mem;

    /* on 16-bit int machines (e.g., 80286) we might be trying to
     * allocate more than a signed int can hold, and that won't
     * work.  Cheap test:
     */
    if ( element_size * size <= 0 )
        flexfatal( "request for < 1 byte in allocate_array()" );
    
    mem = malloc( (unsigned) (element_size * size) );

    if ( mem == NULL )
	flexfatal( "memory allocation failed in allocate_array()" );

    return ( mem );
    }


/* all_lower - true if a string is all lower-case
 *
 * synopsis:
 *    char *str;
 *    int all_lower();
 *    true/false = all_lower( str );
 */

int all_lower( str )
register char *str;

    {
    while ( *str )
	{
	if ( ! islower( *str ) )
	    return ( 0 );
	++str;
	}

    return ( 1 );
    }


/* all_upper - true if a string is all upper-case
 *
 * synopsis:
 *    char *str;
 *    int all_upper();
 *    true/false = all_upper( str );
 */

int all_upper( str )
register char *str;

    {
    while ( *str )
	{
	if ( ! isupper( *str ) )
	    return ( 0 );
	++str;
	}

    return ( 1 );
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

/* flex_gettime - return current time
 *
 * synopsis
 *    char *flex_gettime(), *time_str;
 *    time_str = flex_gettime();
 *
 * note
 *    the routine name has the "flex_" prefix because of name clashes
 *    with Turbo-C
 */

/* include sys/types.h to use time_t and make lint happy */

#ifndef MS_DOS
#ifndef VMS
#include <sys/types.h>
#else
#include <types.h>
#endif
#endif

#ifdef MS_DOS
#include <time.h>
typedef long time_t;
#endif

char *flex_gettime()

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
	case 'a': return ( '\a' );
	case 'b': return ( '\b' );
	case 'f': return ( '\f' );
	case 'n': return ( '\n' );
	case 'r': return ( '\r' );
	case 't': return ( '\t' );
	case 'v': return ( '\v' );

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	    { /* \<octal> */
	    char c, esc_char;
	    register int sptr = 1;

	    while ( isdigit(array[sptr]) )
		/* don't increment inside loop control because if
		 * isdigit() is a macro it will expand it to two
		 * increments ...
		 */
		++sptr;

	    c = array[sptr];
	    array[sptr] = '\0';

	    esc_char = otoi( array + 1 );
	    array[sptr] = c;

	    if ( esc_char == '\0' )
		{
		synerr( "escape sequence for null not allowed" );
		return ( 1 );
		}

	    return ( esc_char );
	    }

	default:
	    return ( array[1] );
	}
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
    int result;

    (void) sscanf( str, "%o", &result );

    return ( result );
    }


/* readable_form - return the the human-readable form of a character
 *
 * synopsis:
 *    int c;
 *    char *readable_form();
 *    <string> = readable_form( c );
 *
 * The returned string is in static storage.
 */

char *readable_form( c )
register int c;

    {
    static char rform[10];

    if ( (c >= 0 && c < 32) || c == 127 )
	{
	switch ( c )
	    {
	    case '\n': return ( "\\n" );
	    case '\t': return ( "\\t" );
	    case '\f': return ( "\\f" );
	    case '\r': return ( "\\r" );
	    case '\b': return ( "\\b" );

	    default:
		sprintf( rform, "\\%.3o", c );
		return ( rform );
	    }
	}
    
    else if ( c == ' ' )
	return ( "' '" );
    
    else
	{
	rform[0] = c;
	rform[1] = '\0';

	return ( rform );
	}
    }


/* reallocate_array - increase the size of a dynamic array */

char *reallocate_array( array, size, element_size )
char *array;
int size, element_size;

    {
    register char *new_array;

    /* same worry as in allocate_array(): */
    if ( size * element_size <= 0 )
        flexfatal( "attempt to increase array size by less than 1 byte" );
    
    new_array = realloc( array, (unsigned) (size * element_size ));

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
	putchar( '\n' );

	if ( ++dataline % 10 == 0 )
	    putchar( '\n' );

	datapos = 0;
	}
    }
