/*  simple_m4.cpp - simple m4 implementation
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

#include "simple_m4.h"

#include <assert.h>
#include <fstream>
#include <map>
#include <set>
#include <stack>
#include <sstream>
#include <string.h>

#define DEFAULT_OPEN_QUOTE "`"
#define DEFAULT_CLOSE_QUOTE "'"

#define DEFAULT_COMMENT "#"

namespace simple_m4
{

const M4Functions main_functions = {
    { "m4_dnl",MacroType::dnl },
    { "m4_changecom",MacroType::changecom },
    { "m4_changequote",MacroType::changequote },
    { "m4_define",MacroType::define },
    { "m4_ifdef",MacroType::ifdef },
    { "m4_ifelse",MacroType::ifelse },
    { "m4_undefine",MacroType::undefine },
};

M4::M4()
{
    open_quote = DEFAULT_OPEN_QUOTE;
    closing_quote = DEFAULT_CLOSE_QUOTE;
    comment = DEFAULT_COMMENT;
    n_quotes = 0;
    n_brackets = 0;
    functions = main_functions;
}

M4Token M4::getNextToken1()
{
    auto c = *p;

    if (c == 0)
        return{ TokenType::eos };

    M4Token tok;

    if (memcmp(p, open_quote.c_str(), open_quote.size()) == 0)
    {
        p += open_quote.size();
        tok.word = open_quote;
        tok.type = TokenType::open_quote;
        return tok;
    }

    if (memcmp(p, closing_quote.c_str(), closing_quote.size()) == 0)
    {
        p += closing_quote.size();
        tok.word = closing_quote;
        tok.type = TokenType::closing_quote;
        return tok;
    }

    if (!comment.empty() && memcmp(p, comment.c_str(), comment.size()) == 0)
    {
        p += comment.size();
        skipToNewLine();
        tok.type = TokenType::comment;
        tok.word = comment;
        return tok;
    }

    if (isalpha(c) || c == '_')
    {
        int len = 0;
        while (1)
        {
            if (isalnum(c) || c == '_')
                len++;
            else
                break;
            c = *++p;
            if (c == 0)
                break;
        }
        tok.word.assign(p - len, p);
        tok.type = TokenType::word;
        return tok;
    }

    if (isdigit(c))
    {
        TokenType t = TokenType::integer;
        while (1)
        {
            if (isdigit(c))
                tok.word += c;
            else
            {
                if (isalnum(c) || c == '_')
                {
                    t = TokenType::digit_word;
                    tok.word += c;
                }
                else
                    break;
            }
            c = *++p;
            if (c == 0)
                break;
        }
        tok.type = t;
        return tok;
    }

    tok.word = c;
    p++;

    switch (c)
    {
    case '\n':
        tok.type = TokenType::eol;
        break;
    case '(':
        tok.type = TokenType::open_brace;
        break;
    case ')':
        tok.type = TokenType::closing_brace;
        break;
    case ',':
        tok.type = TokenType::comma;
        break;
    case '$':
        tok.type = TokenType::dollar;
        break;
    default:
        tok.type = TokenType::character;
        break;
    }
    return tok;
}

M4Token M4::getNextToken()
{
    auto prev = p;
    token = getNextToken1();
    token.length = p - prev;
    return token;
}

void M4::backtrackToken()
{
    p -= token.length;
}

void M4::skipToNewLine()
{
    while (*p && *p != '\n')
        p++;
    if (*p == '\n')
        p++;
}

std::string M4::processDollar(const MacroParams &params)
{
    if (state != State::MacroCall)
        return token.word;

    std::string result;

    getNextToken();

    bool quote = false;
    switch (token.type)
    {
    case TokenType::integer:
        if (params.size())
            result += params[std::stoi(token.word)];
        else
            result += "$" + token.word;
        break;
    case TokenType::character:
        switch (token.word[0])
        {
        case '#':
            result += std::to_string(params.size() ? params.size() - 1 : 0);
            break;
        case '@':
            quote = true;
        case '*':
            if (params.size())
            {
                int i = 0;
                for (auto &p : params)
                {
                    if (i++)
                        result += processLine(quote ? (open_quote + p + closing_quote) : p) + ",";
                }
                if (params.size())
                    result.resize(result.size() - 1);
                break;
            }
            result += "$" + token.word;
            break;
        default:
            result += "$" + token.word;
            break;
        }
        break;
    default:
        result += '$';
        backtrackToken();
        break;
    }
    return result;
}

std::string M4::readToClosingQuote(const MacroParams &params)
{
    int n_current_quotes = n_quotes;
    std::string text;
    while (n_quotes != n_current_quotes - 1)
    {
        getNextToken();
        switch (token.type)
        {
        case TokenType::dollar:
            text += processDollar(params);
            break;
        case TokenType::open_quote:
            n_quotes++;
            text += open_quote + readToClosingQuote(params);
            break;
        case TokenType::closing_quote:
            n_quotes--;
            if (n_quotes > 0)
                text += closing_quote;
            break;
        case TokenType::eos:
            text += "\n";
            return text;
        default:
            text += token.word;
            break;
        }
        if (token.type == TokenType::eos)
            break;
    }
    return text;
}

void M4::readToCommaOrRBracket(const MacroParams &params)
{
    int n_current_brackets = n_brackets;
    while (n_brackets != n_current_brackets - 1)
    {
        getNextToken();
        switch (token.type)
        {
        case TokenType::dollar:
            param += processDollar(params);
            break;
        case TokenType::comma:
            if (n_brackets == n_current_brackets)
            {
                processParameter();
                return;
            }
            param += token.word;
            break;
        case TokenType::open_brace:
            n_brackets++;
            param += token.word;
            break;
        case TokenType::closing_brace:
            n_brackets--;
            if (n_brackets == n_current_brackets - 1)
            {
                processParameter();
                return;
            }
            param += token.word;
            break;
        case TokenType::open_quote:
            n_quotes++;
            param += open_quote + readToClosingQuote(params);
            if (token.type == TokenType::closing_quote && n_quotes == 0)
            {
                param += closing_quote;
                quouted_string.clear();
            }
            break;
        case TokenType::eos:
            param += "\n";
            return;
        default:
            param += token.word;
            break;
        }
        if (token.type == TokenType::eos)
            break;
    }
}

bool M4::isQuoted(const std::string &s)
{
    auto b = s.find(open_quote) == 0;
    auto e = s.rfind(closing_quote) == (s.size() - closing_quote.size());
    return b && e;
}

bool M4::removeQuotes(std::string &s)
{
    auto quoted = isQuoted(s);
    if (quoted)
    {
        s = s.substr(open_quote.size());
        s.resize(s.size() - closing_quote.size());
    }
    return quoted;
}

bool M4::isEmpty(const std::string &s)
{
    for (auto &c : s)
        if (!isspace(c))
            return false;
    return true;
}

std::string M4::processLine(const std::string &text, const MacroParams &params)
{
    auto _prev_p = p;
    auto old_token = token;

    if (text.size() == 0)
    {
        p = text.c_str();
        if (n_quotes)
            quouted_string += "\n";
        else if (state == State::GatherMacroParameters)
            param += "\n";
    }

    std::string result;
    for (p = text.c_str(); *p;)
    {
        if (n_quotes)
        {
            quouted_string += readToClosingQuote(params);
            if (token.type == TokenType::closing_quote && n_quotes == 0)
            {
                if (state == State::GatherMacroParameters)
                    param += quouted_string + closing_quote;
                else
                    result += quouted_string;
                quouted_string.clear();
            }
            continue;
        }
        if (state == State::GatherMacroParameters)
        {
            readToCommaOrRBracket(params);
            if (n_brackets == 0)
            {
                result += processMacro(params);
            }
            continue;
        }

        getNextToken();

        switch (token.type)
        {
        case TokenType::comment:
            result += comment;
            break;
        case TokenType::word:
            result += processWord();
            break;
        case TokenType::open_quote:
            n_quotes++;
            quouted_string += readToClosingQuote(params);
            if (token.type == TokenType::closing_quote && n_quotes == 0)
            {
                result += quouted_string;
                quouted_string.clear();
            }
            break;
        case TokenType::closing_quote:
            if (n_quotes == 0)
            {
                result += token.word;
                break;
            }
            break;
        case TokenType::dollar:
            result += processDollar(params);
            break;
        case TokenType::eos:
            break;
        default:
            result += token.word;
            break;
        }

        if (token.type == TokenType::eos)
            break;
    }

    p = _prev_p;
    token = old_token;

    return result;
}

std::string M4::processMacro(const MacroParams &params)
{
    std::string result;
    auto macro = macros.top();
    macros.pop();

    int i = 0;
    for (auto &p : macro.macro_params)
    {
        if (!i++)
            continue;
        param = "";
        state = State::NoMacroCalls;
        p = processLine(p);
    }

    param = "";
    state = State::MacroCall;

    switch (macro.type)
    {
    case MacroType::changequote:
        switch (macro.macro_params.size())
        {
        case 1:
            open_quote = DEFAULT_OPEN_QUOTE;
            closing_quote = DEFAULT_CLOSE_QUOTE;
            break;
        case 2:
            open_quote = macro.macro_params[1];
            closing_quote = DEFAULT_CLOSE_QUOTE;
            break;
        case 3:
            open_quote = macro.macro_params[1];
            closing_quote = macro.macro_params[2];
            break;
        }
        break;
    case MacroType::define:
        switch (macro.macro_params.size())
        {
        case 2:
            definitions[macro.macro_params[1]];
            break;
        case 3:
            definitions[macro.macro_params[1]] = macro.macro_params[2];
            break;
        }
        break;
    case MacroType::ifdef:
    {
        auto d = definitions.find(macro.macro_params[1]);
        if (d != definitions.end())
        {
            result += processLine
                (macro.macro_params[2]);
        }
        else if (macro.macro_params.size() > 3)
        {
            result += processLine
                (macro.macro_params[3]);
        }
    }
    break;
    case MacroType::ifelse:
        switch (macro.macro_params.size())
        {
        case 5:
        {
            auto s1 = processLine
                (macro.macro_params[1], params);
            auto s2 = processLine
                (macro.macro_params[2], params);
            if (s1 == s2)
                result += processLine(macro.macro_params[3], params);
            else
                result += processLine(macro.macro_params[4], params);
        }
        break;
        default:
            assert(false);
            break;
        }
        break;
    case MacroType::UserDefined:
    {
        auto d = definitions[macro.macro_params[0]];
        result += processLine(d, macro.macro_params);
    }
    break;
    case MacroType::undefine:
        definitions.erase(processLine(macro.macro_params[1]));
        break;
    case MacroType::include:
    {
        auto f = read_file(processLine(macro.macro_params[1]));
        result += processLine(f);
    }
    break;
    default:
        assert(false);
        break;
    }

    param = "";
    state = State::None;

    return result;
}

void M4::processParameter()
{
    auto begin = [this]
    {
        auto &text = param;
        int s = 0;
        for (auto r = text.begin(); r != text.end(); ++r)
        {
            char c = *r;
            if (isspace(c))
                s++;
            else
                break;
        }
        if (s)
            text = text.substr(s);
    };

    begin();

    macros.top().macro_params.push_back(param);
    param = decltype(param)();
}

std::string M4::processWord()
{
    std::string result;
    auto word = token.word;

    auto m = functions.find(word);
    if (m == functions.end())
    {
        auto d = definitions.find(word);
        if (d != definitions.end())
        {
            // no brace, simple replacement
            getNextToken();
            if (token.type != TokenType::open_brace)
            {
                word = processLine(d->second);
                backtrackToken();
            }
            else
            {
                n_brackets++;
                state = State::GatherMacroParameters;
                macros.push(Macro{ MacroType::UserDefined });
                macros.top().macro_params.push_back(MacroParam{ word });
                param = decltype(param)();
                return result;
            }
        }

        result += word;
        return result;
    }

    // dnl comes w/out braces
    if (m->second == MacroType::dnl)
    {
        skipToNewLine();
        return result;
    }

    // no brace, no output
    getNextToken();
    if (token.type != TokenType::open_brace)
    {
        if (m->second == MacroType::changequote)
        {
            open_quote = DEFAULT_OPEN_QUOTE;
            closing_quote = DEFAULT_CLOSE_QUOTE;
        }
        else if (m->second == MacroType::changecom)
        {
            comment = "";
        }
        backtrackToken();
        return result;
    }

    n_brackets++;
    state = State::GatherMacroParameters;
    macros.push(Macro{ m->second });
    macros.top().macro_params.push_back(MacroParam{ m->first });
    param = decltype(param)();
    return result;
}

std::string read_file(const std::string &filename)
{
    std::ifstream ifile(filename);
    if (!ifile)
        throw std::runtime_error("Cannot open file " + filename);
    std::string f, s;
    while (std::getline(ifile, s))
        f += s + "\n";
    return f;
}

} // namespace simple_m4

bool m4(Context::Lines &lines)
{
    simple_m4::M4 processor;
    processor.setFunctions(simple_m4::main_functions);
    for (auto &line : lines)
        line.text = processor.processLine(line.text);
    return true;
}
