/* misc - miscellaneous flex routines */

/*  Copyright (c) 1990 The Regents of the University of California. */
/*  All rights reserved. */

/*  This code is derived from software contributed to Berkeley by */
/*  Vern Paxson. */

/*  The United States Government has rights in this work pursuant */
/*  to contract no. DE-AC03-76SF00098 between the United States */
/*  Department of Energy and the University of California. */

/*  This file is part of flex. */

/*  Redistribution and use in source and binary forms, with or without */
/*  modification, are permitted provided that the following conditions */
/*  are met: */

/*  1. Redistributions of source code must retain the above copyright */
/*     notice, this list of conditions and the following disclaimer. */
/*  2. Redistributions in binary form must reproduce the above copyright */
/*     notice, this list of conditions and the following disclaimer in the */
/*     documentation and/or other materials provided with the distribution. */

/*  Neither the name of the University nor the names of its contributors */
/*  may be used to endorse or promote products derived from this software */
/*  without specific prior written permission. */

/*  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR */
/*  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED */
/*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR */
/*  PURPOSE. */

#include <fstream>
#include <iostream>
#include <iomanip>
#include <stack>

#include "misc.h"

#include "flexdef.h"

#include "context.h"
#include "tables.h"

#define CMD_IF_TABLES_SER "%if-tables-serialization"
#define CMD_TABLES_YYDMAP "%tables-yydmap"
#define CMD_DEFINE_YYTABLES "%define-yytables"
#define CMD_IF_CPP_ONLY "%if-c++-only"
#define CMD_IF_C_ONLY "%if-c-only"
#define CMD_IF_C_OR_CPP "%if-c-or-c++"
#define CMD_NOT_FOR_HEADER "%not-for-header"
#define CMD_OK_FOR_HEADER "%ok-for-header"
#define CMD_PUSH "%push"
#define CMD_POP "%pop"
#define CMD_IF_REENTRANT "%if-reentrant"
#define CMD_IF_NOT_REENTRANT "%if-not-reentrant"
#define CMD_IF_BISON_BRIDGE "%if-bison-bridge"
#define CMD_IF_NOT_BISON_BRIDGE "%if-not-bison-bridge"
#define CMD_ENDIF "%endif"

extern std::ifstream skelfile;

/* Append "#define defname value\n" to the running buffer. */
void action_define(const String &defname, int value)
{
    add_action("#define " + defname + " " + std::to_string(value) + "\n");
}

/* Append "new_text" to the running buffer. */
void add_action(const String &new_text)
{
    action_array += new_text;
}

/* allocate_array - allocate memory for an integer array of the given size */
void *allocate_array(int size, size_t element_size)
{
    void *mem;
    size_t num_bytes = element_size * size;

    mem = malloc(num_bytes);
    if (!mem)
        flexfatal(_("memory allocation failed in allocate_array()"));

    return mem;
}

/* is_all_lower - true if a string is all lower-case */
bool is_all_lower(const String &s)
{
    for (auto &c : s)
        if (!isascii(c) || !islower(c))
            return false;
    return true;
}

/* is_all_upper - true if a string is all upper-case */
bool is_all_upper(const String &s)
{
    for (auto &c : s)
        if (!isascii(c) || !isupper(c))
            return false;
    return true;
}

/* intcmp - compares two integers for use by qsort. */
int intcmp(const void *a, const void *b)
{
    return *(const int *)a - *(const int *)b;
}

/* check_char - checks a character to make sure it's within the range
 *		we're expecting.  If not, generates fatal error message
 *		and exits.
 */
void check_char(int c)
{
    if (c >= CSIZE)
        lerr(_("bad character '%s' detected in check_char()"),
             readable_form(c));

    if (c >= csize)
        lerr(_("scanner requires -8 flag to use the character %s"),
             readable_form(c));
}

