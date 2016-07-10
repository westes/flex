/* dfa - DFA construction routines */

/*  Copyright (c) 1990 The Regents of the University of California. */
/*  All rights reserved. */

/*  This code is derived from software contributed to Berkeley by */
/*  Vern Paxson. */

/*  The United States Government has rights in this work pursuant */
/*  to contract no. DE-AC03-76SF00098 between the United States */
/*  Department of Energy and the University of California. */

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

#include "dfa.h"

#include "flexdef.h"

#include "ccl.h"
#include "context.h"
#include "ecs.h"
#include "misc.h"
#include "nfa.h"
#include "sym.h"
#include "tables.h"
#include "tblcmp.h"

/* declare functions that have forward references */

void dump_associated_rules(FILE *, int);
void dump_transitions(FILE *, int[]);
void sympartition(const std::vector<int> &, int[], int[]);
int symfollowset(const std::vector<int> &, int, std::vector<int> &);

/* Construct the epsilon closure of a set of ndfa states. */
void epsclosure(std::vector<int> &, int *, std::vector<int> &, int *, int *);

/* Converts a set of ndfa states into a dfa state. */
int snstods(std::vector<int> &, int, std::vector<int> &, int, int, int *);

/* Check to see if NFA state set constitutes "dangerous" trailing context. */
void check_trailing_context(std::vector<int> &, int, std::vector<int> &, int);

/* check_for_backing_up - check a DFA state for backing up
 *
 * synopsis
 *     void check_for_backing_up( int ds, int state[numecs] );
 *
 * ds is the number of the state to check and state[] is its out-transitions,
 * indexed by equivalence class.
 */
void check_for_backing_up(int ds, int state[])
{
    if ((reject && !dfas[ds].acc_set) || (!reject && !dfas[ds].acc_state))
    {
        /* state is non-accepting */
        ++num_backing_up;

        if (backing_up_report)
        {
            fprintf(backing_up_file,
                    _("State #%d is non-accepting -\n"), ds);

            /* identify the state */
            dump_associated_rules(backing_up_file, ds);

            /* Now identify it further using the out- and
			 * jam-transitions.
			 */
            dump_transitions(backing_up_file, state);

            putc('\n', backing_up_file);
        }
    }
}

/* check_trailing_context - check to see if NFA state set constitutes
 *                          "dangerous" trailing context
 *
 * synopsis
 *    void check_trailing_context( int nfa_states[num_states+1], int num_states,
 *				int accset[nacc+1], int nacc );
 *
 * NOTES
 *  Trailing context is "dangerous" if both the head and the trailing
 *  part are of variable size \and/ there's a DFA state which contains
 *  both an accepting state for the head part of the rule and NFA states
 *  which occur after the beginning of the trailing context.
 *
 *  When such a rule is matched, it's impossible to tell if having been
 *  in the DFA state indicates the beginning of the trailing context or
 *  further-along scanning of the pattern.  In these cases, a warning
 *  message is issued.
 *
 *    nfa_states[1 .. num_states] is the list of NFA states in the DFA.
 *    accset[1 .. nacc] is the list of accepting numbers for the DFA state.
 */
void check_trailing_context(std::vector<int> &nfa_states, int num_states, std::vector<int> &accset, int nacc)
{
    for (int i = 1; i <= num_states; ++i)
    {
        int ns = nfa_states[i];
        auto type = nfas[ns].state_type;
        int ar = nfas[ns].assoc_rule;

        if (type == StateType::Normal || rules[ar].type != RuleType::Variable)
        {
            /* do nothing */
        }
        else if (type == StateType::TrailingContext)
        {
            /* Potential trouble.  Scan set of accepting numbers
			 * for the one marking the end of the "head".  We
			 * assume that this looping will be fairly cheap
			 * since it's rare that an accepting number set
			 * is large.
			 */
            for (int j = 1; j <= nacc; ++j)
            {
                if (accset[j] & YY_TRAILING_HEAD_MASK)
                {
                    line_warning(_("dangerous trailing context"), rules[ar].linenum);
                    return;
                }
            }
        }
    }
}

/* dump_associated_rules - list the rules associated with a DFA state
 *
 * Goes through the set of NFA states associated with the DFA and
 * extracts the first MAX_ASSOC_RULES unique rules, sorts them,
 * and writes a report to the given file.
 */
