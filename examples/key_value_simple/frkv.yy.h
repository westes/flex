#ifndef _HPP_frkv
#define _HPP_frkv

/* Initially generated with flex --c++ --yyclass=frkv -o frkv.yy.cc frkv.l */

#include <FlexLexer.h>
#include <boost/signals2.hpp>

class frkv : public yyFlexLexer {
 protected:
  bool key; // Expecting key next
public:
 frkv(FLEX_STD istream* arg_yyin = 0, FLEX_STD ostream* arg_yyout = 0) : yyFlexLexer(arg_yyin, arg_yyout), key(true) { }
virtual ~frkv() {}
int yylex();

 boost::signals2::signal<void(const std::string &)> got_key;
 boost::signals2::signal<void(const std::string &)> got_value;
};

#endif
