/*  context.cpp - context implementation
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

#include "context.h"

#include <stdio.h>

/* global buffers. */
Context userdef_buf;		/**< for user #definitions triggered by cmd-line. */
Context yydmap_buf;		/**< string buffer to hold yydmap elements */
Context m4defs_buf;          /**< m4 definitions. List of strings. */
Context top_buf;             /**< contains %top code. String buffer. */

Context::eol_type Context::eol;

Context::Lines &operator+=(Context::Lines &s1, const Context::Lines &s2)
{
    s1.insert(s1.end(), s2.begin(), s2.end());
    return s1;
}

Context::Lines operator+(const Context::Lines &s1, const Context::Lines &s2)
{
    Context::Lines s(s1.begin(), s1.end());
    return s += s2;
}

Context::Context(const Text &indent, const Text &newline)
    : indent(indent), newline(newline)
{
}

void Context::initFromString(const std::string &s)
{
    int p = 0;
    while (1)
    {
        int p2 = s.find('\n', p);
        if (p2 == s.npos)
            break;
        auto line = s.substr(p, p2 - p);
        int space = 0;
        for (auto i = line.rbegin(); i != line.rend(); ++i)
        {
            if (isspace(*i))
                space++;
            else
                break;
        }
        if (space)
            line.resize(line.size() - space);
        lines.push_back({ line });
        p = p2 + 1;
    }
}

void Context::addText(const Text &s)
{
    if (lines.empty())
        lines.emplace_back();
    lines.back() += s;
}

void Context::addText(const char* str, int n)
{
    addText(Text(str, str + n));
}

void Context::addNoNewLine(const Text &s)
{
    lines.push_back(Line{ s, n_indents });
}

void Context::addLineNoSpace(const Text & s)
{
    lines.push_back(Line{ s });
}

void Context::addLine(const Text &s)
{
    if (s.empty())
        lines.push_back(Line{});
    else
        lines.push_back({ s, n_indents });
}

void Context::decreaseIndent()
{
    n_indents--;
}

void Context::increaseIndent()
{
    n_indents++;
}

void Context::beginBlock(const Text &s, bool indent)
{
    if (!s.empty())
        addLine(s);
    addLine("{");
    if (indent)
        increaseIndent();
}

void Context::endBlock(bool semicolon)
{
    decreaseIndent();
    addLine(semicolon ? "};" : "}");
}

void Context::beginFunction(const Text &s)
{
    beginBlock(s);
}

void Context::endFunction()
{
    endBlock();
    addLine();
}

void Context::beginNamespace(const Text &s)
{
    addLineNoSpace("namespace " + s);
    addLineNoSpace("{");
    addLine();
    namespaces.push(s);
}

void Context::endNamespace(const Text &ns)
{
    Text s = ns;
    if (!namespaces.empty() && ns.empty())
    {
        s = namespaces.top();
        namespaces.pop();
    }
    addLineNoSpace("} // namespace " + s);
    addLine();
}

void Context::ifdef(const Text &s)
{
    addLineNoSpace("#ifdef " + s);
}

void Context::endif()
{
    addLineNoSpace("#endif");
}

void Context::trimEnd(size_t n)
{
    if (lines.empty())
        return;
    auto &t = lines.back().text;
    auto sz = t.size();
    if (n > sz)
        n = sz;
    t.resize(sz - n);
}

Context::Text Context::getText() const
{
    Text s;
    auto lines = getLines();
    for (auto &line : lines)
    {
        Text space;
        for (int i = 0; i < line.n_indents; i++)
            space += indent;
        s += space + line.text + newline;
    }
    return s;
}

Context::Lines Context::getLines() const
{
    Lines lines;
    if (before_)
        lines += before_->getLines();
    lines += this->lines;
    if (after_)
        lines += after_->getLines();
    return lines;
}

void Context::setLines(const Lines &lines)
{
    before_.reset();
    after_.reset();
    this->lines = lines;
}

void Context::mergeBeforeAndAfterLines()
{
    if (before_)
    {
        before_->mergeBeforeAndAfterLines();
        lines.insert(lines.begin(), before_->getLinesRef().begin(), before_->getLinesRef().end());
        before_.reset();
    }
    if (after_)
    {
        after_->mergeBeforeAndAfterLines();
        lines.insert(lines.end(), after_->getLinesRef().begin(), after_->getLinesRef().end());
        after_.reset();
    }
}

void Context::setMaxEmptyLines(int n)
{
    // remove all empty lines at begin
    while (1)
    {
        auto line = lines.begin();
        if (line == lines.end())
            break;
        bool empty = true;
        for (auto &c : line->text)
        {
            if (!isspace(c))
            {
                empty = false;
                break;
            }
        }
        if (empty)
            lines.erase(line);
        else
            break;
    }

    int el = 0;
    for (auto line = lines.begin(); line != lines.end(); ++line)
    {
        bool empty = true;
        for (auto &c : line->text)
        {
            if (!isspace(c))
            {
                empty = false;
                break;
            }
        }
        if (empty)
            el++;
        else
            el = 0;
        if (el > n)
        {
            line = lines.erase(line);
            --line;
        }
    }
}

void Context::splitLines()
{
    for (auto line = lines.begin(); line != lines.end(); ++line)
    {
        auto &text = line->text;
        auto p = text.find('\n');
        if (p == text.npos)
            continue;

        int old_pos = 0;
        Lines ls;
        while (1)
        {
            ls.push_back(Line{text.substr(old_pos, p - old_pos)});
            p++;
            old_pos = p;
            p = text.find('\n', p);
            if (p == text.npos)
            {
                ls.push_back(Line{ text.substr(old_pos) });
                break;
            }
        }
        lines.insert(line, ls.begin(), ls.end());
        line = lines.erase(line);
        line--;
    }
}

void Context::emptyLines(int n)
{
    int e = 0;
    for (auto i = lines.rbegin(); i != lines.rend(); ++i)
    {
        if (i->text.empty())
            e++;
        else
            break;
    }
    if (e < n)
    {
        while (e++ != n)
            addLine();
    }
    else if (e > n)
    {
        lines.resize(lines.size() - (e - n));
    }
}

Context &Context::operator+=(const Context &rhs)
{
    if (before_ && rhs.before_)
        before_->lines += rhs.before_->lines;
    else if (rhs.before_)
    {
        before().lines += rhs.before_->lines;
    }
    lines += rhs.lines;
    if (after_ && rhs.after_)
        after_->lines += rhs.after_->lines;
    else if (rhs.after_)
    {
        after().lines += rhs.after_->lines;
    }
    return *this;
}

void Context::addWithRelativeIndent(const Context &rhs)
{
    auto addWithRelativeIndent = [this](Lines &l1, Lines l2)
    {
        for (auto &l : l2)
            l.n_indents += n_indents;
        l1 += l2;
    };

    if (before_ && rhs.before_)
        addWithRelativeIndent(before_->lines, rhs.before_->lines);
    else if (rhs.before_)
    {
        addWithRelativeIndent(before().lines, rhs.before_->lines);
    }
    addWithRelativeIndent(lines, rhs.lines);
    if (after_ && rhs.after_)
        addWithRelativeIndent(after_->lines, rhs.after_->lines);
    else if (rhs.after_)
    {
        addWithRelativeIndent(after().lines, rhs.after_->lines);
    }
}

void Context::printToFile(FILE* out) const
{
    fprintf(out, "%s", getText().c_str());
}
