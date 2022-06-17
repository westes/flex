/*
 * This file is part of flex.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the University nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.
 */
 
%{
/* A template scanner file to build "scanner.cc". 
   The scanner tests that we correctly manage the state of the buffer
   when the scanner called yyinput from user actions.
   
   This tests guards against reversions to Issue 525. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <config.h>

#define MAX_FILES 10
#define MAX_LENGTH 100
char files[MAX_FILES][MAX_LENGTH] = { 0 };
int filecounter = 0;

%}

%option 8bit prefix="test"
%option reentrant warn yylineno
%option nodefault nomain nostdinit nounput

%%

.                       { ECHO; 
                          char c;
                          for (unsigned int i = 0;
                                        i < 1000;
                                        ++i) {
                            c = yyinput (yyscanner);
                            if (c == 0) 
                              if (filecounter <= 0)
							    yyterminate();
                          } 
                        }
<<EOF>>                 { yyterminate(); }
%%

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

size_t readFile(const char* filename, char* buffer)
{
  size_t size, result;
  FILE *in; 
  
  in = fopen(filename, "rb");
  if (in)
  {
    fseek(in, 0, SEEK_END);
    size = (size_t)ftell(in);
	size = size>(MAX_LENGTH-2)?(MAX_LENGTH-2):size;
    rewind(in);
    result = fread(buffer, sizeof(char), size, in);
	fclose(in);
	if ( result != size) {
	  fprintf(stderr, "*** Error: fread(%s) failed.\n", filename);
      exit(-1);
    }
	else {
      buffer[size + 0] = '\0';
      buffer[size + 1] = '\0';
	}
  }
  else {
    fprintf(stderr, "*** Error: fopen(%s) failed.\n", filename);
    exit(-1);
  }
  
  return (size + 2);
}

int testwrap (yyscan_t yyscanner)
{   
  if ( --filecounter >= 0 ) {
    
	fprintf(stderr, "*** INFO: Wrapping to file %d.\n", filecounter);
    
    test_scan_buffer(files[filecounter], MAX_LENGTH, yyscanner);

    return 0;
  }
  return 1;
}

int main (int argc, char* argv[])
{
  yyscan_t scanner;

  if ( argc < 2) {
    printf("*** Error: Must specify at least one filename.\n");
	exit(-1);
  }
  for (int i = 1; i < argc && i <= MAX_FILES; i++) {
    readFile(argv[i], files[i-1]);
	++filecounter;
  }
 
  testlex_init(&scanner);
  if ( !testwrap(scanner) )
  {
    testlex(scanner);
  }
  testlex_destroy(scanner);

  
  return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */