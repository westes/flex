/* nfa - NFA construction routines */

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

#include "nfa.h"

#include "flexdef.h"

#include "ecs.h"
#include "misc.h"

/* declare functions that have forward references */

int dupmachine(int);
void mkxtion(int, int);

/* add_accept - add an accepting state to a machine
 *
 * accepting_number becomes mach's accepting number.
 */
void add_accept(int mach, int accepting_number)
{
    /* Hang the accepting number off an epsilon state.  if it is associated
	 * with a state that has a non-epsilon out-transition, then the state
	 * will accept BEFORE it makes that transition, i.e., one character
	 * too soon.
	 */
    auto f = nfas[mach].finalst;
    if (nfas[f].transchar == SYM_EPSILON)
        nfas[f].accptnum = accepting_number;
    else
    {
        int astate = mkstate(SYM_EPSILON);
        nfas[astate].accptnum = accepting_number;
        link_machines(mach, astate);
    }
}

/* copysingl - make a given number of copies of a singleton machine
 *
 * synopsis
 *
 *   newsng = copysingl( singl, num );
 *
 *     newsng - a new singleton composed of num copies of singl
 *     singl  - a singleton machine
 *     num    - the number of copies of singl to be present in newsng
 */
int copysingl(int singl, int num)
{
    int copy, i;

    copy = mkstate(SYM_EPSILON);

    for (i = 1; i <= num; ++i)
        copy = link_machines(copy, dupmachine(singl));

    return copy;
}

/* dumpnfa - debugging routine to write out an nfa */
void dumpnfa(int state1)
{
    fprintf(stderr, _("\n\n********** beginning dump of nfa with start state %d\n"), state1);

    /* We probably should loop starting at firstst[state1] and going to
	 * lastst[state1], but they're not maintained properly when we "or"
	 * all of the rules together.  So we use our knowledge that the machine
	 * starts at state 1 and ends at lastnfa.
	 */

    /* for ( ns = firstst[state1]; ns <= lastst[state1]; ++ns ) */
    for (size_t ns = 1; ns < nfas.size(); ++ns)
    {
        fprintf(stderr, _("state # %4d\t"), (int)ns);

        int sym = nfas[ns].transchar;
        int tsp1 = nfas[ns].trans1;
        int tsp2 = nfas[ns].trans2;
        int anum = nfas[ns].accptnum;

        fprintf(stderr, "%3d:  %4d, %4d", sym, tsp1, tsp2);

        if (anum != NIL)
            fprintf(stderr, "  [%d]", anum);

        fprintf(stderr, "\n");
    }

    fprintf(stderr, _("********** end of dump\n"));
}

/* dupmachine - make a duplicate of a given machine
 *
 * synopsis
 *
 *   copy = dupmachine( mach );
 *
 *     copy - holds duplicate of mach
 *     mach - machine to be duplicated
 *
 * note that the copy of mach is NOT an exact duplicate; rather, all the
 * transition states values are adjusted so that the copy is self-contained,
 * as the original should have been.
 *
 * also note that the original MUST be contiguous, with its low and high
 * states accessible by the arrays firstst and lastst
 */
int dupmachine(int mach)
{
    int i, init, state_offset;
    int state = 0;
    int last = nfas[mach].lastst;

    for (i = nfas[mach].firstst; i <= last; ++i)
    {
        state = mkstate(nfas[i].transchar);

        if (nfas[i].trans1 != NO_TRANSITION)
        {
            mkxtion(nfas[state].finalst, nfas[i].trans1 + state - i);

            if (nfas[i].transchar == SYM_EPSILON && nfas[i].trans2 != NO_TRANSITION)
                mkxtion(nfas[state].finalst, nfas[i].trans2 + state - i);
        }

        nfas[state].accptnum = nfas[i].accptnum;
    }

    if (state == 0)
        flexfatal(_("empty machine in dupmachine()"));

    state_offset = state - i + 1;

    init = mach + state_offset;
    nfas[init].firstst = nfas[mach].firstst + state_offset;
    nfas[init].finalst = nfas[mach].finalst + state_offset;
    nfas[init].lastst = nfas[mach].lastst + state_offset;

    return init;
}

