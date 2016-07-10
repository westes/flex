/*  simple_m4.h - simple m4 implementation for flex
 *
 *  Copyright (c) 2016 Egor Pugin
 *
 *  This file is part of flex.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  Neither the name of the University nor the names of its contributors
 *  may be used to endorse or promote products derived from this software
 *  without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE.
 */

#pragma once

#include <map>
#include <string>

#include "context.h"

namespace simple_m4
{

enum class State
{
    None,
    GatherMacroParameters,
    MacroCall,
    NoMacroCalls,
};

enum class MacroType
{
    None,
    UserDefined,

    dnl,
    changecom,
    changequote,
    define,
    ifdef,
    ifelse,
    undefine,
    include,
};

using MacroParam = std::string;
using MacroParams = std::vector<MacroParam>;

struct Macro
{
    MacroType type;
    MacroParams macro_params;
};

enum class TokenType
{
    eos,
    eol,
    word,
    digit_word,
    open_quote,
    closing_quote,
    integer,
    comment,
    character,
    open_brace,
    closing_brace,
    comma,
    dollar,
};

struct M4Token
{
    TokenType type;
    std::string word;
    int length;
};

using M4Functions = std::map<std::string, MacroType>;

class M4
{
public:
    M4();

    std::string processLine(const std::string &text, const MacroParams &params = MacroParams());

    void setFunctions(const M4Functions &functions)
    {
        this->functions = functions;
    }

    void addDefinition(const std::string &key, const std::string &value)
    {
        definitions[key] = value;
    }
    std::string getDefinition(const std::string &key)
    {
        return definitions[key];
    }

private:
    State state = State::None;
    std::map<std::string, std::string> definitions;
    std::string open_quote;
    std::string closing_quote;
    std::string comment;
    int n_quotes;
    std::stack<Macro> macros;
    MacroParam param;
    M4Functions functions;
    int n_brackets;

    const char *p; // current input symbol
    M4Token token;
    std::string quouted_string;

    M4Token getNextToken();
    M4Token getNextToken1();
    void backtrackToken();
    void skipToNewLine();
    void readToCommaOrRBracket(const MacroParams &params);
    std::string readToClosingQuote(const MacroParams &params);
    std::string processDollar(const MacroParams &params);
    bool isQuoted(const std::string &s);
    bool removeQuotes(std::string &s);
    bool isEmpty(const std::string &s);

    std::string processWord();
    void processParameter();
    std::string processMacro(const MacroParams &params);
};

std::string read_file(const std::string &filename);

} // namespace simple_m4

bool m4(Context::Lines &lines);
