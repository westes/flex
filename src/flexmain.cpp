/* flex - tool to generate fast lexical analyzers */

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
#include <regex>

#include "flexdef.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "options.h"
#include "tables.h"
#include "misc.h"
#include "dfa.h"
#include "ecs.h"
#include "simple_m4.h"
#include "gen.h"
#include "scanflags.h"
#include <parse.h>

const std::string flex_version = FLEX_VERSION;

/* declare functions that have forward references */

void flexinit(int, char **);
void readin();
static String basename2(String path);

/* these globals are all defined and commented in flexdef.h */
int printstats, syntaxerror, eofseen, ddebug, trace, nowarn, spprdflt;
int interactive, lex_compat, posix_compat, do_yylineno, useecs, fulltbl, usemecs;
int fullspd, gen_line_dirs, performance_report, backing_up_report;
int C_plus_plus, long_align, use_read, yytext_is_array, do_yywrap, csize;
int reentrant, bison_bridge_lval, bison_bridge_lloc;
int yymore_used, reject, real_reject, continued_action, in_rule;
int yymore_really_used, reject_really_used;
int trace_hex = 0;

int datapos, dataline, linenum;
std::ifstream skelfile;
int skel_ind = 0;
String infilename, headerfilename;
String outfilename;
int did_outfilename;
String prefix, yyclass, extra_type;
int do_stdinit, use_stdout;
int onestate[ONE_STACK_SIZE], onesym[ONE_STACK_SIZE];
int onenext[ONE_STACK_SIZE], onedef[ONE_STACK_SIZE], onesp;
int numtemps, numprots, protprev[MSP], protnext[MSP], prottbl[MSP];
int protcomst[MSP], firstprot, lastprot, protsave[PROT_SAVE_SIZE];
int numecs;
int nextecm[CSIZE + 1];
int ecgroup[CSIZE + 1];
int nummecs;
int tecfwd[CSIZE + 1];
int tecbck[CSIZE + 1];
int numas;
int numsnpairs, jambase, jamstate;
String nmstr;
int sectnum, nummt, hshcol, dfaeql, numeps, eps2, num_reallocs;
int tmpuses, totnst, peakpairs, numuniq, numdup, hshsave;
int num_backing_up, bol_needed;
FILE *backing_up_file;
int end_of_buffer_state;
InputFiles input_files;
int nlch = '\n';
bool ansi_func_defs, ansi_func_protos;
String action_array, defs1_array, prolog_array;

Nfas nfas(1);
StateType current_state_type;

StartConditions start_conditions(1);

std::vector<int> nxt(1), chk(1), tnxt(1);
int NUL_ec, tblend, firstfree;
bool nultrans = false;
Dfas dfas(1);

CharacterClasses ccls(1);

int num_eof_rules, default_rule;
int variable_trailing_context_rules;
Rules rules(1); // make one dummy rule at 0 position

bool tablesext, tablesverify, gentables;
String tablesfilename, tablesname;
struct yytbl_writer tableswr;

/* Open the given file (if NULL, stdin) for scanning. */
void set_input_file(const String &);

void usage();

/* Make sure program_name is initialized so we don't crash if writing
 * out an error message before getting the program name from argv[0].
 */
String program_name = "flex";

static const char *outfile_template = "lex.%s.%s";
static const char *backing_name = "lex.backup";
static const char *tablesfile_template = "lex.%s.tables";

static char outfile_path[MAXLINE];
static String skelname;

/* For debugging. The max number of filters to apply to skeleton. */
static int preproc_level = 1000;

Context processed_file; // main buffer