/* finish_rule - finish up the processing for a rule
 *
 * An accepting number is added to the given machine.  If variable_trail_rule
 * is true then the rule has trailing context and both the head and trail
 * are variable size.  Otherwise if headcnt or trailcnt is non-zero then
 * the machine recognizes a pattern with trailing context and headcnt is
 * the number of characters in the matched part of the pattern, or zero
 * if the matched part has variable length.  trailcnt is the number of
 * trailing context characters in the pattern, or zero if the trailing
 * context has variable length.
 */
void finish_rule(int mach, int variable_trail_rule, int headcnt, int trailcnt, int pcont_act)
{
    char action_text[MAXLINE];
    auto rule_id = EOB_ACTION - 1;

    add_accept(mach, rule_id);

    /* We did this in new_rule(), but it often gets the wrong
	 * number because we do it before we start parsing the current rule.
	 */
    rules[rule_id].linenum = linenum;

    /* If this is a continued action, then the line-number has already
	 * been updated, giving us the wrong number.
	 */
    if (continued_action)
        --rules[rule_id].linenum;

    /* If the previous rule was continued action, then we inherit the
	 * previous newline flag, possibly overriding the current one.
	 */
    if (pcont_act && rules[rule_id - 1].has_nl)
        rules[rule_id].has_nl = true;

    snprintf(action_text, sizeof(action_text), "case %d:\n", (int)rule_id);
    add_action(action_text);
    if (rules[rule_id].has_nl)
    {
        snprintf(action_text, sizeof(action_text), "/* rule %d can match eol */\n", (int)rule_id);
        add_action(action_text);
    }

    if (variable_trail_rule)
    {
        rules[rule_id].type = RuleType::Variable;

        if (performance_report > 0)
            fprintf(stderr, _("Variable trailing context rule at line %d\n"), rules[rule_id].linenum);

        variable_trailing_context_rules = true;
    }
    else
    {
        rules[rule_id].type = RuleType::Normal;

        if (headcnt > 0 || trailcnt > 0)
        {
            /* Do trailing context magic to not match the trailing
			 * characters.
			 */
            auto scanner_cp = "YY_G(yy_c_buf_p) = yy_cp";
            auto scanner_bp = "yy_bp";

            add_action("*yy_cp = YY_G(yy_hold_char); /* undo effects of setting up yytext */\n");

            if (headcnt > 0)
            {
                if (rules[rule_id].has_nl)
                {
                    snprintf(action_text, sizeof(action_text), "YY_LINENO_REWIND_TO(%s + %d);\n", scanner_bp, headcnt);
                    add_action(action_text);
                }
                snprintf(action_text, sizeof(action_text), "%s = %s + %d;\n", scanner_cp, scanner_bp, headcnt);
                add_action(action_text);
            }
            else
            {
                if (rules[rule_id].has_nl)
                {
                    snprintf(action_text, sizeof(action_text), "YY_LINENO_REWIND_TO(yy_cp - %d);\n", trailcnt);
                    add_action(action_text);
                }

                snprintf(action_text, sizeof(action_text), "%s -= %d;\n", scanner_cp, trailcnt);
                add_action(action_text);
            }

            add_action("YY_DO_BEFORE_ACTION; /* set up yytext again */\n");
        }
    }

    /* Okay, in the action code at this point yytext and yyleng have
	 * their proper final values for this rule, so here's the point
	 * to do any user action.  But don't do it for continued actions,
	 * as that'll result in multiple YY_RULE_SETUP's.
	 */
    if (!continued_action)
        add_action("YY_RULE_SETUP\n");

    line_directive_out(false, true);
}

/* link_machines - connect two machines together
 *
 * synopsis
 *
 *   new = link_machines( first, last );
 *
 *     new    - a machine constructed by connecting first to last
 *     first  - the machine whose successor is to be last
 *     last   - the machine whose predecessor is to be first
 *
 * note: this routine concatenates the machine first with the machine
 *  last to produce a machine new which will pattern-match first first
 *  and then last, and will fail if either of the sub-patterns fails.
 *  FIRST is set to new by the operation.  last is unmolested.
 */
int link_machines(int first, int last)
{
    if (first == NIL)
        return last;

    else if (last == NIL)
        return first;

    else
    {
        mkxtion(nfas[first].finalst, last);
        nfas[first].finalst = nfas[last].finalst;
        nfas[first].lastst = std::max(nfas[first].lastst, nfas[last].lastst);
        nfas[first].firstst = std::min(nfas[first].firstst, nfas[last].firstst);

        return first;
    }
}

