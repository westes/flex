/*
 * numbers.lex : An example of the definitions and techniques
 *               for scanning numbers
 */

%{
#include <stdio.h>

#define UNSIGNED_LONG_SYM   1
#define SIGNED_LONG_SYM     2
#define UNSIGNED_SYM        3
#define SIGNED_SYM          4
#define LONG_DOUBLE_SYM     5
#define FLOAT_SYM           6

union _yylval {
  long double    ylong_double;
  float          yfloat; 
  unsigned long  yunsigned_long;
  unsigned       yunsigned;
  long           ysigned_long;
  int            ysigned;
} yylval;

%}

digit             [0-9]
hex_digit         [0-9a-fA-F]
oct_digit         [0-7]

exponent          [eE][+-]?{digit}+
i                 {digit}+
float_constant    ({i}\.{i}?|{i}?\.{i}){exponent}?
hex_constant      0[xX]{hex_digit}+
oct_constant      0{oct_digit}*
int_constant      {digit}+
long_ext          [lL]
unsigned_ext      [uU]
float_ext         [fF]
ulong_ext         {long_ext}{unsigned_ext}|{unsigned_ext}{long_ext}

%%

{hex_constant}{ulong_ext} {  /* we need to skip the "0x" part */
                             sscanf(&yytext[2],"%lx",&yylval.yunsigned_long); 
                             return(UNSIGNED_LONG_SYM);
                          }
{hex_constant}{long_ext}  {  
                             sscanf(&yytext[2],"%lx",&yylval.ysigned_long); 
                             return(SIGNED_LONG_SYM);
                          }
{hex_constant}{unsigned_ext}  { 
                             sscanf(&yytext[2],"%x",&yylval.yunsigned); 
                             return(UNSIGNED_SYM);
                          }
{hex_constant}            { /* use %lx to protect against overflow */
                             sscanf(&yytext[2],"%lx",&yylval.ysigned_long); 
                             return(SIGNED_LONG_SYM);
                          }
{oct_constant}{ulong_ext} {
                             sscanf(yytext,"%lo",&yylval.yunsigned_long); 
                             return(UNSIGNED_LONG_SYM);
                          }
{oct_constant}{long_ext}  {
                             sscanf(yytext,"%lo",&yylval.ysigned_long); 
                             return(SIGNED_LONG_SYM);
                          }
{oct_constant}{unsigned_ext}  {
                             sscanf(yytext,"%o",&yylval.yunsigned); 
                             return(UNSIGNED_SYM);
                          }
{oct_constant}            { /* use %lo to protect against overflow */
                             sscanf(yytext,"%lo",&yylval.ysigned_long); 
                             return(SIGNED_LONG_SYM);
                          }
{int_constant}{ulong_ext} {
                             sscanf(yytext,"%ld",&yylval.yunsigned_long); 
                             return(UNSIGNED_LONG_SYM);
                          }
{int_constant}{long_ext}  {
                             sscanf(yytext,"%ld",&yylval.ysigned_long); 
                             return(SIGNED_LONG_SYM);
                          }
{int_constant}{unsigned_ext}  {
                             sscanf(yytext,"%d",&yylval.yunsigned); 
                             return(UNSIGNED_SYM);
                          }
{int_constant}            { /* use %ld to protect against overflow */
                             sscanf(yytext,"%ld",&yylval.ysigned_long); 
                             return(SIGNED_LONG_SYM);
                          }
{float_constant}{long_ext}  {
                             sscanf(yytext,"%lf",&yylval.ylong_double); 
                             return(LONG_DOUBLE_SYM);
                          }
{float_constant}{float_ext}  {
                             sscanf(yytext,"%f",&yylval.yfloat); 
                             return(FLOAT_SYM);
                          }
{float_constant}          { /* use %lf to protect against overflow */
                             sscanf(yytext,"%lf",&yylval.ylong_double); 
                             return(LONG_DOUBLE_SYM);
                          }
%%

int main(void)
{
  int code;

  while((code = yylex())){
    printf("yytext          : %s\n",yytext);
    switch(code){
    case UNSIGNED_LONG_SYM:
       printf("Type of number  : UNSIGNED LONG\n");
       printf("Value of number : %lu\n",yylval.yunsigned_long);
       break;
    case SIGNED_LONG_SYM:  
       printf("Type of number  : SIGNED LONG\n");
       printf("Value of number : %ld\n",yylval.ysigned_long);
       break;
    case UNSIGNED_SYM:     
       printf("Type of number  : UNSIGNED\n");
       printf("Value of number : %u\n",yylval.yunsigned);
       break;
    case SIGNED_SYM:       
       printf("Type of number  : SIGNED\n");
       printf("Value of number : %d\n",yylval.ysigned);
       break;
    case LONG_DOUBLE_SYM:  
       printf("Type of number  : LONG DOUBLE\n");
       printf("Value of number : %lf\n",yylval.ylong_double);
       break;
    case FLOAT_SYM:        
       printf("Type of number  : FLOAT\n");
       printf("Value of number : %f\n",yylval.yfloat);
       break;
    default:
       printf("Type of number  : UNDEFINED\n");
       printf("Value of number : UNDEFINED\n");
       break;
    }
  }
  return(0);
}

