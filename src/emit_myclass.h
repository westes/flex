/**
 * Function defintions for emit_myclass. I'm just glomming this on to
 * flex, so it's probably not going to follow the original coding
 * convetions. Sorry about that.
 * BMI 12/03/2014
 */

#ifndef _H_EMIT_MYCLASS
#define _H_EMIT_MYCLASS

// If classname.yy.h does not currently exist, it will create it,
// include FlexLexer.h, and derive myclass from FlexLexer.h
//
// Note that in this scenario, yyFlexLexer will remain a pure
// virtual function because I'm not definining yylex for it.
// So you'll get a compiler error if you try to compile anything
// that makes a direct instance of it. At the same time, you should
// be able to pass pointeres and references to any of your scanners
// using yyFlexLexer as a base class.

void emit_myclass(const char * const classname);

#endif
