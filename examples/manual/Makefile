#############################################################
#
# Makefile     : Makefile for Flex examples.
# Author       : G.T.Nicol
# Last Updated : 1993/10/05
#
# If you use bison, you may have to supply an alloca
#
#############################################################

CC       = gcc -g
LEX      = flex -i -I 
YACC     = bison -d -y
ALLOCA   =

############################################################
#
# DO NOT CHANGE ANYTHING FROM HERE ON !!!!!!!!! 
#
############################################################

all: expr front myname eof wc replace user_act string1\
     string2 yymore numbers dates cat

expr:   expr.y expr.lex
	$(YACC) expr.y
	$(LEX)  expr.lex
	$(CC) -o expr lex.yy.c y.tab.c $(ALLOCA) -ll -lm

front:  front.y front.lex
	$(YACC) front.y
	$(LEX)  front.lex
	$(CC) -o front lex.yy.c y.tab.c $(ALLOCA) -ll -lm

numbers: numbers.lex
	$(LEX)  numbers.lex
	$(CC)   lex.yy.c -o numbers

dates: dates.lex
	$(LEX)  dates.lex
	$(CC)   lex.yy.c -o dates -ll

yymore: yymore.lex
	$(LEX)  yymore.lex
	$(CC)   lex.yy.c -o yymore -ll

string1: string1.lex
	$(LEX)  string1.lex
	$(CC)   lex.yy.c -o string1 -ll

string2: string2.lex
	$(LEX)  string2.lex
	$(CC)   lex.yy.c -o string2 -ll

myname: myname.lex
	$(LEX)  myname.lex
	$(CC)   lex.yy.c -o myname -ll

myname2: myname2.lex
	$(LEX)  myname2.lex
	$(CC)   lex.yy.c -o myname2 -ll

eof:    eof_rules.lex
	$(LEX)  eof_rules.lex
	$(CC)   lex.yy.c -o eof -ll

wc:     wc.lex
	$(LEX)  wc.lex
	$(CC)   lex.yy.c -o wc -ll

cat:    cat.lex
	$(LEX)  cat.lex
	$(CC)   lex.yy.c -o cat -ll

replace: replace.lex
	$(LEX)  replace.lex
	$(CC)   lex.yy.c -o replace -ll

user_act: expr.y expr.lex
	$(LEX)  user_act.lex
	$(CC) -o user_act lex.yy.c -ll 

clean:
	rm -f *.BAK *.o core *~* *.a 
	rm -f *.tab.h *.tab.c
	rm -f myname expr lex.yy.c *.out eof wc yymore
	rm -f replace front user_act string1 string2
	rm -f dates numbers cat
