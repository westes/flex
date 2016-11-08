%option 8bit noyywrap
%%
.|\n	{ ECHO;
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