void dump_associated_rules(FILE *file, int ds)
{
    int i, j;
    int num_associated_rules = 0;
    int rule_set[MAX_ASSOC_RULES + 1];
    auto &dset = dfas[ds].dss;
    int size = dset.size();

    for (i = 1; i < size; ++i)
    {
        int rule_num = rules[nfas[dset[i]].assoc_rule].linenum;

        for (j = 1; j <= num_associated_rules; ++j)
            if (rule_num == rule_set[j])
                break;

        if (j > num_associated_rules)
        {
            /* new rule */
            if (num_associated_rules < MAX_ASSOC_RULES)
                rule_set[++num_associated_rules] =
                    rule_num;
        }
    }

    qsort(&rule_set[1], (size_t)num_associated_rules, sizeof(rule_set[1]), intcmp);

    fprintf(file, _(" associated rule line numbers:"));

    for (i = 1; i <= num_associated_rules; ++i)
    {
        if (i % 8 == 1)
            putc('\n', file);

        fprintf(file, "\t%d", rule_set[i]);
    }

    putc('\n', file);
}

/* dump_transitions - list the transitions associated with a DFA state
 *
 * synopsis
 *     dump_transitions( FILE *file, int state[numecs] );
 *
 * Goes through the set of out-transitions and lists them in human-readable
 * form (i.e., not as equivalence classes); also lists jam transitions
 * (i.e., all those which are not out-transitions, plus EOF).  The dump
 * is done to the given file.
 */
void dump_transitions(FILE *file, int state[])
{
    int i, ec;
    int out_char_set[CSIZE];

    for (i = 0; i < csize; ++i)
    {
        ec = abs(ecgroup[i]);
        out_char_set[i] = state[ec];
    }

    fprintf(file, _(" out-transitions: "));

    list_character_set(file, out_char_set);

    /* now invert the members of the set to get the jam transitions */
    for (i = 0; i < csize; ++i)
        out_char_set[i] = !out_char_set[i];

    fprintf(file, _("\n jam-transitions: EOF "));

    list_character_set(file, out_char_set);

    putc('\n', file);
}

/* epsclosure - construct the epsilon closure of a set of ndfa states
 *
 * synopsis
 *    int *epsclosure( int t[num_states], int *numstates_addr,
 *			int accset[num_rules+1], int *nacc_addr,
 *			int *hashval_addr );
 *
 * NOTES
 *  The epsilon closure is the set of all states reachable by an arbitrary
 *  number of epsilon transitions, which themselves do not have epsilon
 *  transitions going out, unioned with the set of states which have non-null
 *  accepting numbers.  t is an array of size numstates of nfa state numbers.
 *  Upon return, t holds the epsilon closure and *numstates_addr is updated.
 *  accset holds a list of the accepting numbers, and the size of accset is
 *  given by *nacc_addr.  t may be subjected to reallocation if it is not
 *  large enough to hold the epsilon closure.
 *
 *  hashval is the hash value for the dfa corresponding to the state set.
 */