int flex_main(int argc, char *argv[])
{
    flexinit(argc, argv);

    readin();

    skelout();
    /* %% [1.5] DFA */
    ntod();

    for (int i = 1; i < rules.size(); ++i)
        if (!rules[i].useful && i != default_rule)
            line_warning(_("rule cannot be matched"), rules[i].linenum);

    if (spprdflt && !reject && rules[default_rule].useful)
        line_warning(_("-s option given but default rule can be matched"), rules[default_rule].linenum);

    /* Generate the C state transition tables from the DFA. */
    make_tables();

    // rest of processsing
    auto print_lines = [](FILE *f, const auto &ctx)
    {
        for (auto &line : ctx.getLines())
            fprintf(f, "%s\n", line.text.c_str());
    };

    Context header_file;
    do
    {
        // init context
        if (preproc_level == 0)
            break;

        // create header
        if (preproc_level == 1)
            break;
        {
            if (!headerfilename.empty())
            {
                header_file.before().addLine("m4_changecom`'m4_dnl");
                header_file.before().addLine("m4_changequote`'m4_dnl");
                header_file.before().addLine("m4_changequote([[,]])[[]]m4_dnl");
                header_file.before().addLine("m4_define([[M4_YY_NOOP]])[[]]m4_dnl");
                header_file.before().addLine("m4_define( [[M4_YY_IN_HEADER]],[[]])m4_dnl");
                header_file.before().addLine("#ifndef " + prefix + "HEADER_H");
                header_file.before().addLine("#define " + prefix + "HEADER_H 1");
                header_file.before().addLine("#define " + prefix + "IN_HEADER 1");
                header_file.before().addLine();
                header_file.before().addLine("m4_define( [[M4_YY_OUTFILE_NAME]],[[" +
                                             (!headerfilename.empty() ? headerfilename : "<stdout>") + "]])m4_dnl");

                header_file += processed_file;

                header_file.after().addLine();
                header_file.after().addLine("#line 4000 \"M4_YY_OUTFILE_NAME\"\n");
                header_file.after().addLine("#undef " + String(prefix) + "IN_HEADER\n");
                header_file.after().addLine("#endif /* " + String(prefix) + "HEADER_H */\n");
                header_file.after().addLine("m4_undefine( [[M4_YY_IN_HEADER]])m4_dnl\n");
            }

            // write after src
            processed_file.before().addLine("m4_changecom`'m4_dnl");
            processed_file.before().addLine("m4_changequote`'m4_dnl");
            processed_file.before().addLine("m4_changequote([[,]])[[]]m4_dnl");
            processed_file.before().addLine("m4_define([[M4_YY_NOOP]])[[]]m4_dnl");
            processed_file.before().addLine("m4_define( [[M4_YY_OUTFILE_NAME]],[[" +
                                            (!outfilename.empty() ? outfilename : "<stdout>") + "]])m4_dnl");
        }

        processed_file.mergeBeforeAndAfterLines();
        header_file.mergeBeforeAndAfterLines();

        // run m4
        if (preproc_level == 2)
            break;
        m4(processed_file.getLinesRef());
        m4(header_file.getLinesRef());

        // shorten empty lines
        if (preproc_level == 3)
            break;
        processed_file.splitLines();
        header_file.splitLines();
        processed_file.setMaxEmptyLines(1);
        header_file.setMaxEmptyLines(1);

        // fix #line directives
        if (preproc_level == 4)
            break;
        {
            std::regex regex_blank_line("^[[:space:]]*$", std::regex_constants::extended);
            std::regex regex_linedir(R"r(^#line ([[:digit:]]+) "(.*)")r", std::regex_constants::extended);

            auto fix_lines = [&regex_blank_line, &regex_linedir](auto &ctx) {
                int lineno = 1;
                bool in_gen = true; // in generated code
                bool last_was_blank = false;

                for (auto &line : ctx.getLinesRef())
                {
                    String &buf = line.text;
                    std::cmatch m;

                    // Check for #line directive.
                    if (buf[0] == '#' && std::regex_match(buf.c_str(), m, regex_linedir))
                    {
                        // extract the line number and filename
                        String fname = m[2].str();
                        String sfile = !outfilename.empty() ? outfilename.c_str() : "<stdout>";
                        String hfile = !headerfilename.empty() ? headerfilename : "<stdout>";

                        if (fname == sfile || fname == hfile)
                        {
                            // Adjust the line directives.
                            in_gen = true;
                            line = "#line " + std::to_string(lineno + 1) + " \"" + fname + "\"";
                        }
                        else
                        {
                            // it's a #line directive for code we didn't write
                            in_gen = false;
                        }
                        last_was_blank = false;
                    }
                    // squeeze blank lines from generated code
                    else if (in_gen && std::regex_match(buf.c_str(), regex_blank_line))
                    {
                        if (last_was_blank)
                            continue;
                        else
                            last_was_blank = true;
                    }
                    else
                    {
                        // it's a line of normal, non-empty code.
                        last_was_blank = false;
                    }
                    lineno++;
                }
            };

            fix_lines(processed_file);
            fix_lines(header_file);
        }
    } while (0);

    // write files
    {
        assert(!use_stdout);

        FILE *output_file;
        output_file = fopen(outfilename.c_str(), "wb");
        if (!output_file)
            lerr(_("could not create %s"), outfilename.c_str());
        print_lines(output_file, processed_file);
        fclose(output_file);

        if (!headerfilename.empty())
        {
            output_file = fopen(headerfilename.c_str(), "wb");
            if (!output_file)
                lerr(_("could not create %s"), headerfilename.c_str());
            print_lines(output_file, header_file);
            fclose(output_file);
        }
    }

    /* Note, flexend does not return.  It exits with its argument
	 * as status.
	 */
    flexend(0);

    return 0; /* keep compilers/lint happy */
}

void flex_exit(int code)
{
    exit(code);
}

