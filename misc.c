/* lexmisc - miscellaneous flex routines */

/*
 * Copyright (c) University of California, 1987
 */

#include <ctype.h>
#include "flexdef.h"

char *malloc(), *realloc();


/* allocate_array - allocate memory for an integer array of the given size */

char *allocate_array( size, element_size )
int size, element_size;

    {
    register char *mem = malloc( (unsigned) (element_size * size) );

    if ( mem == NULL )
	lexfatal( "memory allocation failed in allocate_array()" );

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
	lexfatal( "dynamic memory failure in copy_string()" );

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

    if ( genftl )
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

    if ( genftl )
	{
	if ( ++dataline >= NUMDATALINES )
	    {
	    /* put out a blank line so that the table is grouped into
	     * large blocks that enable the user to find elements easily
	     */
	    putchar( '\n' );
	    dataline = 0;
	    }
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
    lexerror( errmsg );
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
    lexerror( errmsg );
    }


/* lexerror - report an error message and terminate
 *
 * synopsis
 *    char msg[];
 *    lexerror( msg );
 */

lexerror( msg )
char msg[];

    {
    fprintf( stderr, "flex: %s\n", msg );
    lexend( 1 );
    }


/* lexfatal - report a fatal error message and terminate
 *
 * synopsis
 *    char msg[];
 *    lexfatal( msg );
 */

lexfatal( msg )
char msg[];

    {
    fprintf( stderr, "flex: fatal internal error %s\n", msg );
    lexend( 1 );
    }


/* line_directive_out - spit out a "# line" statement */

line_directive_out()

    {
    if ( infilename ) 
        printf( "# line %d \"%s\"\n", linenum, infilename );
    }


/* mk2data - generate a data statement for a two-dimensional array
 *
 * synopsis
 *    char name;
 *    int row, column, value;
 *    mk2data( name, row, column, value );
 *
 *  generates a data statement initializing "name(row, column)" to "value"
 *  Note that name is only a character; NOT a string.  If we're generating
 *  FTL (-f flag), "name", "row", and "column" get ignored.
 */
mk2data( name, row, column, value )
char name;
int row, column, value;

    {
    int datalen;
    static char dindent[] = DATAINDENTSTR;

    if ( genftl )
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

    else
	{
	/* figure out length of data statement to be written.  7 is the constant
	 * overhead of a one character name, '(', ',',  and ')' to delimit
	 * the array reference, a '/' and a '/' to delimit the value, and
	 * room for a blank or a comma between this data statement and the
	 * previous one
	 */

	datalen = 7 + numdigs( row ) + numdigs( column ) + numdigs( value );

	if ( datalen + datapos >= DATALINEWIDTH | datapos == 0 )
	    {
	    if ( datapos != 0 )
		dataflush();

	    /* precede data statement with '%' so rat4 preprocessor doesn't have
	     * to bother looking at it -- speed hack
	     */
	    printf( "%%%sdata ", dindent );

	    /* 4 is the constant overhead of writing out the word "DATA" */
	    datapos = DATAINDENTWIDTH + 4 + datalen;
	    }

	else
	    {
	    putchar( ',' );
	    datapos = datapos + datalen;
	    }

	printf( "%c(%d,%d)/%d/", name, row, column, value );
	}
    }


/* mkdata - generate a data statement
 *
 * synopsis
 *    char name;
 *    int arrayelm, value;
 *    mkdata( name, arrayelm, value );
 *
 *  generates a data statement initializing "name(arrayelm)" to "value"
 *  Note that name is only a character; NOT a string.  If we're generating
 *  FTL (-f flag), "name" and "arrayelm" get ignored.
 */
mkdata( name, arrayelm, value )
char name;
int arrayelm, value;

    {
    int datalen;
    static char dindent[] = DATAINDENTSTR;

    if ( genftl )
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

    else
	{
	/* figure out length of data statement to be written.  6 is the constant
	 * overhead of a one character name, '(' and ')' to delimit the array
	 * reference, a '/' and a '/' to delimit the value, and room for a
	 * blank or a comma between this data statement and the previous one
	 */

	datalen = 6 + numdigs( arrayelm ) + numdigs( value );

	if ( datalen + datapos >= DATALINEWIDTH | datapos == 0 )
	    {
	    if ( datapos != 0 )
		dataflush();

	    /* precede data statement with '%' so rat4 preprocessor doesn't have
	     * to bother looking at it -- speed hack
	     */
	    printf( "%%%sdata ", dindent );

	    /* 4 is the constant overhead of writing out the word "DATA" */
	    datapos = DATAINDENTWIDTH + 4 + datalen;
	    }

	else
	    {
	    putchar( ',' );
	    datapos = datapos + datalen;
	    }

	printf( "%c(%d)/%d/", name, arrayelm, value );
	}
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


/* numdigs - number of digits (includes leading sign) in number
 *
 * synopsis
 *    int numdigs, x;
 *    num = numdigs( x );
 */
int numdigs( x )
int x;

    {
    if ( x < 0 )
	{
	/* the only negative numbers we expect to encounter are very
	 * small ones
	 */
	if ( x < -9 )
	    lexfatal( "assumption of small negative numbers botched in numdigs()" );

	return ( 2 );
	}

    if ( x < 10 )
	return ( 1 );
    else if ( x < 100 )
	return ( 2 );
    else if ( x < 1000 )
	return ( 3 );
    else if ( x < 10000 )
	return ( 4 );
    else if ( x < 100000 )
	return ( 5 );
    else
	return ( 6 );
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
	lexfatal( "attempt to increase array size failed" );
    
    return ( new_array );
    }


/* skelout - write out one section of the lexskel file
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
