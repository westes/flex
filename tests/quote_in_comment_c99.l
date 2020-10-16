%option emit="c99"
%option 8bit noyywrap
%%
.|\n	{ yyecho();
   //' "
  }
%%
int
main (void)
{
    yyin = stdin;
    yyout = stdout;
    while (yylex())
        ;
    printf("TEST RETURNING OK.\n");
    return 0;
}