/* clower - replace upper-case letter to lower-case */
unsigned char clower(int c)
{
    return (unsigned char)((isascii(c) && isupper(c)) ? tolower(c) : c);
}

/* dataend - finish up a block of data declarations */
void dataend(void)
{
    /* short circuit any output */
    if (gentables)
    {

        if (datapos > 0)
            dataflush();

        /* add terminator for initialization; { for vi */
        outn("    } ;\n");
    }
    dataline = 0;
    datapos = 0;
}

/* dataflush - flush generated data statements */
void dataflush(void)
{
    /* short circuit any output */
    if (!gentables)
        return;

    processed_file << Context::eol;

    if (++dataline >= NUMDATALINES)
    {
        /* Put out a blank line so that the table is grouped into
		 * large blocks that enable the user to find elements easily.
         */
        processed_file << Context::eol;
        dataline = 0;
    }

    /* Reset the number of characters written on the current line. */
    datapos = 0;
}

/* flexerror - report an error message and terminate */
void flexerror(const String &msg)
{
    std::cerr << program_name << ": " << " " << msg << "\n";
    flexend(1);
}

/* flexfatal - report a fatal error message and terminate */
void flexfatal(const String &msg)
{
    std::cerr << program_name << ": " << _("fatal internal error") << ", " << msg << "\n";
    flex_exit(1);
}

/* lerr - report an error message */
void lerr(const char *msg, ...)
{
    char errmsg[MAXLINE];
    va_list args;

    va_start(args, msg);
    vsnprintf(errmsg, sizeof(errmsg), msg, args);
    va_end(args);
    flexerror(errmsg);
}

/* lerr_fatal - as lerr, but call flexfatal */
void lerr_fatal(const char *msg, ...)
{
    char errmsg[MAXLINE];
    va_list args;
    va_start(args, msg);

    vsnprintf(errmsg, sizeof(errmsg), msg, args);
    va_end(args);
    flexfatal(errmsg);
}

/* line_directive_out - spit out a "#line" statement */
void line_directive_out(bool print, bool do_infile)
{
    if (!gen_line_dirs)
        return;

    String filename = do_infile ? infilename : "M4_YY_OUTFILE_NAME";

    if (do_infile && filename.empty())
        filename = "<stdin>";

    replace_all(filename, "\\", "\\\\");

    std::ostringstream ss;
    ss << "#line " << (do_infile ? linenum : 0) << " \"" << filename << "\"" << "\n";

    /* If print is false then we should put the directive in
	 * the accumulated actions.
	 */
    if (print)
        processed_file.addLine(ss.str());
    else
        add_action(ss.str().c_str());
}

/* mark_defs1 - mark the current position in the action array as
 *               representing where the user's section 1 definitions end
 *		 and the prolog begins
 */
void mark_defs1(void)
{
    defs1_array = action_array;
    action_array.clear();
}

/* mark_prolog - mark the current position in the action array as
 *               representing the end of the action prolog
 */
void mark_prolog(void)
{
    prolog_array = action_array;
    action_array.clear();
}

/* mkdata - generate a data statement
 *
 * Generates a data statement initializing the current array element to
 * "value".
 */
void mkdata(int value)
{
    /* short circuit any output */
    if (!gentables)
        return;

    if (datapos >= NUMDATAITEMS)
    {
        processed_file << ',';
        dataflush();
    }

    if (datapos == 0)
        processed_file << "    ";
    else
        processed_file << ',';

    ++datapos;

    processed_file << std::setw(5) << value;
}