/* check_options - check user-specified options */
void check_options(void)
{
    if (lex_compat)
    {
        if (C_plus_plus)
            flexerror(_("Can't use -+ with -l option"));

        if (fulltbl || fullspd)
            flexerror(_("Can't use -f or -F with -l option"));

        if (reentrant || bison_bridge_lval)
            flexerror(_("Can't use --reentrant or --bison-bridge with -l option"));

        yytext_is_array = true;
        do_yylineno = true;
        use_read = false;
    }

    if (csize == unspecified)
    {
        if ((fulltbl || fullspd) && !useecs)
            csize = DEFAULT_CSIZE;
        else
            csize = CSIZE;
    }

    if (interactive == unspecified)
    {
        if (fulltbl || fullspd)
            interactive = false;
        else
            interactive = true;
    }

    if (fulltbl || fullspd)
    {
        if (usemecs)
            flexerror(_("-Cf/-CF and -Cm don't make sense together"));

        if (interactive)
            flexerror(_("-Cf/-CF and -I are incompatible"));

        if (lex_compat)
            flexerror(_("-Cf/-CF are incompatible with lex-compatibility mode"));

        if (fulltbl && fullspd)
            flexerror(_("-Cf and -CF are mutually exclusive"));
    }

    if (C_plus_plus && fullspd)
        flexerror(_("Can't use -+ with -CF option"));

    if (C_plus_plus && yytext_is_array)
    {
        warn(_("%array incompatible with -+ option"));
        yytext_is_array = false;
    }

    if (C_plus_plus && (reentrant))
        flexerror(_("Options -+ and --reentrant are mutually exclusive."));

    if (C_plus_plus && bison_bridge_lval)
        flexerror(_("bison bridge not supported for the C++ scanner."));

    if (useecs)
    { /* Set up doubly-linked equivalence classes. */

        /* We loop all the way up to csize, since ecgroup[csize] is
		 * the position used for NUL characters.
		 */
        ecgroup[1] = NIL;

        for (int i = 2; i <= csize; ++i)
        {
            ecgroup[i] = i - 1;
            nextecm[i - 1] = i;
        }

        nextecm[csize] = NIL;
    }

    else
    {
        /* Put everything in its own equivalence class. */
        for (int i = 1; i <= csize; ++i)
        {
            ecgroup[i] = i;
            nextecm[i] = BAD_SUBSCRIPT; /* to catch errors */
        }
    }

    if (!ansi_func_defs)
        m4defs_buf.m4define("M4_YY_NO_ANSI_FUNC_DEFS");

    if (!ansi_func_protos)
        m4defs_buf.m4define("M4_YY_NO_ANSI_FUNC_PROTOS");

    if (!extra_type.empty())
        m4defs_buf.m4define("M4_EXTRA_TYPE_DEFS", extra_type);

    if (!use_stdout)
    {
        if (!did_outfilename)
        {
            const char *suffix;

            if (C_plus_plus)
                suffix = "cc";
            else
                suffix = "c";

            snprintf(outfile_path, sizeof(outfile_path), outfile_template, prefix.c_str(), suffix);

            outfilename = outfile_path;
        }

        if (outfilename.empty())
            outfilename = "lex.yy.c";
    }

    /* always generate the tablesverify flag. */
    m4defs_buf.m4define("M4_YY_TABLES_VERIFY", tablesverify ? "1" : "0");

    if (tablesext)
        gentables = false;

    if (tablesverify)
        /* force generation of C tables. */
        gentables = true;

    if (tablesext)
    {
        FILE *tablesout;
        struct yytbl_hdr hdr;
        char *pname = 0;
        int nbytes = 0;

        m4defs_buf.m4define("M4_YY_TABLES_EXTERNAL");

        if (tablesfilename.empty())
        {
            nbytes = prefix.size() + strlen(tablesfile_template) + 2;
            tablesfilename = pname = (decltype(pname))calloc(nbytes, 1);
            snprintf(pname, nbytes, tablesfile_template, prefix);
        }

        if ((tablesout = fopen(tablesfilename.c_str(), "wb")) == NULL)
            lerr(_("could not create %s"), tablesfilename.c_str());
        free(pname);
        tablesfilename.clear();

        yytbl_writer_init(&tableswr, tablesout);

        tablesname = prefix + "tables";
        yytbl_hdr_init(&hdr, flex_version.c_str(), tablesname);

        if (yytbl_hdr_fwrite(&tableswr, &hdr) <= 0)
            flexerror(_("could not write tables header"));
    }

    if (!skelname.empty() && !(skelfile = decltype(skelfile)(skelname.c_str())))
        lerr(_("can't open skeleton file %s"), skelname.c_str());

    if (reentrant)
    {
        m4defs_buf.m4define("M4_YY_REENTRANT");
        if (yytext_is_array)
            m4defs_buf.m4define("M4_YY_TEXT_IS_ARRAY");
    }

    if (bison_bridge_lval)
        m4defs_buf.m4define("M4_YY_BISON_LVAL");

    if (bison_bridge_lloc)
        m4defs_buf.m4define("<M4_YY_BISON_LLOC>");

    m4defs_buf.m4define("M4_YY_PREFIX", prefix);

    if (did_outfilename)
        line_directive_out(true, false);

    if (do_yylineno)
        m4defs_buf.m4define("M4_YY_USE_LINENO");

    /* Create the alignment type. */
    userdef_buf.define("YY_INT_ALIGNED", long_align ? "long int" : "short int");

    /* Define the start condition macros. */
    {
        Context tmpbuf;
        for (int i = 1; i < start_conditions.size(); i++)
            tmpbuf.addLine("#define " + start_conditions[i].name + " " + std::to_string(i - 1));
        m4defs_buf.m4define("M4_YY_SC_DEFS", tmpbuf.getText());
    }

    /* This is where we begin writing to the file. */

    /* Dump the %top code. */
    if (!top_buf.empty())
        outn(top_buf.getText());

    /* Dump the m4 definitions. */
    processed_file += m4defs_buf;

    /* Place a bogus line directive, it will be fixed in the filter. */
	if (gen_line_dirs)
		outn("#line 0 \"M4_YY_OUTFILE_NAME\"\n");

    /* Dump the user defined preproc directives. */
    if (!userdef_buf.empty())
        outn(userdef_buf.getText());

    skelout();
    /* %% [1.0] */
}

/* flexend - terminate flex
 *
 * note
 *    This routine does not return.
 */
void flexend(int exit_status)
{
    static int called_before = -1; /* prevent infinite recursion. */
    int tblsiz;

    if (++called_before)
        flex_exit(exit_status);

    if (skelfile)
    {
        if (skelfile.bad())
            lerr(_("input error reading skeleton file %s"), skelname.c_str());
        skelfile.close();
    }

    if (backing_up_report && backing_up_file)
    {
        if (num_backing_up == 0)
            fprintf(backing_up_file, _("No backing up.\n"));
        else if (fullspd || fulltbl)
            fprintf(backing_up_file,
                    _("%d backing up (non-accepting) states.\n"),
                    num_backing_up);
        else
            fprintf(backing_up_file,
                    _("Compressed tables always back up.\n"));

        if (ferror(backing_up_file))
            lerr(_("error writing backup file %s"),
                 backing_name);

        else if (fclose(backing_up_file))
            lerr(_("error closing backup file %s"),
                 backing_name);
    }

    if (printstats)
    {
        fprintf(stderr, _("%s version %s usage statistics:\n"), program_name.c_str(), flex_version.c_str());
        fprintf(stderr, _("  scanner options: -"));

        if (C_plus_plus)
            putc('+', stderr);
        if (backing_up_report)
            putc('b', stderr);
        if (ddebug)
            putc('d', stderr);
        if (sf_case_ins())
            putc('i', stderr);
        if (lex_compat)
            putc('l', stderr);
        if (posix_compat)
            putc('X', stderr);
        if (performance_report > 0)
            putc('p', stderr);
        if (performance_report > 1)
            putc('p', stderr);
        if (spprdflt)
            putc('s', stderr);
        if (reentrant)
            fputs("--reentrant", stderr);
        if (bison_bridge_lval)
            fputs("--bison-bridge", stderr);
        if (bison_bridge_lloc)
            fputs("--bison-locations", stderr);
        if (use_stdout)
            putc('t', stderr);
        if (printstats)
            putc('v', stderr); /* always true! */
        if (nowarn)
            putc('w', stderr);
        if (interactive == false)
            putc('B', stderr);
        if (interactive == true)
            putc('I', stderr);
        if (!gen_line_dirs)
            putc('L', stderr);
        if (trace)
            putc('T', stderr);

        if (csize == unspecified)
            /* We encountered an error fairly early on, so csize
			 * never got specified.  Define it now, to prevent
			 * bogus table sizes being written out below.
			 */
            csize = 256;

        if (csize == 128)
            putc('7', stderr);
        else
            putc('8', stderr);

        fprintf(stderr, " -C");

        if (long_align)
            putc('a', stderr);
        if (fulltbl)
            putc('f', stderr);
        if (fullspd)
            putc('F', stderr);
        if (useecs)
            putc('e', stderr);
        if (usemecs)
            putc('m', stderr);
        if (use_read)
            putc('r', stderr);

        if (did_outfilename)
            fprintf(stderr, " -o%s", outfilename.c_str());

        if (!skelname.empty())
            fprintf(stderr, " -S%s", skelname.c_str());

        if (prefix != "yy")
            fprintf(stderr, " -P%s", prefix.c_str());

        putc('\n', stderr);

        fprintf(stderr, _("  %d NFA states\n"), (int)nfas.size() - 1);
        fprintf(stderr, _("  %d DFA states (%d words)\n"), (int)dfas.size() - 1, totnst);
        fprintf(stderr, _("  %d rules\n"),
            (int)rules.size() - 1 /* - 1 for dummy rule */ + num_eof_rules - 1 /* - 1 for def. rule */);

        if (num_backing_up == 0)
            fprintf(stderr, _("  No backing up\n"));
        else if (fullspd || fulltbl)
            fprintf(stderr, _("  %d backing-up (non-accepting) states\n"), num_backing_up);
        else
            fprintf(stderr, _("  Compressed tables always back-up\n"));

        if (bol_needed)
            fprintf(stderr, _("  Beginning-of-line patterns used\n"));

        fprintf(stderr, _("  %d start conditions\n"), (int)start_conditions.size() - 1);
        fprintf(stderr, _("  %d epsilon states, %d double epsilon states\n"), numeps, eps2);

        fprintf(stderr, _("  %d character classes\n"), (int)ccls.size() - 1);

        fprintf(stderr, _("  %d state/nextstate pairs created\n"), numsnpairs);
        fprintf(stderr, _("  %d/%d unique/duplicate transitions\n"), numuniq, numdup);

        if (fulltbl)
        {
            tblsiz = (dfas.size() - 1) * numecs;
            fprintf(stderr, _("  %d table entries\n"), tblsiz);
        }
        else
        {
            tblsiz = 2 * ((dfas.size() - 1) + numtemps) + 2 * tblend;

            fprintf(stderr, _("  %d base-def entries created\n"),
                (int)(dfas.size() - 1) + numtemps);
            fprintf(stderr, _("  %d (peak %d) nxt-chk entries created\n"),
                    tblend, peakpairs);
            fprintf(stderr,  _("  %d (peak %d) template nxt-chk entries created\n"),
                    numtemps * nummecs, numtemps * numecs);
            fprintf(stderr, _("  %d empty table entries\n"), nummt);
            fprintf(stderr, _("  %d protos created\n"), numprots);
            fprintf(stderr, _("  %d templates created, %d uses\n"), numtemps, tmpuses);
        }

        if (useecs)
        {
            tblsiz = tblsiz + csize;
            fprintf(stderr,
                    _("  %d/%d equivalence classes created\n"),
                    numecs, csize);
        }

        if (usemecs)
        {
            tblsiz = tblsiz + numecs;
            fprintf(stderr,
                    _("  %d/%d meta-equivalence classes created\n"),
                    nummecs, csize);
        }

        fprintf(stderr, _("  %d (%d saved) hash collisions, %d DFAs equal\n"),
                hshcol, hshsave, dfaeql);
        fprintf(stderr, _("  %d sets of reallocations needed\n"), num_reallocs);
        fprintf(stderr, _("  %d total table entries needed\n"), tblsiz);
    }

    flex_exit(exit_status);
}

/* flexinit - initialize flex */
void flexinit(int argc, char **argv)
{
    int i, sawcmpflag, rv, optind;
    char *arg;
    scanopt_t *sopt;

    printstats = syntaxerror = trace = spprdflt = false;
    lex_compat = posix_compat = C_plus_plus = backing_up_report =
        ddebug = fulltbl = false;
    fullspd = long_align = nowarn = yymore_used = continued_action = false;
    do_yylineno = yytext_is_array = in_rule = reject = do_stdinit = false;
    yymore_really_used = reject_really_used = unspecified;
    interactive = csize = unspecified;
    do_yywrap = gen_line_dirs = usemecs = useecs = true;
    reentrant = bison_bridge_lval = bison_bridge_lloc = false;
    performance_report = 0;
    did_outfilename = 0;
    prefix = "yy";
    use_read = use_stdout = false;
    tablesext = tablesverify = false;
    gentables = true;
    ansi_func_defs = ansi_func_protos = true;

    sawcmpflag = false;

    /* Initialize any buffers. */
    m4defs_buf.addLine("m4_changequote");
    m4defs_buf.addLine("m4_changequote([[, ]])");

    sf_init();

    /* Enable C++ if program name ends with '+'. */
    program_name = basename2(argv[0]);

    if (!program_name.empty() && program_name.back() == '+')
        C_plus_plus = true;

    /* read flags */
    sopt = scanopt_init(flexopts, argc, argv, 0);
    if (!sopt)
    {
        /* This will only happen when flexopts array is altered. */
        fprintf(stderr, _("Internal error. flexopts are malformed.\n"));
        flex_exit(1);
    }

    while ((rv = scanopt(sopt, &arg, &optind)) != 0)
    {

        if (rv < 0)
        {
            /* Scanopt has already printed an option-specific error message. */
            fprintf(stderr, _("Try `%s --help' for more information.\n"), program_name.c_str());
            flex_exit(1);
        }

        switch ((enum flexopt_flag_t)rv)
        {
        case OPT_CPLUSPLUS:
            C_plus_plus = true;
            break;

        case OPT_BATCH:
            interactive = false;
            break;

        case OPT_BACKUP:
            backing_up_report = true;
            break;

        case OPT_DONOTHING:
            break;

        case OPT_COMPRESSION:
            if (!sawcmpflag)
            {
                useecs = false;
                usemecs = false;
                fulltbl = false;
                sawcmpflag = true;
            }

            for (i = 0; arg && arg[i] != '\0'; i++)
                switch (arg[i])
                {
                case 'a':
                    long_align = true;
                    break;

                case 'e':
                    useecs = true;
                    break;

                case 'F':
                    fullspd = true;
                    break;

                case 'f':
                    fulltbl = true;
                    break;

                case 'm':
                    usemecs = true;
                    break;

                case 'r':
                    use_read = true;
                    break;

                default:
                    lerr(_("unknown -C option '%c'"),
                         arg[i]);
                    break;
                }
            break;

        case OPT_DEBUG:
            ddebug = true;
            break;

        case OPT_NO_DEBUG:
            ddebug = false;
            break;

        case OPT_FULL:
            useecs = usemecs = false;
            use_read = fulltbl = true;
            break;

        case OPT_FAST:
            useecs = usemecs = false;
            use_read = fullspd = true;
            break;

        case OPT_HELP:
            usage();
            flex_exit(0);

        case OPT_INTERACTIVE:
            interactive = true;
            break;

        case OPT_CASE_INSENSITIVE:
            sf_set_case_ins(true);
            break;

        case OPT_LEX_COMPAT:
            lex_compat = true;
            break;

        case OPT_POSIX_COMPAT:
            posix_compat = true;
            break;

        case OPT_PREPROC_LEVEL:
            preproc_level = strtol(arg, NULL, 0);
            if (preproc_level < 0)
                preproc_level = 0;
            break;

        case OPT_MAIN:
            userdef_buf.define("YY_MAIN", "1");
            do_yywrap = false;
            break;

        case OPT_NO_MAIN:
            userdef_buf.define("YY_MAIN", "0");
            break;

        case OPT_NO_LINE:
            gen_line_dirs = false;
            break;

        case OPT_OUTFILE:
            outfilename = arg;
            did_outfilename = 1;
            break;

        case OPT_PREFIX:
            prefix = arg;
            break;

        case OPT_PERF_REPORT:
            ++performance_report;
            break;

        case OPT_BISON_BRIDGE:
            bison_bridge_lval = true;
            break;

        case OPT_BISON_BRIDGE_LOCATIONS:
            bison_bridge_lval = bison_bridge_lloc = true;
            break;

        case OPT_REENTRANT:
            reentrant = true;
            break;

        case OPT_NO_REENTRANT:
            reentrant = false;
            break;

        case OPT_SKEL:
            skelname = arg;
            break;

        case OPT_DEFAULT:
            spprdflt = false;
            break;

        case OPT_NO_DEFAULT:
            spprdflt = true;
            break;

        case OPT_STDOUT:
            use_stdout = true;
            break;

        case OPT_NO_UNISTD_H:
            m4defs_buf.m4define("M4_YY_NO_UNISTD_H");
            break;

        case OPT_TABLES_FILE:
            tablesext = true;
            tablesfilename = arg;
            break;

        case OPT_TABLES_VERIFY:
            tablesverify = true;
            break;

        case OPT_TRACE:
            trace = true;
            break;

        case OPT_VERBOSE:
            printstats = true;
            break;

        case OPT_VERSION:
            printf(_("%s %s\n"), program_name.c_str(), flex_version.c_str());
            flex_exit(0);

        case OPT_WARN:
            nowarn = false;
            break;

        case OPT_NO_WARN:
            nowarn = true;
            break;

        case OPT_7BIT:
            csize = 128;
            break;

        case OPT_8BIT:
            csize = CSIZE;
            break;

        case OPT_ALIGN:
            long_align = true;
            break;

        case OPT_NO_ALIGN:
            long_align = false;
            break;

        case OPT_ALWAYS_INTERACTIVE:
            m4defs_buf.m4define("M4_YY_ALWAYS_INTERACTIVE");
            break;

        case OPT_NEVER_INTERACTIVE:
            m4defs_buf.m4define("M4_YY_NEVER_INTERACTIVE");
            break;

        case OPT_ARRAY:
            yytext_is_array = true;
            break;

        case OPT_POINTER:
            yytext_is_array = false;
            break;

        case OPT_ECS:
            useecs = true;
            break;

        case OPT_NO_ECS:
            useecs = false;
            break;

        case OPT_HEADER_FILE:
            headerfilename = arg;
            break;

        case OPT_META_ECS:
            usemecs = true;
            break;

        case OPT_NO_META_ECS:
            usemecs = false;
            break;

        case OPT_PREPROCDEFINE:
        {
            /* arg is "symbol" or "symbol=definition". */
            char *def;

            for (def = arg;
                 *def != '\0' && *def != '='; ++def)
                ;

            userdef_buf.addText("#define ");
            if (*def == '\0')
            {
                userdef_buf.addText(arg);
                userdef_buf.addText(" 1\n");
            }
            else
            {
                userdef_buf.addText(arg, def - arg);
                userdef_buf.addText(" ");
                userdef_buf.addText(def + 1);
                userdef_buf.addText("\n");
            }
        }
        break;

        case OPT_READ:
            use_read = true;
            break;

        case OPT_STACK:
            m4defs_buf.m4define("M4_YY_STACK_USED");
            break;

        case OPT_STDINIT:
            do_stdinit = true;
            break;

        case OPT_NO_STDINIT:
            do_stdinit = false;
            break;

        case OPT_YYCLASS:
            yyclass = arg;
            break;

        case OPT_YYLINENO:
            do_yylineno = true;
            break;

        case OPT_NO_YYLINENO:
            do_yylineno = false;
            break;

        case OPT_YYWRAP:
            do_yywrap = true;
            break;

        case OPT_NO_YYWRAP:
            do_yywrap = false;
            break;

        case OPT_YYMORE:
            yymore_really_used = true;
            break;

        case OPT_NO_YYMORE:
            yymore_really_used = false;
            break;

        case OPT_REJECT:
            reject_really_used = true;
            break;

        case OPT_NO_REJECT:
            reject_really_used = false;
            break;

        case OPT_NO_ANSI_FUNC_DEFS:
            ansi_func_defs = false;
            break;

        case OPT_NO_ANSI_FUNC_PROTOS:
            ansi_func_protos = false;
            break;

        case OPT_NO_YY_PUSH_STATE:
            m4defs_buf.m4define("M4_YY_NO_PUSH_STATE");
            break;
        case OPT_NO_YY_POP_STATE:
            m4defs_buf.m4define("M4_YY_NO_POP_STATE");
            break;
        case OPT_NO_YY_TOP_STATE:
            m4defs_buf.m4define("M4_YY_NO_TOP_STATE");
            break;
        case OPT_NO_UNPUT:
            m4defs_buf.m4define("M4_YY_NO_UNPUT");
            break;
        case OPT_NO_YY_SCAN_BUFFER:
            m4defs_buf.m4define("M4_YY_NO_SCAN_BUFFER");
            break;
        case OPT_NO_YY_SCAN_BYTES:
            m4defs_buf.m4define("M4_YY_NO_SCAN_BYTES");
            break;
        case OPT_NO_YY_SCAN_STRING:
            m4defs_buf.m4define("M4_YY_NO_SCAN_STRING");
            break;
        case OPT_NO_YYGET_EXTRA:
            m4defs_buf.m4define("M4_YY_NO_GET_EXTRA");
            break;
        case OPT_NO_YYSET_EXTRA:
            m4defs_buf.m4define("M4_YY_NO_SET_EXTRA");
            break;
        case OPT_NO_YYGET_LENG:
            m4defs_buf.m4define("M4_YY_NO_GET_LENG");
            break;
        case OPT_NO_YYGET_TEXT:
            m4defs_buf.m4define("M4_YY_NO_GET_TEXT");
            break;
        case OPT_NO_YYGET_LINENO:
            m4defs_buf.m4define("M4_YY_NO_GET_LINENO");
            break;
        case OPT_NO_YYSET_LINENO:
            m4defs_buf.m4define("M4_YY_NO_SET_LINENO");
            break;
        case OPT_NO_YYGET_IN:
            m4defs_buf.m4define("M4_YY_NO_GET_IN");
            break;
        case OPT_NO_YYSET_IN:
            m4defs_buf.m4define("M4_YY_NO_SET_IN");
            break;
        case OPT_NO_YYGET_OUT:
            m4defs_buf.m4define("M4_YY_NO_GET_OUT");
            break;
        case OPT_NO_YYSET_OUT:
            m4defs_buf.m4define("M4_YY_NO_SET_OUT");
            break;
        case OPT_NO_YYGET_LVAL:
            m4defs_buf.m4define("M4_YY_NO_GET_LVAL");
            break;
        case OPT_NO_YYSET_LVAL:
            m4defs_buf.m4define("M4_YY_NO_SET_LVAL");
            break;
        case OPT_NO_YYGET_LLOC:
            m4defs_buf.m4define("M4_YY_NO_GET_LLOC");
            break;
        case OPT_NO_YYSET_LLOC:
            m4defs_buf.m4define("M4_YY_NO_SET_LLOC");
            break;
        case OPT_HEX:
            trace_hex = 1;
        } /* switch */
    }     /* while scanopt() */

    scanopt_destroy(sopt);

    for (auto i = optind; i < argc; i++)
        input_files.push(argv[i]);
    if (input_files.size())
    {
        set_input_file(input_files.front());
        input_files.pop();
    }
    else
        set_input_file(String());

    num_eof_rules = default_rule = 0;
    numas = numsnpairs = tmpuses = 0;
    numecs = numeps = eps2 = num_reallocs = hshcol = dfaeql = totnst = 0;
    numuniq = numdup = hshsave = eofseen = datapos = dataline = 0;
    num_backing_up = onesp = numprots = 0;
    variable_trailing_context_rules = bol_needed = false;

    linenum = sectnum = 1;
    firstprot = NIL;

    /* Used in mkprot() so that the first proto goes in slot 1
	 * of the proto queue.
	 */
    lastprot = 1;
}

