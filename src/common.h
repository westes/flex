/*  common.h - common data structures, classes and routines
 *
 *  Copyright (c) 1990 The Regents of the University of California.
 *  All rights reserved.
 *
 *  This code is derived from software contributed to Berkeley by
 *  Vern Paxson.
 *
 *  The United States Government has rights in this work pursuant
 *  to contract no. DE-AC03-76SF00098 between the United States
 *  Department of Energy and the University of California.
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

#include <memory>
#include <set>
#include <stdint.h>
#include <string>
#include <queue>
#include <unordered_set>
#include <vector>

using Character = uint32_t;

using String = std::string;
using Strings = std::vector<String>;

using InputFiles = std::queue<String>;

template <class T>
class CapacityVector : public std::vector<T>
{
public:
    using base = std::vector<T>;
    using reference = typename base::reference;
    using const_reference = typename base::const_reference;
    using size_type = typename base::size_type;

public:
    using base::base;
    using base::capacity;
    using base::data;

    reference operator[](size_type i)
    {
        if (capacity() <= i)
            abort();
        return *(data() + i);
    }

    const_reference operator[](size_type i) const
    {
        if (capacity() <= i)
            abort();
        return *(data() + i);
    }
};

//
// rules
//

enum class RuleType
{
    Normal = 0,
    Variable = 1,
};

struct Rule
{
    RuleType type = RuleType::Normal;
    int linenum = 0;
    bool useful = false;
    bool has_nl = false;
};

using Rules = std::vector<Rule>;

// start conditions
struct StartCondition
{
    String name;
    int set;        // set of rules active in start condition
    int bol;        // set of rules active only at the beginning of line in a s.c.
    bool xclu;      // true if start condition is exclusive
    bool eof;       // true if start condition has EOF rule
};

using StartConditions = std::vector<StartCondition>;

//
// character classes
//

using CharacterTable = std::set<Character>;

struct CharacterClass
{
    using Table = CharacterTable;

    Table table;            // holds the characters in each ccl
    bool ng = false;        // true for a given ccl if the ccl is negated
    bool has_nl = false;    // true if current ccl could match a newline
};

using CharacterClasses = std::vector<CharacterClass>;

//
// NFA
//

/* Different types of states; values are useful as masks, as well, for
* routines like check_trailing_context().
*/
enum class StateType
{
    Normal = 1,
    TrailingContext = 2,
};

struct Nfa
{
    int firstst;    // physically the first state of a fragment
    int lastst;     // last physical state of fragment
    int finalst;    // last logical state of fragment
    int transchar;  // transition character
    int trans1;     // transition state
    int trans2;     // 2nd transition state for epsilons
    int accptnum;   // accepting number
    int assoc_rule; // rule associated with this NFA state (or 0 if none)
                    /* a STATE_xxx type identifying whether the state is part
                    * 	of a normal rule, the leading state in a trailing context
                    * 	rule (i.e., the state which marks the transition from
                    * 	recognizing the text-to-be-matched to the beginning of
                    * 	the trailing context), or a subsequent state in a trailing
                    * 	context rule
                    */
    StateType state_type;
};

using Nfas = std::vector<Nfa>;

//
// DFA
//

struct Dfa
{
    using AccSet = std::vector<int>;
    using AccSetPtr = std::shared_ptr<AccSet>;

    int base;                   // offset into "nxt" for given state
    int def;                    // where to go if "chk" disallows "nxt" entry
    int hash;                   // dfa state hash value
    int nultrans;               // NUL transition for each state
    std::vector<int> dss;       // nfa state set for each dfa
    AccSetPtr acc_set;          // accepting set for each dfa state (if using REJECT)
    int acc_state;              // or accepting number, if not
};

using Dfas = CapacityVector<Dfa>;