/* myesc - return character corresponding to escape sequence */
unsigned char myesc(unsigned char array[])
{
    unsigned char c, esc_char;

    switch (array[1])
    {
    case 'b':
        return '\b';
    case 'f':
        return '\f';
    case 'n':
        return '\n';
    case 'r':
        return '\r';
    case 't':
        return '\t';
    case 'a':
        return '\a';
    case 'v':
        return '\v';
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    { /* \<octal> */
        int sptr = 1;

        while (isascii(array[sptr]) &&
               isdigit(array[sptr]))
            /* Don't increment inside loop control
				 * because if isdigit() is a macro it might
				 * expand into multiple increments ...
				 */
            ++sptr;

        c = array[sptr];
        array[sptr] = '\0';

        auto o = std::stoi((const char *)array + 1, 0, 8);
        esc_char = o;

        array[sptr] = c;

        return esc_char;
    }

    case 'x':
    { /* \x<hex> */
        int sptr = 2;

        while (isascii(array[sptr]) &&
               isxdigit(array[sptr]))
            /* Don't increment inside loop control
				 * because if isdigit() is a macro it might
				 * expand into multiple increments ...
				 */
            ++sptr;

        c = array[sptr];
        array[sptr] = '\0';

        auto h = std::stoi((const char *)array + 2, 0, 16);
        esc_char = h;

        array[sptr] = c;

        return esc_char;
    }

    default:
        return array[1];
    }
}

/* readable_form - return the the human-readable form of a character
 *
 * The returned string is in static storage.
 */
const char *readable_form(int c)
{
    static char rform[20];

    if ((c >= 0 && c < 32) || c >= 127)
    {
        switch (c)
        {
        case '\b':
            return "\\b";
        case '\f':
            return "\\f";
        case '\n':
            return "\\n";
        case '\r':
            return "\\r";
        case '\t':
            return "\\t";
        case '\a':
            return "\\a";
        case '\v':
            return "\\v";
        default:
            if (trace_hex)
                snprintf(rform, sizeof(rform), "\\x%.2x", (unsigned int)c);
            else
                snprintf(rform, sizeof(rform), "\\%.3o", (unsigned int)c);
            return rform;
        }
    }

    else if (c == ' ')
        return "' '";

    else
    {
        rform[0] = c;
        rform[1] = '\0';

        return rform;
    }
}

/* reallocate_array - increase the size of a dynamic array */
void *reallocate_array(void *array, int size, size_t element_size)
{
    void *new_array;
    size_t num_bytes = element_size * size;

    new_array = realloc(array, num_bytes);
    if (!new_array)
        flexfatal(_("attempt to increase array size failed"));

    return new_array;
}

/* skelout - write out one section of the skeleton file
 *
 * Description
 *    Copies skelfile or skel array to stdout until a line beginning with
 *    "%%" or EOF is found.
 */
