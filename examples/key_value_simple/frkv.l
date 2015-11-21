%option noyywrap

%{
#include <stdexcept>
#include <string>
%}

string     \"[^\n"]+\"
comment    #[^\n]+
whitespace [ \t]+
delim      =
alnum      [A-Za-z0-9]+

%%

{whitespace} /* eat whitespace */
{comment} /* Eat comments */

{alnum} {
         if (this->key) {
            this->got_key(std::string(YYText()));
         } else {
            this->got_value(std::string(YYText()));
         }
}

{string} {
          if (!this->key) {
             this->got_value(std::string(YYText()));
          } else {
             throw std::logic_error("Key was quoted in key/value file");
          }
}

{delim} {
         if (this->key) {
            this->key = false;
         } else {
            throw std::logic_error("Misplaced delimter detected");
         }
}

\n       this->key = true;
