%{

/*
 * wc.lex : A simple example of using FLEX
 *          to create a wc-like utility.
 *
 *	    See MISC/fastwc/ in the flex distribution for examples
 *	    of how to write this scanner for maximum performance.
 */   

int  numchars = 0;
int  numwords = 0;
int  numlines = 0;
int  totchars = 0;
int  totwords = 0;
int  totlines = 0;

/*
 * rules start from here
 */

%}

%%

[\n]        { numchars++;  numlines++;         }
[\r]        { numchars++;                      }
[^ \t\n]+   { numwords++;  numchars += yyleng; }
.           { numchars++;                      }

%%

/*
 * additional C code start from here. This supplies
 * all the argument processing etc.
 */

int main(int argc, char *argv[])
{
  int  loop,first=1;
  int  lflag = 0; /* 1 if we count # of lines      */
  int  wflag = 0; /* 1 if we count # of words      */
  int  cflag = 0; /* 1 if we count # of characters */
  int  fflag = 0; /* 1 if we have a file name      */

  for(loop=1; loop<argc; loop++){
     if(argv[loop][0] == '-'){
	switch(argv[loop][1]){
	case 'l':
	   lflag = 1;
	   break;
	case 'w':
	   wflag = 1;
	   break;
	case 'c':
	   cflag = 1;
	   break;
	default:
	   fprintf(stderr,"unknown option -%c\n",
                   argv[loop][1]);
	}
     }
  }
  if(lflag == 0 && wflag == 0 && cflag == 0){
    lflag = wflag = cflag = 1; /* default to all on */
  }

  for(loop=1; loop<argc; loop++){
    if(argv[loop][0] != '-'){
      fflag = 1;
      numlines = numchars = numwords = 0;  
      if((yyin = fopen(argv[loop],"rb")) != NULL){
        if(first){
          first = 0;
	} else {
          YY_NEW_FILE;
	}
        (void) yylex();
        fclose(yyin);
        totwords += numwords;
        totchars += numchars;
        totlines += numlines;
        printf("file  : %25s :",argv[loop]) ;
        if(lflag){
          fprintf(stdout,"lines %5d ",numlines); 
        }
        if(cflag){
          fprintf(stdout,"characters %5d ",numchars); 
        }
        if(wflag){
          fprintf(stdout,"words %5d ",numwords); 
        }
        fprintf(stdout,"\n");
      }else{
        fprintf(stderr,"wc : file not found %s\n",argv[loop]);
      } 
    }
  }
  if(!fflag){
    fprintf(stderr,"usage : wc [-l -w -c] file [file...]\n");
    fprintf(stderr,"-l = count lines\n");
    fprintf(stderr,"-c = count characters\n");
    fprintf(stderr,"-w = count words\n");
    exit(1);
  }
  for(loop=0;loop<79; loop++){
    fprintf(stdout,"-");
  }
  fprintf(stdout,"\n");
  fprintf(stdout,"total : %25s  ","") ;
  if(lflag){
    fprintf(stdout,"lines %5d ",totlines); 
  }
  if(cflag){
    fprintf(stdout,"characters %5d ",totchars); 
  }
  if(wflag){
     fprintf(stdout,"words %5d ",totwords); 
  }
  fprintf(stdout,"\n");
  return(0);
}