void epsclosure(std::vector<int> &t, int *ns_addr, std::vector<int> &accset, int *nacc_addr, int *hv_addr)
{
    int stkpos, ns, tsp;
    int numstates = *ns_addr, nacc, hashval, transsym, nfaccnum;
    int stkend, nstate;
    static std::vector<int> stk(dfas.capacity() + 1);

    /* Enough so that if it's subtracted from an NFA state number, the result
    * is guaranteed to be negative.
    */
//#define MARKER_DIFFERENCE (maximum_mns + 2)
//#define MARKER_DIFFERENCE (nfas.size() + 2)
#define MARKER_DIFFERENCE (1'000'000'000)

#define MARK_STATE(state)                                  \
    do                                                     \
    {                                                      \
        nfas[state].trans1 = nfas[state].trans1 - MARKER_DIFFERENCE; \
    } while (0)

#define IS_MARKED(state) (nfas[state].trans1 < 0)

#define UNMARK_STATE(state)                                \
    do                                                     \
    {                                                      \
        nfas[state].trans1 = nfas[state].trans1 + MARKER_DIFFERENCE; \
    } while (0)

#define CHECK_ACCEPT(state)            \
    do                                 \
    {                                  \
        nfaccnum = nfas[state].accptnum;    \
        if (nfaccnum != NIL)           \
            accset[++nacc] = nfaccnum; \
    } while (0)

#define DO_REALLOCATION(x)                                                 \
    do                                                                    \
    {                                                                     \
        t.resize(x);     \
        stk.resize(x); \
    } while (0)

#define PUT_ON_STACK(state)                   \
    do                                        \
    {                                         \
        if (++stkend + 1 > stk.size()) \
            DO_REALLOCATION(stkend + 1);                \
        stk[stkend] = state;                  \
        MARK_STATE(state);                    \
    } while (0)

#define ADD_STATE(state)                         \
    do                                           \
    {                                            \
        if (++numstates + 1 > stk.size()) \
            DO_REALLOCATION(numstates + 1);                   \
        t[numstates] = state;                    \
        hashval += state;                        \
    } while (0)

#define STACK_STATE(state)                                      \
    do                                                          \
    {                                                           \
        PUT_ON_STACK(state);                                    \
        CHECK_ACCEPT(state);                                    \
        if (nfaccnum != NIL || nfas[state].transchar != SYM_EPSILON) \
            ADD_STATE(state);                                   \
    } while (0)

    nacc = stkend = hashval = 0;

    for (nstate = 1; nstate <= numstates; ++nstate)
    {
        ns = t[nstate];

        /* The state could be marked if we've already pushed it onto
		 * the stack.
		 */
        if (!IS_MARKED(ns))
        {
            PUT_ON_STACK(ns);
            CHECK_ACCEPT(ns);
            hashval += ns;
        }
    }

    for (stkpos = 1; stkpos <= stkend; ++stkpos)
    {
        ns = stk[stkpos];
        transsym = nfas[ns].transchar;

        if (transsym == SYM_EPSILON)
        {
            tsp = nfas[ns].trans1 + MARKER_DIFFERENCE;

            if (tsp != NO_TRANSITION)
            {
                if (!IS_MARKED(tsp))
                    STACK_STATE(tsp);

                tsp = nfas[ns].trans2;

                if (tsp != NO_TRANSITION && !IS_MARKED(tsp))
                    STACK_STATE(tsp);
            }
        }
    }

    /* Clear out "visit" markers. */
    for (stkpos = 1; stkpos <= stkend; ++stkpos)
    {
        if (IS_MARKED(stk[stkpos]))
            UNMARK_STATE(stk[stkpos]);
        else
            flexfatal(_("consistency check failed in epsclosure()"));
    }

    *ns_addr = numstates;
    *hv_addr = hashval;
    *nacc_addr = nacc;
}

/* ntod - convert an ndfa to a dfa
 *
 * Creates the dfa corresponding to the ndfa we've constructed.  The
 * dfa starts out in state #1.
 */
void ntod()
{
    int newds;
    int sym;
    int num_full_table_rows = 0; /* used only for -f */
    std::vector<int> nset;
    int i, comstate, comfreq, targ;
    int num_start_states;
    int todo_head, todo_next;

    struct yytbl_data *yynxt_tbl = 0;
    flex_int32_t *yynxt_data = 0, yynxt_curr = 0;

    /* Note that the following are indexed by *equivalence classes*
	 * and not by characters.  Since equivalence classes are indexed
	 * beginning with 1, even if the scanner accepts NUL's, this
	 * means that (since every character is potentially in its own
	 * equivalence class) these arrays must have room for indices
	 * from 1 to CSIZE, so their size must be CSIZE + 1.
     */
    int symlist[CSIZE + 1] = { 0 };
    int duplist[CSIZE + 1] = { 0 };
    int state[CSIZE + 1];
    int targfreq[CSIZE + 1] = { 0 };
    int targstate[CSIZE + 1];

    /* accset needs to be large enough to hold all of the rules present
	 * in the input, *plus* their YY_TRAILING_HEAD_MASK variants.
     */
    std::vector<int> accset(rules.size() * 2, 0);
    nset.resize(dfas.capacity() + 1);

    /* The "todo" queue is represented by the head, which is the DFA
	 * state currently being processed, and the "next", which is the
	 * next DFA state number available (not in use).  We depend on the
	 * fact that snstods() returns DFA's \in increasing order/, and thus
	 * need only know the bounds of the dfas to be processed.
	 */
    todo_head = todo_next = 0;

    if (trace)
    {
        dumpnfa(start_conditions[1].set);
        fputs(_("\n\nDFA Dump:\n\n"), stderr);
    }

    inittbl();

    /* Check to see whether we should build a separate table for
	 * transitions on NUL characters.  We don't do this for full-speed
	 * (-F) scanners, since for them we don't have a simple state
	 * number lying around with which to index the table.  We also
	 * don't bother doing it for scanners unless (1) NUL is in its own
	 * equivalence class (indicated by a positive value of
	 * ecgroup[NUL]), (2) NUL's equivalence class is the last
	 * equivalence class, and (3) the number of equivalence classes is
	 * the same as the number of characters.  This latter case comes
	 * about when useecs is false or when it's true but every character
	 * still manages to land in its own class (unlikely, but it's
	 * cheap to check for).  If all these things are true then the
	 * character code needed to represent NUL's equivalence class for
	 * indexing the tables is going to take one more bit than the
	 * number of characters, and therefore we won't be assured of
	 * being able to fit it into a YY_CHAR variable.  This rules out
	 * storing the transitions in a compressed table, since the code
	 * for interpreting them uses a YY_CHAR variable (perhaps it
	 * should just use an integer, though; this is worth pondering ...
	 * ###).
	 *
	 * Finally, for full tables, we want the number of entries in the
	 * table to be a power of two so the array references go fast (it
	 * will just take a shift to compute the major index).  If
	 * encoding NUL's transitions in the table will spoil this, we
	 * give it its own table (note that this will be the case if we're
	 * not using equivalence classes).
	 */

    /* Note that the test for ecgroup[0] == numecs below accomplishes
	 * both (1) and (2) above
	 */
    if (!fullspd && ecgroup[0] == numecs)
    {
        /* NUL is alone in its equivalence class, which is the
		 * last one.
		 */
        int use_NUL_table = (numecs == csize);

        if (fulltbl && !use_NUL_table)
        {
            /* We still may want to use the table if numecs
			 * is a power of 2.
			 */
            int power_of_two;

            for (power_of_two = 1; power_of_two <= csize; power_of_two *= 2)
                if (numecs == power_of_two)
                {
                    use_NUL_table = true;
                    break;
                }
        }

        if (use_NUL_table)
            nultrans = true;

        /* From now on, nultrans != nil indicates that we're
		 * saving null transitions for later, separate encoding.
		 */
    }

    if (fullspd)
    {
        for (i = 0; i <= numecs; ++i)
            state[i] = 0;

        place_state(state, 0, 0);
        dfas[0].acc_state = 0;
    }
    else if (fulltbl)
    {
        if (nultrans)
            /* We won't be including NUL's transitions in the
			 * table, so build it for entries from 0 .. numecs - 1.
			 */
            num_full_table_rows = numecs;

        else
            /* Take into account the fact that we'll be including
			 * the NUL entries in the transition table.  Build it
			 * from 0 .. numecs.
			 */
            num_full_table_rows = numecs + 1;

        /* Begin generating yy_nxt[][]
		 * This spans the entire LONG function.
		 * This table is tricky because we don't know how big it will be.
		 * So we'll have to realloc() on the way...
		 * we'll wait until we can calculate yynxt_tbl->td_hilen.
		 */
        yynxt_tbl = (decltype(yynxt_tbl))calloc(1, sizeof(struct yytbl_data));

        yytbl_data_init(yynxt_tbl, YYTD_ID_NXT);
        yynxt_tbl->td_hilen = 1;
        yynxt_tbl->td_lolen = num_full_table_rows;
        yynxt_tbl->td_data = yynxt_data = (decltype(yynxt_data))
            calloc(yynxt_tbl->td_lolen *
                       yynxt_tbl->td_hilen,
                   sizeof(flex_int32_t));
        yynxt_curr = 0;

        yydmap_buf.addLine(String() + "\t{YYTD_ID_NXT, (void**)&yy_nxt, sizeof(" + (long_align ? "flex_int32_t" : "flex_int16_t") + ")},");

        /* Unless -Ca, declare it "short" because it's a real
		 * long-shot that that won't be large enough.
		 */
        if (gentables)
        {
            processed_file << "static yyconst " << (long_align ? "flex_int32_t" : "flex_int16_t")
                << " yy_nxt[][" << num_full_table_rows << "] =" << Context::eol;
            processed_file << "    {" << Context::eol;
        }
        else
        {
            processed_file << "#undef YY_NXT_LOLEN" << Context::eol;
            processed_file << "#define YY_NXT_LOLEN (" << num_full_table_rows << ")" << Context::eol;
            processed_file << "static yyconst " << (long_align ? "flex_int32_t" : "flex_int16_t") << " *yy_nxt = 0;" << Context::eol;
        }

        if (gentables)
            outn("    {");

        /* Generate 0 entries for state #0. */
        for (i = 0; i < num_full_table_rows; ++i)
        {
            mkdata(0);
            yynxt_data[yynxt_curr++] = 0;
        }

        dataflush();
        if (gentables)
            outn("    },\n");
    }

    /* Create the first states. */

    num_start_states = (start_conditions.size() - 1) * 2;

    for (i = 1; i <= num_start_states; ++i)
    {
        int numstates = 1;

        /* For each start condition, make one state for the case when
		 * we're at the beginning of the line (the '^' operator) and
		 * one for the case when we're not.
		 */
        if (i % 2 == 1)
            nset[numstates] = start_conditions[(i / 2) + 1].set;
        else
            nset[numstates] = mkbranch(start_conditions[i / 2].bol, start_conditions[i / 2].set);

        int hashval = 0;
        int nacc = 0;
        epsclosure(nset, &numstates, accset, &nacc, &hashval);

        int ds = 0;
        if (snstods(nset, numstates, accset, nacc, hashval, &ds))
        {
            numas += nacc;
            totnst += numstates;
            ++todo_next;

            if (variable_trailing_context_rules && nacc > 0)
                check_trailing_context(nset, numstates, accset, nacc);
        }
    }

    if (!fullspd)
    {
        if (!snstods(nset, 0, accset, 0, 0, &end_of_buffer_state))
            flexfatal(_("could not create unique end-of-buffer state"));

        ++numas;
        ++num_start_states;
        ++todo_next;
    }

    while (todo_head < todo_next)
    {
        int targptr = 0;
        int totaltrans = 0;

        for (i = 1; i <= numecs; ++i)
            state[i] = 0;

        int ds = ++todo_head;

        if (trace)
            fprintf(stderr, _("state # %d:\n"), ds);

        sympartition(dfas[ds].dss, symlist, duplist);

        for (sym = 1; sym <= numecs; ++sym)
        {
            if (symlist[sym])
            {
                symlist[sym] = 0;

                if (duplist[sym] == NIL)
                {
                    /* Symbol has unique out-transitions. */
                    int numstates = symfollowset(dfas[ds].dss, sym, nset);

                    int hashval = 0;
                    int nacc = 0;
                    epsclosure(nset, &numstates, accset, &nacc, &hashval);

                    if (snstods(nset, numstates, accset, nacc, hashval, &newds))
                    {
                        totnst = totnst + numstates;
                        ++todo_next;
                        numas += nacc;

                        if (variable_trailing_context_rules && nacc > 0)
                            check_trailing_context(nset, numstates, accset, nacc);
                    }

                    state[sym] = newds;

                    if (trace)
                        fprintf(stderr, "\t%d\t%d\n", sym, newds);

                    targfreq[++targptr] = 1;
                    targstate[targptr] = newds;
                    ++numuniq;
                }
                else
                {
                    /* sym's equivalence class has the same
					 * transitions as duplist(sym)'s
					 * equivalence class.
					 */
                    targ = state[duplist[sym]];
                    state[sym] = targ;

                    if (trace)
                        fprintf(stderr, "\t%d\t%d\n", sym, targ);

                    /* Update frequency count for
					 * destination state.
					 */
                    i = 0;
                    while (targstate[++i] != targ)
                        ;

                    ++targfreq[i];
                    ++numdup;
                }

                ++totaltrans;
                duplist[sym] = NIL;
            }
        }

        numsnpairs += totaltrans;

        if (ds > num_start_states)
            check_for_backing_up(ds, state);

        if (nultrans)
        {
            dfas[ds].nultrans = state[NUL_ec];
            state[NUL_ec] = 0; /* remove transition */
        }

        if (fulltbl)
        {

            /* Each time we hit here, it's another td_hilen, so we realloc. */
            yynxt_tbl->td_hilen++;
            yynxt_tbl->td_data = yynxt_data =
                (decltype(yynxt_data))realloc(yynxt_data,
                                              yynxt_tbl->td_hilen *
                                                  yynxt_tbl->td_lolen *
                                                  sizeof(flex_int32_t));

            if (gentables)
                outn("    {");

            /* Supply array's 0-element. */
            if (ds == end_of_buffer_state)
            {
                mkdata(-end_of_buffer_state);
                yynxt_data[yynxt_curr++] =
                    -end_of_buffer_state;
            }
            else
            {
                mkdata(end_of_buffer_state);
                yynxt_data[yynxt_curr++] =
                    end_of_buffer_state;
            }

            for (i = 1; i < num_full_table_rows; ++i)
            {
                /* Jams are marked by negative of state
				 * number.
				 */
                mkdata(state[i] ? state[i] : -ds);
                yynxt_data[yynxt_curr++] =
                    state[i] ? state[i] : -ds;
            }

            dataflush();
            if (gentables)
                outn("    },\n");
        }
        else if (fullspd)
            place_state(state, ds, totaltrans);
        else if (ds == end_of_buffer_state)
            /* Special case this state to make sure it does what
			 * it's supposed to, i.e., jam on end-of-buffer.
			 */
            stack1(ds, 0, 0, JAMSTATE);
        else
        { /* normal, compressed state */

            /* Determine which destination state is the most
			 * common, and how many transitions to it there are.
			 */

            comfreq = 0;
            comstate = 0;

            for (i = 1; i <= targptr; ++i)
            {
                if (targfreq[i] > comfreq)
                {
                    comfreq = targfreq[i];
                    comstate = targstate[i];
                }
            }

            bldtbl(state, ds, totaltrans, comstate, comfreq);
        }
    }

    if (fulltbl)
    {
        dataend();
        if (tablesext)
        {
            yytbl_data_compress(yynxt_tbl);
            if (yytbl_data_fwrite(&tableswr, yynxt_tbl) < 0)
                flexerror(_("Could not write yynxt_tbl[][]"));
        }
        if (yynxt_tbl)
        {
            yytbl_data_destroy(yynxt_tbl);
            yynxt_tbl = 0;
        }
    }
    else if (!fullspd)
    {
        cmptmps(); /* create compressed template entries */

        /* Create tables for all the states with only one
		 * out-transition.
		 */
        while (onesp > 0)
        {
            mk1tbl(onestate[onesp], onesym[onesp],
                   onenext[onesp], onedef[onesp]);
            --onesp;
        }

        mkdeftbl();
    }
}

/* snstods - converts a set of ndfa states into a dfa state
 *
 * synopsis
 *    is_new_state = snstods( int sns[numstates], int numstates,
 *				int accset[num_rules+1], int nacc,
 *				int hashval, int *newds_addr );
 *
 * On return, the dfa state number is in newds.
 */
int snstods(std::vector<int> &sns, int numstates, std::vector<int> &accset, int nacc, int hashval, int *newds_addr)
{
    int didsort = 0;
    int i, j;

    for (i = 1; i < dfas.size(); ++i)
    {
        if (hashval == dfas[i].hash)
        {
            if (numstates == dfas[i].dss.size() - 1)
            {
                const auto &oldsns = dfas[i].dss;

                if (!didsort)
                {
                    /* We sort the states in sns so we
                     * can compare it to oldsns quickly.
                     */
                    qsort(&sns[1], (size_t)numstates, sizeof(sns[1]), intcmp);
                    didsort = 1;
                }

                for (j = 1; j <= numstates; ++j)
                {
                    if (sns[j] != oldsns[j])
                        break;
                }

                if (j > numstates)
                {
                    ++dfaeql;
                    *newds_addr = i;
                    return 0;
                }

                ++hshcol;
            }
            else
                ++hshsave;
        }
    }

    /* Make a new dfa. */
    dfas.emplace_back();
    auto &dfa = dfas.back();

    /* If we haven't already sorted the states in sns, we do so now,
	 * so that future comparisons with it can be made quickly.
	 */
    if (!didsort)
        qsort(&sns[1], (size_t)numstates, sizeof(sns[1]), intcmp);

    dfa.dss.resize(numstates + 1, 0);
    for (i = 1; i <= numstates; ++i)
        dfa.dss[i] = sns[i];

    dfa.hash = hashval;

    if (nacc == 0)
    {
        if (!reject)
            dfa.acc_state = 0;
    }
    else if (reject)
    {
        /* We sort the accepting set in increasing order so the
		 * disambiguating rule that the first rule listed is considered
		 * match in the event of ties will work.
		 */
        qsort(&accset[1], (size_t)nacc, sizeof(accset[1]), intcmp);

        /* Save the accepting set for later */
        dfa.acc_set = std::make_shared<Dfa::AccSet>(nacc + 1);
        for (i = 1; i <= nacc; ++i)
        {
            (*dfa.acc_set)[i] = accset[i];

            if (accset[i] < rules.size())
                /* Who knows, perhaps a REJECT can yield this rule. */
                rules[accset[i]].useful = true;
        }
    }
    else
    {
        /* Find lowest numbered rule so the disambiguating rule will work. */
        j = rules.size();

        for (i = 1; i <= nacc; ++i)
        {
            if (accset[i] < j)
                j = accset[i];
        }

        dfa.acc_state = j;

        if (j < rules.size())
            rules[j].useful = true;
    }

    *newds_addr = dfas.size() - 1;

    return 1;
}