/* mark_beginning_as_normal - mark each "beginning" state in a machine
 *                            as being a "normal" (i.e., not trailing context-
 *                            associated) states
 *
 * The "beginning" states are the epsilon closure of the first state
 */
void mark_beginning_as_normal(int mach)
{
    switch (nfas[mach].state_type)
    {
    case StateType::Normal:
        /* Oh, we've already visited here. */
        return;
    case StateType::TrailingContext:
        nfas[mach].state_type = StateType::Normal;

        if (nfas[mach].transchar == SYM_EPSILON)
        {
            if (nfas[mach].trans1 != NO_TRANSITION)
                mark_beginning_as_normal(nfas[mach].trans1);

            if (nfas[mach].trans2 != NO_TRANSITION)
                mark_beginning_as_normal(nfas[mach].trans2);
        }
        break;
    default:
        flexerror(_("bad state type in mark_beginning_as_normal()"));
        break;
    }
}

/* mkbranch - make a machine that branches to two machines
 *
 * synopsis
 *
 *   branch = mkbranch( first, second );
 *
 *     branch - a machine which matches either first's pattern or second's
 *     first, second - machines whose patterns are to be or'ed (the | operator)
 *
 * Note that first and second are NEITHER destroyed by the operation.  Also,
 * the resulting machine CANNOT be used with any other "mk" operation except
 * more mkbranch's.  Compare with mkor()
 */
int mkbranch(int first, int second)
{
    int eps;

    if (first == NO_TRANSITION)
        return second;

    else if (second == NO_TRANSITION)
        return first;

    eps = mkstate(SYM_EPSILON);

    mkxtion(eps, first);
    mkxtion(eps, second);

    return eps;
}

/* mkclos - convert a machine into a closure
 *
 * synopsis
 *   new = mkclos( state );
 *
 * new - a new state which matches the closure of "state"
 */
int mkclos(int state)
{
    return mkopt(mkposcl(state));
}

/* mkopt - make a machine optional
 *
 * synopsis
 *
 *   new = mkopt( mach );
 *
 *     new  - a machine which optionally matches whatever mach matched
 *     mach - the machine to make optional
 *
 * notes:
 *     1. mach must be the last machine created
 *     2. mach is destroyed by the call
 */
int mkopt(int mach)
{
    int eps;

    if (!SUPER_FREE_EPSILON(nfas[mach].finalst))
    {
        eps = mkstate(SYM_EPSILON);
        mach = link_machines(mach, eps);
    }

    /* Can't skimp on the following if FREE_EPSILON(mach) is true because
	 * some state interior to "mach" might point back to the beginning
	 * for a closure.
	 */
    eps = mkstate(SYM_EPSILON);
    mach = link_machines(eps, mach);

    mkxtion(mach, nfas[mach].finalst);

    return mach;
}

/* mkor - make a machine that matches either one of two machines
 *
 * synopsis
 *
 *   new = mkor( first, second );
 *
 *     new - a machine which matches either first's pattern or second's
 *     first, second - machines whose patterns are to be or'ed (the | operator)
 *
 * note that first and second are both destroyed by the operation
 * the code is rather convoluted because an attempt is made to minimize
 * the number of epsilon states needed
 */
int mkor(int first, int second)
{
    int eps, orend;

    if (first == NIL)
        return second;

    else if (second == NIL)
        return first;

    else
    {
        /* See comment in mkopt() about why we can't use the first
		 * state of "first" or "second" if they satisfy "FREE_EPSILON".
		 */
        eps = mkstate(SYM_EPSILON);

        first = link_machines(eps, first);

        mkxtion(first, second);

        if (SUPER_FREE_EPSILON(nfas[first].finalst) && nfas[nfas[first].finalst].accptnum == NIL)
        {
            orend = nfas[first].finalst;
            mkxtion(nfas[second].finalst, orend);
        }
        else if (SUPER_FREE_EPSILON(nfas[second].finalst) && nfas[nfas[second].finalst].accptnum == NIL)
        {
            orend = nfas[second].finalst;
            mkxtion(nfas[first].finalst, orend);
        }
        else
        {
            eps = mkstate(SYM_EPSILON);

            first = link_machines(first, eps);
            orend = nfas[first].finalst;

            mkxtion(nfas[second].finalst, orend);
        }
    }

    nfas[first].finalst = orend;
    return first;
}

