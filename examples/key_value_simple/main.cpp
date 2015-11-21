#include "frkv.yy.h"
#include <iostream>

void key_callback(const std::string &key)
{
  std::cout << "Got key: " << key << std::endl;
}

void value_callback(const std::string &value)
{
  std::cout << "Got value: " << value << std::endl;
}

int main(int argc, char *argv[])
{
  frkv *lexer = new frkv;
  lexer->got_key.connect(&key_callback);
  lexer->got_value.connect(&value_callback);
  while (lexer->yylex() != 0);
}