/* readin - read in the rules section of the input file(s) */
void readin(void)
{
    static char yy_stdinit[] = "FILE *yyin = stdin, *yyout = stdout;";
    static char yy_nostdinit[] =
        "FILE *yyin = NULL, *yyout = NULL;";

    line_directive_out(false, true);

    if (yyparse())
    {
        pinpoint_message(_("fatal parse error"));
        flexend(1);
    }

    if (syntaxerror)
        flexend(1);

    /* If the user explicitly requested posix compatibility by specifing the
	 * posix-compat option, then we check for conflicting options. However, if
	 * the POSIXLY_CORRECT variable is set, then we quietly make flex as
	 * posix-compatible as possible.  This is the recommended behavior
	 * according to the GNU Coding Standards.
	 *
	 * Note: The posix option was added to flex to provide the posix behavior
	 * of the repeat operator in regular expressions, e.g., `ab{3}'
	 */
    if (posix_compat)
    {
        /* TODO: This is where we try to make flex behave according to
		 * posiz, AND check for conflicting options. How far should we go
		 * with this? Should we disable all the neat-o flex features?
		 */
        /* Update: Estes says no, since other flex features don't violate posix. */
    }

    if (getenv("POSIXLY_CORRECT"))
    {
        posix_compat = true;
    }

    if (backing_up_report)
    {
        backing_up_file = fopen(backing_name, "wb");
        if (backing_up_file == NULL)
            lerr(_("could not create backing-up info file %s"),
                 backing_name);
    }

    else
        backing_up_file = NULL;

    if (yymore_really_used == true)
        yymore_used = true;
    else if (yymore_really_used == false)
        yymore_used = false;

    if (reject_really_used == true)
        reject = true;
    else if (reject_really_used == false)
        reject = false;

    if (performance_report > 0)
    {
        if (lex_compat)
        {
            fprintf(stderr,
                    _("-l AT&T lex compatibility option entails a large performance penalty\n"));
            fprintf(stderr,
                    _(" and may be the actual source of other reported performance penalties\n"));
        }

        else if (do_yylineno)
        {
            fprintf(stderr,
                    _("%%option yylineno entails a performance penalty ONLY on rules that can match newline characters\n"));
        }

        if (performance_report > 1)
        {
            if (interactive)
                fprintf(stderr,
                        _("-I (interactive) entails a minor performance penalty\n"));

            if (yymore_used)
                fprintf(stderr,
                        _("yymore() entails a minor performance penalty\n"));
        }

        if (reject)
            fprintf(stderr,
                    _("REJECT entails a large performance penalty\n"));

        if (variable_trailing_context_rules)
            fprintf(stderr,
                    _("Variable trailing context rules entail a large performance penalty\n"));
    }

    if (reject)
        real_reject = true;

    if (variable_trailing_context_rules)
        reject = true;

    if ((fulltbl || fullspd) && reject)
    {
        if (real_reject)
            flexerror(_("REJECT cannot be used with -f or -F"));
        else if (do_yylineno)
            flexerror(_("%option yylineno cannot be used with REJECT"));
        else
            flexerror(_("variable trailing context rules cannot be used with -f or -F"));
    }

    if (reject)
        processed_file.m4define("M4_YY_USES_REJECT");

    if (!do_yywrap)
    {
        if (!C_plus_plus)
        {
            if (reentrant)
                outn("\n#define yywrap(yyscanner) (/*CONSTCOND*/1)");
            else
                outn("\n#define yywrap() (/*CONSTCOND*/1)");
        }
        outn("#define YY_SKIP_YYWRAP");
    }

    if (ddebug)
        outn("\n#define FLEX_DEBUG");

	OUT_BEGIN_CODE();
	outn("typedef flex_uint8_t YY_CHAR;");
    OUT_END_CODE();

    if (C_plus_plus)
    {
        outn("#define yytext_ptr yytext");

        if (interactive)
            outn("#define YY_INTERACTIVE");
    }

    else
    {
        OUT_BEGIN_CODE();
        /* In reentrant scanner, stdinit is handled in flex.skl. */
        if (do_stdinit)
        {
            if (reentrant)
            {
                outn("#ifdef VMS");
                outn("#ifdef __VMS_POSIX");
                outn("#define YY_STDINIT");
                outn("#endif");
                outn("#else");
                outn("#define YY_STDINIT");
                outn("#endif");
            }

            outn("#ifdef VMS");
            outn("#ifndef __VMS_POSIX");
            outn(yy_nostdinit);
            outn("#else");
            outn(yy_stdinit);
            outn("#endif");
            outn("#else");
            outn(yy_stdinit);
            outn("#endif");
        }

        else
        {
            if (!reentrant)
                outn(yy_nostdinit);
        }
        OUT_END_CODE();
    }

    OUT_BEGIN_CODE();
    if (fullspd)
        outn("typedef yyconst struct yy_trans_info *yy_state_type;");
    else if (!C_plus_plus)
        outn("typedef int yy_state_type;");
    OUT_END_CODE();

    if (lex_compat)
        outn("#define YY_FLEX_LEX_COMPAT");

    if (!C_plus_plus && !reentrant)
    {
        outn("extern int yylineno;");
        OUT_BEGIN_CODE();
        outn("int yylineno = 1;");
        OUT_END_CODE();
    }

    if (C_plus_plus)
    {
        outn("\n#include <FlexLexer.h>");

        if (!do_yywrap)
        {
            outn("\nint yyFlexLexer::yywrap() { return 1; }");
        }

        if (!yyclass.empty())
        {
            outn("int yyFlexLexer::yylex()");
            outn("\t{");
            outn("\tLexerError( \"yyFlexLexer::yylex invoked but %option yyclass used\" );");
            outn("\treturn 0;");
            outn("\t}");

            processed_file.addLine();
            processed_file << "#define YY_DECL int " << yyclass << "::yylex()" << Context::eol;
        }
    }

    else
    {

        /* Watch out: yytext_ptr is a variable when yytext is an array,
		 * but it's a macro when yytext is a pointer.
		 */
        if (yytext_is_array)
        {
            if (!reentrant)
                outn("extern char yytext[];\n");
        }
        else
        {
            if (reentrant)
            {
                outn("#define yytext_ptr yytext_r");
            }
            else
            {
                outn("extern char *yytext;");

                outn("#ifdef yytext_ptr");
                outn("#undef yytext_ptr");
                outn("#endif");
                outn("#define yytext_ptr yytext");
            }
        }

        if (!yyclass.empty())
            flexerror(_("%option yyclass only meaningful for C++ scanners"));
    }

    if (useecs)
        numecs = cre8ecs(nextecm, ecgroup, csize);
    else
        numecs = csize;

    /* Now map the equivalence class for NUL to its expected place. */
    ecgroup[0] = ecgroup[csize];
    NUL_ec = abs(ecgroup[0]);

    if (useecs)
        ccl2ecl();
}