/* mkposcl - convert a machine into a positive closure
 *
 * synopsis
 *   new = mkposcl( state );
 *
 *    new - a machine matching the positive closure of "state"
 */
int mkposcl(int state)
{
    int eps;

    if (SUPER_FREE_EPSILON(nfas[state].finalst))
    {
        mkxtion(nfas[state].finalst, state);
        return state;
    }

    else
    {
        eps = mkstate(SYM_EPSILON);
        mkxtion(eps, state);
        return link_machines(state, eps);
    }
}

/* mkrep - make a replicated machine
 *
 * synopsis
 *   new = mkrep( mach, lb, ub );
 *
 *    new - a machine that matches whatever "mach" matched from "lb"
 *          number of times to "ub" number of times
 *
 * note
 *   if "ub" is INFINITE_REPEAT then "new" matches "lb" or more occurrences of "mach"
 */
int mkrep(int mach, int lb, int ub)
{
    int base_mach, tail, copy, i;

    base_mach = copysingl(mach, lb - 1);

    if (ub == INFINITE_REPEAT)
    {
        copy = dupmachine(mach);
        mach = link_machines(mach,
                             link_machines(base_mach,
                                           mkclos(copy)));
    }

    else
    {
        tail = mkstate(SYM_EPSILON);

        for (i = lb; i < ub; ++i)
        {
            copy = dupmachine(mach);
            tail = mkopt(link_machines(copy, tail));
        }

        mach =
            link_machines(mach,
                          link_machines(base_mach, tail));
    }

    return mach;
}

/* mkstate - create a state with a transition on a given symbol
 *
 * synopsis
 *
 *   state = mkstate( sym );
 *
 *     state - a new state matching sym
 *     sym   - the symbol the new state is to have an out-transition on
 *
 * note that this routine makes new states in ascending order through the
 * state array (and increments LASTNFA accordingly).  The routine DUPMACHINE
 * relies on machines being made in ascending order and that they are
 * CONTIGUOUS.  Change it and you will have to rewrite DUPMACHINE (kludge
 * that it admittedly is)
 */
int mkstate(int sym)
{
    Nfa nfa;

    nfa.firstst = nfas.size();
    nfa.finalst = nfas.size();
    nfa.lastst = nfas.size();
    nfa.transchar = sym;
    nfa.trans1 = NO_TRANSITION;
    nfa.trans2 = NO_TRANSITION;
    nfa.accptnum = NIL;
    nfa.assoc_rule = rules.size() - 1;
    nfa.state_type = current_state_type;

    nfas.push_back(nfa);

    /* Fix up equivalence classes base on this transition.  Note that any
	 * character which has its own transition gets its own equivalence
	 * class.  Thus only characters which are only in character classes
	 * have a chance at being in the same equivalence class.  E.g. "a|b"
	 * puts 'a' and 'b' into two different equivalence classes.  "[ab]"
	 * puts them in the same equivalence class (barring other differences
	 * elsewhere in the input).
	 */
    if (sym < 0)
    {
        /* We don't have to update the equivalence classes since
		 * that was already done when the ccl was created for the
		 * first time.
		 */
    }
    else if (sym == SYM_EPSILON)
        ++numeps;
    else
    {
        check_char(sym);

        if (useecs)
            /* Map NUL's to csize. */
            mkechar(sym ? sym : csize, nextecm, ecgroup);
    }

    return nfas.size() - 1;
}

/* mkxtion - make a transition from one state to another
 *
 * synopsis
 *
 *   mkxtion( statefrom, stateto );
 *
 *     statefrom - the state from which the transition is to be made
 *     stateto   - the state to which the transition is to be made
 */
void mkxtion(int statefrom, int stateto)
{
    if (nfas[statefrom].trans1 == NO_TRANSITION)
        nfas[statefrom].trans1 = stateto;
    else if ((nfas[statefrom].transchar != SYM_EPSILON) ||
             (nfas[statefrom].trans2 != NO_TRANSITION))
        flexfatal(_("found too many transitions in mkxtion()"));
    else
    {
        /* second out-transition for an epsilon state */
        ++eps2;
        nfas[statefrom].trans2 = stateto;
    }
}

/* new_rule - initialize for a new rule */
void new_rule()
{
    Rule r;
    r.linenum = linenum;
    rules.push_back(r);
}