/* symfollowset - follow the symbol transitions one step
 *
 * synopsis
 *    numstates = symfollowset( int ds[current_max_dfa_size], int dsize,
 *				int transsym, int nset[current_max_dfa_size] );
 */
int symfollowset(const std::vector<int> &ds, int transsym, std::vector<int> &nset)
{
    int numstates = 0;

    for (int i = 1; i < ds.size(); ++i)
    {
        /* for each nfa state ns in the state set of ds */
        int ns = ds[i];
        int sym = nfas[ns].transchar;
        int tsp = nfas[ns].trans1;

        if (sym < 0)
        {
            /* it's a character class */
            sym = -sym;

            auto &ccl = ccls[sym];

            if (ccl.ng)
            {
                for (auto ch : ccl.table)
                {
                    /* Loop through negated character class. */
                    if (ch == 0)
                        ch = NUL_ec;

                    if (ch > transsym)
                        /* Transsym isn't in negated ccl. */
                        break;
                    else if (ch == transsym)
                    {
                        /* next 2 */
                        goto bottom;
                    }
                }

                /* Didn't find transsym in ccl. */
                nset[++numstates] = tsp;
            }
            else
            {
                for (auto ch : ccl.table)
                {
                    if (ch == 0)
                        ch = NUL_ec;

                    if (ch > transsym)
                        break;
                    else if (ch == transsym)
                    {
                        nset[++numstates] = tsp;
                        break;
                    }
                }
            }
        }
        else if (sym == SYM_EPSILON)
        {
            /* do nothing */
        }
        else if (abs(ecgroup[sym]) == transsym)
            nset[++numstates] = tsp;

    bottom:;
    }

    return numstates;
}