/* extracts basename from path, optionally stripping the extension "\.*"
 * (same concept as /bin/sh `basename`, but different handling of extension). */
static String basename2(String path)
{
    replace_all(path, "\\", "/");
    replace_all(path, ".exe", "");
    auto p = path.rfind("/");
    if (p != path.npos)
        return path.substr(p + 1);
    return path;
}

void usage()
{
    FILE *f = stdout;

    if (!did_outfilename)
    {
        snprintf(outfile_path, sizeof(outfile_path), outfile_template,
                 prefix.c_str(), C_plus_plus ? "cc" : "c");
        outfilename = outfile_path;
    }

    fprintf(f, _("Usage: %s [OPTIONS] [FILE]...\n"), program_name.c_str());
    fprintf(f,
            _("Generates programs that perform pattern-matching on text.\n"
              "\n"
              "Table Compression:\n"
              "  -Ca, --align      trade off larger tables for better memory alignment\n"
              "  -Ce, --ecs        construct equivalence classes\n"
              "  -Cf               do not compress tables; use -f representation\n"
              "  -CF               do not compress tables; use -F representation\n"
              "  -Cm, --meta-ecs   construct meta-equivalence classes\n"
              "  -Cr, --read       use read() instead of stdio for scanner input\n"
              "  -f, --full        generate fast, large scanner. Same as -Cfr\n"
              "  -F, --fast        use alternate table representation. Same as -CFr\n"
              "  -Cem              default compression (same as --ecs --meta-ecs)\n"
              "\n"
              "Debugging:\n"
              "  -d, --debug             enable debug mode in scanner\n"
              "  -b, --backup            write backing-up information to %s\n"
              "  -p, --perf-report       write performance report to stderr\n"
              "  -s, --nodefault         suppress default rule to ECHO unmatched text\n"
              "  -T, --trace             %s should run in trace mode\n"
              "  -w, --nowarn            do not generate warnings\n"
              "  -v, --verbose           write summary of scanner statistics to stdout\n"
              "      --hex               use hexadecimal numbers instead of octal in debug outputs\n"
              "\n"
              "Files:\n"
              "  -o, --outfile=FILE      specify output filename\n"
              "  -S, --skel=FILE         specify skeleton file\n"
              "  -t, --stdout            write scanner on stdout instead of %s\n"
              "      --yyclass=NAME      name of C++ class\n"
              "      --header-file=FILE   create a C header file in addition to the scanner\n"
              "      --tables-file[=FILE] write tables to FILE\n"
              "\n"
              "Scanner behavior:\n"
              "  -7, --7bit              generate 7-bit scanner\n"
              "  -8, --8bit              generate 8-bit scanner\n"
              "  -B, --batch             generate batch scanner (opposite of -I)\n"
              "  -i, --case-insensitive  ignore case in patterns\n"
              "  -l, --lex-compat        maximal compatibility with original lex\n"
              "  -X, --posix-compat      maximal compatibility with POSIX lex\n"
              "  -I, --interactive       generate interactive scanner (opposite of -B)\n"
              "      --yylineno          track line count in yylineno\n"
              "\n"
              "Generated code:\n"
              "  -+,  --c++               generate C++ scanner class\n"
              "  -Dmacro[=defn]           #define macro defn  (default defn is '1')\n"
              "  -L,  --noline            suppress #line directives in scanner\n"
              "  -P,  --prefix=STRING     use STRING as prefix instead of \"yy\"\n"
              "  -R,  --reentrant         generate a reentrant C scanner\n"
              "       --bison-bridge      scanner for bison pure parser.\n"
              "       --bison-locations   include yylloc support.\n"
              "       --stdinit           initialize yyin/yyout to stdin/stdout\n"
              "       --noansi-definitions old-style function definitions\n"
              "       --noansi-prototypes  empty parameter list in prototypes\n"
              "       --nounistd          do not include <unistd.h>\n"
              "       --noFUNCTION        do not generate a particular FUNCTION\n"
              "\n"
              "Miscellaneous:\n"
              "  -c                      do-nothing POSIX option\n"
              "  -n                      do-nothing POSIX option\n"
              "  -?\n"
              "  -h, --help              produce this help message\n"
              "  -V, --version           report %s version\n"),
            backing_name, program_name.c_str(), outfile_path, program_name.c_str());
}
