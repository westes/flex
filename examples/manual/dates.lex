/*
 * dates.lex: An example of using start states to
 *            distinguish between different date formats.
 */

%{
#include <ctype.h>

char month[20],dow[20],day[20],year[20];

%}

skip        of|the|[ \t,]*

mon	    (mon(day)?)
tue	    (tue(sday)?)
wed	    (wed(nesday)?)
thu	    (thu(rsday)?)
fri	    (fri(day)?)
sat	    (sat(urday)?)
sun	    (sun(day)?)

day_of_the_week	({mon}|{tue}|{wed}|{thu}|{fri}|{sat}|{sun})

jan	    (jan(uary)?)
feb	    (feb(ruary)?)
mar	    (mar(ch)?)
apr	    (apr(il)?)
may	    (may)
jun	    (jun(e)?)
jul	    (jul(y)?)
aug	    (aug(ust)?)
sep	    (sep(tember)?)
oct	    (oct(ober)?)
nov	    (nov(ember)?)
dec	    (dec(ember)?)

first_half  ({jan}|{feb}|{mar}|{apr}|{may}|{jun})
second_half ({jul}|{aug}|{sep}|{oct}|{nov}|{dec})
month 	    {first_half}|{second_half}

nday         [1-9]|[1-2][0-9]|3[0-1]
nmonth       [1-9]|1[0-2]
nyear        [0-9]{1,4}

year_ext    (ad|AD|bc|BC)?
day_ext     (st|nd|rd|th)?

%s LONG SHORT
%s DAY DAY_FIRST YEAR_FIRST YEAR_LAST YFMONTH YLMONTH

%%

  /* the default is month-day-year */

<LONG>{day_of_the_week}    strcpy(dow,yytext); 
<LONG>{month}              strcpy(month,yytext); BEGIN(DAY);

  /* handle the form: day-month-year */

<LONG>{nday}{day_ext}      strcpy(day,yytext);   BEGIN(DAY_FIRST);
<DAY_FIRST>{month}         strcpy(month,yytext); BEGIN(LONG);
<DAY>{nday}{day_ext}       strcpy(day,yytext);   BEGIN(LONG);  

<LONG>{nyear}{year_ext}  {
                           printf("Long:\n");
                           printf("  DOW   : %s \n",dow);
                           printf("  Day   : %s \n",day);
                           printf("  Month : %s \n",month);
                           printf("  Year  : %s \n",yytext);
                           strcpy(dow,"");
                           strcpy(day,"");
                           strcpy(month,"");
                         }

  /* handle dates of the form: day-month-year */

<SHORT>{nday}              strcpy(day,yytext);  BEGIN(YEAR_LAST);
<YEAR_LAST>{nmonth}        strcpy(month,yytext);BEGIN(YLMONTH);
<YLMONTH>{nyear}           strcpy(year,yytext); BEGIN(SHORT);

  /* handle dates of the form: year-month-day */

<SHORT>{nyear}             strcpy(year,yytext); BEGIN(YEAR_FIRST);
<YEAR_FIRST>{nmonth}       strcpy(month,yytext);BEGIN(YFMONTH);
<YFMONTH>{nday}            strcpy(day,yytext);  BEGIN(SHORT);


<SHORT>\n                {
                           printf("Short:\n");
                           printf("  Day   : %s \n",day);
                           printf("  Month : %s \n",month);
                           printf("  Year  : %s \n",year);
                           strcpy(year,""); 
                           strcpy(day,"");
                           strcpy(month,"");
                         }

long\n                      BEGIN(LONG);
short\n                     BEGIN(SHORT);

{skip}*
\n
.