/* sympartition - partition characters with same out-transitions
 *
 * synopsis
 *    sympartition( int ds[current_max_dfa_size], int numstates,
 *			int symlist[numecs], int duplist[numecs] );
 */
void sympartition(const std::vector<int> &ds, int symlist[], int duplist[])
{
    int dupfwd[CSIZE + 1];

    /* Partitioning is done by creating equivalence classes for those
	 * characters which have out-transitions from the given state.  Thus
	 * we are really creating equivalence classes of equivalence classes.
	 */
    for (int i = 1; i <= numecs; ++i)
    {
        /* initialize equivalence class list */
        duplist[i] = i - 1;
        dupfwd[i] = i + 1;
    }

    duplist[1] = NIL;
    dupfwd[numecs] = NIL;

    for (int i = 1; i < ds.size(); ++i)
    {
        int ns = ds[i];
        int tch = nfas[ns].transchar;

        if (tch != SYM_EPSILON)
        {
            if (tch < -((int)ccls.size() - 1) || tch >= csize)
            {
                flexfatal(_("bad transition character detected in sympartition()"));
            }

            if (tch >= 0)
            {
                /* character transition */
                int ec = ecgroup[tch];

                mkechar(ec, dupfwd, duplist);
                symlist[ec] = 1;
            }
            else
            {
                /* character class */
                tch = -tch;

                auto &ccl = ccls[tch];

                mkeccl(ccl.table, dupfwd, duplist, numecs, NUL_ec);

                if (ccl.ng)
                {
                    int j = 0;

                    for (auto ich : ccl.table)
                    {
                        if (ich == 0)
                            ich = NUL_ec;

                        for (++j; j < ich; ++j)
                            symlist[j] = 1;
                    }

                    for (++j; j <= numecs; ++j)
                        symlist[j] = 1;
                }
                else
                {
                    for (auto ich : ccl.table)
                    {
                        if (ich == 0)
                            ich = NUL_ec;

                        symlist[ich] = 1;
                    }
                }
            }
        }
    }
}
