/* sym - symbol table routines */

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

#include "flexdef.h"

struct hash_entry *ndtbl[NAME_TABLE_HASH_SIZE];
struct hash_entry *sctbl[START_COND_HASH_SIZE];
struct hash_entry *ccltab[CCL_HASH_SIZE];

struct hash_entry *findsym();


/* addsym - add symbol and definitions to symbol table
 *
 * synopsis
 *    char sym[], *str_def;
 *    int int_def;
 *    hash_table table;
 *    int table_size;
 *    0 / -1 = addsym( sym, def, int_def, table, table_size );
 *
 * -1 is returned if the symbol already exists, and the change not made.
 */

int addsym( sym, str_def, int_def, table, table_size )
register char sym[];
char *str_def;
int int_def;
hash_table table;
int table_size;

    {
    int hash_val = hashfunct( sym, table_size );
    register struct hash_entry *entry = table[hash_val];
    register struct hash_entry *new_entry;
    register struct hash_entry *successor;
    char *malloc();

    while ( entry )
	{
	if ( ! strcmp( sym, entry->name ) )
	    { /* entry already exists */
	    return ( -1 );
	    }
	
	entry = entry->next;
	}

    /* create new entry */
    new_entry = (struct hash_entry *) malloc( sizeof( struct hash_entry ) );

    if ( new_entry == NULL )
	flexfatal( "symbol table memory allocation failed" );

    if ( (successor = table[hash_val]) )
	{
	new_entry->next = successor;
	successor->prev = new_entry;
	}
    else
	new_entry->next = NULL;

    new_entry->prev = NULL;
    new_entry->name = sym;
    new_entry->str_val = str_def;
    new_entry->int_val = int_def;

    table[hash_val] = new_entry;

    return ( 0 );
    }


/* cclinstal - save the text of a character class
 *
 * synopsis
 *    char ccltxt[];
 *    int cclnum;
 *    cclinstal( ccltxt, cclnum );
 */

cclinstal( ccltxt, cclnum )
char ccltxt[];
int cclnum;

    {
    /* we don't bother checking the return status because we are not called
     * unless the symbol is new
     */
    char *copy_string();

    (void) addsym( copy_string( ccltxt ), (char *) 0, cclnum,
		   ccltab, CCL_HASH_SIZE );
    }


/* ccllookup - lookup the number associated with character class text
 *
 * synopsis
 *    char ccltxt[];
 *    int ccllookup, cclval;
 *    cclval/0 = ccllookup( ccltxt );
 */

int ccllookup( ccltxt )
char ccltxt[];

    {
    return ( findsym( ccltxt, ccltab, CCL_HASH_SIZE )->int_val );
    }


/* findsym - find symbol in symbol table
 *
 * synopsis
 *    char sym[];
 *    hash_table table;
 *    int table_size;
 *    struct hash_entry *entry, *findsym();
 *    entry = findsym( sym, table, table_size );
 */

struct hash_entry *findsym( sym, table, table_size )
register char sym[];
hash_table table;
int table_size;

    {
    register struct hash_entry *entry = table[hashfunct( sym, table_size )];
    static struct hash_entry empty_entry =
	{
	(struct hash_entry *) 0, (struct hash_entry *) 0, NULL, NULL, 0,
	} ;

    while ( entry )
	{
	if ( ! strcmp( sym, entry->name ) )
	    return ( entry );
	entry = entry->next;
	}

    return ( &empty_entry );
    }

    
/* hashfunct - compute the hash value for "str" and hash size "hash_size"
 *
 * synopsis
 *    char str[];
 *    int hash_size, hash_val;
 *    hash_val = hashfunct( str, hash_size );
 */

int hashfunct( str, hash_size )
register char str[];
int hash_size;

    {
    register int hashval;
    register int locstr;

    hashval = 0;
    locstr = 0;

    while ( str[locstr] )
	hashval = ((hashval << 1) + str[locstr++]) % hash_size;

    return ( hashval );
    }


/* ndinstal - install a name definition
 *
 * synopsis
 *    char nd[], def[];
 *    ndinstal( nd, def );
 */

ndinstal( nd, def )
char nd[], def[];

    {
    char *copy_string();

    if ( addsym( copy_string( nd ), copy_string( def ), 0,
		 ndtbl, NAME_TABLE_HASH_SIZE ) )
	synerr( "name defined twice" );
    }


/* ndlookup - lookup a name definition
 *
 * synopsis
 *    char nd[], *def;
 *    char *ndlookup();
 *    def/NULL = ndlookup( nd );
 */

char *ndlookup( nd )
char nd[];

    {
    return ( findsym( nd, ndtbl, NAME_TABLE_HASH_SIZE )->str_val );
    }


/* scinstal - make a start condition
 *
 * synopsis
 *    char str[];
 *    int xcluflg;
 *    scinstal( str, xcluflg );
 *
 * NOTE
 *    the start condition is Exclusive if xcluflg is true
 */

scinstal( str, xcluflg )
char str[];
int xcluflg;

    {
    char *copy_string();

    /* bit of a hack.  We know how the default start-condition is
     * declared, and don't put out a define for it, because it
     * would come out as "#define 0 1"
     */
    /* actually, this is no longer the case.  The default start-condition
     * is now called "INITIAL".  But we keep the following for the sake
     * of future robustness.
     */

    if ( strcmp( str, "0" ) )
	printf( "#define %s %d\n", str, lastsc * 2 );

    if ( ++lastsc >= current_max_scs )
	{
	current_max_scs += MAX_SCS_INCREMENT;

	++num_reallocs;

	scset = reallocate_integer_array( scset, current_max_scs );
	scbol = reallocate_integer_array( scbol, current_max_scs );
	scxclu = reallocate_integer_array( scxclu, current_max_scs );
	actvsc = reallocate_integer_array( actvsc, current_max_scs );
	}

    if ( addsym( copy_string( str ), (char *) 0, lastsc,
	 sctbl, START_COND_HASH_SIZE ) )
	lerrsf( "start condition %s declared twice", str );

    scset[lastsc] = mkstate( SYM_EPSILON );
    scbol[lastsc] = mkstate( SYM_EPSILON );
    scxclu[lastsc] = xcluflg;
    }


/* sclookup - lookup the number associated with a start condition
 *
 * synopsis
 *    char str[], scnum;
 *    int sclookup;
 *    scnum/0 = sclookup( str );
 */

int sclookup( str )
char str[];

    {
    return ( findsym( str, sctbl, START_COND_HASH_SIZE )->int_val );
    }