void skelout(void)
{
    String buf;
    bool do_copy = true;

    std::stack<bool> sko_stack;
    sko_stack.push(true);

    /* Loop pulling lines either from the skelfile, if we're using
	 * one, or from the skel[] array.
	 */
    while (1)
    {
        if (skelfile.is_open())
        {
            if (!std::getline(skelfile, buf))
                break;
            while (buf.back() == '\r')
                buf.resize(buf.size() - 1);
        }
        else
        {
            auto s = skel[skel_ind++];
            if (!s)
                break;
            buf = s;
        }

        /* copy from skel array */
        if (buf[0] == '%')
        { /* control line */
            /* print the control line as a comment. */
            if (ddebug && buf[1] != '#')
            {
                processed_file << "/* " << buf << " */" << (buf.back() == '\\' ? "\\" : "") << Context::eol;
            }

/* We've been accused of using cryptic markers in the skel.
			 * So we'll use emacs-style-hyphenated-commands.
             * We might consider a hash if this if-else-if-else
             * chain gets too large.
			 */
#define cmd_match(s) (buf.find(s) == 0)

            if (buf[1] == '%')
            {
                /* %% is a break point for skelout() */
                return;
            }
            else if (cmd_match(CMD_PUSH))
            {
                sko_stack.push(do_copy);
                if (ddebug)
                {
                    processed_file << "/*(state = (" << (do_copy ? "true" : "false") << ") */" << Context::eol;
                }
                processed_file << (buf.back() == '\\' ? "\\" : "") << Context::eol;
            }
            else if (cmd_match(CMD_POP))
            {
                do_copy = sko_stack.top();
                sko_stack.pop();
                if (ddebug)
                {
                    processed_file << "/*(state = (" << (do_copy ? "true" : "false") << ") */" << Context::eol;
                }
                processed_file << (buf.back() == '\\' ? "\\" : "") << Context::eol;
            }
            else if (cmd_match(CMD_IF_REENTRANT))
            {
                sko_stack.push(do_copy);
                do_copy = reentrant && do_copy;
            }
            else if (cmd_match(CMD_IF_NOT_REENTRANT))
            {
                sko_stack.push(do_copy);
                do_copy = !reentrant && do_copy;
            }
            else if (cmd_match(CMD_IF_BISON_BRIDGE))
            {
                sko_stack.push(do_copy);
                do_copy = bison_bridge_lval && do_copy;
            }
            else if (cmd_match(CMD_IF_NOT_BISON_BRIDGE))
            {
                sko_stack.push(do_copy);
                do_copy = !bison_bridge_lval && do_copy;
            }
            else if (cmd_match(CMD_ENDIF))
            {
                do_copy = sko_stack.top();
                sko_stack.pop();
            }
            else if (cmd_match(CMD_IF_TABLES_SER))
            {
                do_copy = do_copy && tablesext;
            }
            else if (cmd_match(CMD_TABLES_YYDMAP))
            {
                if (tablesext && !yydmap_buf.empty())
                    outn((char *)(yydmap_buf.getText().c_str()));
            }
            else if (cmd_match(CMD_DEFINE_YYTABLES))
            {
                processed_file << "#define YYTABLES_NAME \""
                    << (!tablesname.empty() ? tablesname : "yytables") << "\"" << Context::eol;
            }
            else if (cmd_match(CMD_IF_CPP_ONLY))
            {
                /* only for C++ */
                sko_stack.push(do_copy);
                do_copy = !!C_plus_plus;
            }
            else if (cmd_match(CMD_IF_C_ONLY))
            {
                /* %- only for C */
                sko_stack.push(do_copy);
                do_copy = !C_plus_plus;
            }
            else if (cmd_match(CMD_IF_C_OR_CPP))
            {
                /* %* for C and C++ */
                sko_stack.push(do_copy);
                do_copy = true;
            }
            else if (cmd_match(CMD_NOT_FOR_HEADER))
            {
                /* %c begin linkage-only (non-header) code. */
                OUT_BEGIN_CODE();
            }
            else if (cmd_match(CMD_OK_FOR_HEADER))
            {
                /* %e end linkage-only code. */
                OUT_END_CODE();
            }
            else if (buf[1] == '#')
            {
                /* %# a comment in the skel. ignore. */
            }
            else
            {
                flexfatal(_("bad line in skeleton file"));
            }
        }

        else if (do_copy)
            outn(buf.c_str());
    } /* end while */
}

/* transition_struct_out - output a yy_trans_info structure
 *
 * outputs the yy_trans_info structure with the two elements, element_v and
 * element_n.  Formats the output with spaces and carriage returns.
 */
void transition_struct_out(int element_v, int element_n)
{
    /* short circuit any output */
    if (!gentables)
        return;

    processed_file << " {" << std::setw(4) << element_v << "," << std::setw(4) << element_n << " }," << Context::eol;

    datapos += TRANS_STRUCT_PRINT_LENGTH;

    if (datapos >= 79 - TRANS_STRUCT_PRINT_LENGTH)
    {
        processed_file << Context::eol;

        if (++dataline % 10 == 0)
            processed_file << Context::eol;

        datapos = 0;
    }
}

void report_internal(const String &s, const char *file, int line, const char *func)
{
    fprintf(stderr, _("%s: fatal internal error at %s:%d (%s): %s\n"),
        program_name.c_str(), file, line, func, s.c_str());
    flex_exit(1);
}

