# This file is part of flex.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:

# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.

# Neither the name of the University nor the names of its contributors
# may be used to endorse or promote products derived from this software
# without specific prior written permission.

# THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE.

# ------------------------------------------------
# This test is really a set of tests, one for
# each compression flag. -Ca, -Cem, etc..
# The 'opt' tests execute non-serialized scanners with various table options.
# The 'ver' verify that the serialized tables match the in-code tables.
# The 'ser' deserialize the tables and scan using them.
# ------------------------------------------------

BISON = @BISON@
FLEX = $(top_builddir)/flex

testname  := test-table-opts
allopts   := -Ca -Ce -Cf -CF -Cm -Cem -Cae -Caef -CaeF -Cam -Caem
variations := opt-nr opt-r ser-nr ser-r ver-nr ver-r
alltests  := $(foreach opt,$(allopts), $(foreach vari,$(variations),test-$(vari)$(opt)))
alltestexe := $(addsuffix $(EXEEXT),$(alltests))
alltestsrc := $(addsuffix .c,$(alltests))
alltestobj := $(addsuffix .o,$(alltests))
alltables  := $(addsuffix .tables,$(alltests))

EXTRA_DIST = scanner.l test.input
CLEANFILES = scanner.c OUTPUT $(alltestobj) $(alltestsrc) $(alltestexe)\
             $(alltables)
INCLUDES = -I $(srcdir) -I $(top_srcdir) -I $(top_builddir) -I .


test-table-opts: $(alltests) comparison_test

test-opt-r%.c: $(srcdir)/scanner.l
	$(FLEX) -L --reentrant $*  -o $@ $<

test-opt-nr%.c: $(srcdir)/scanner.l
	$(FLEX) -L $* -o $@ $<

test-ser-r%.c: $(srcdir)/scanner.l
	$(FLEX) -L --reentrant --tables-file="test-ser-r$*.tables" $*  -o $@ $<

test-ser-nr%.c: $(srcdir)/scanner.l
	$(FLEX) -L --tables-file="test-ser-nr$*.tables"  $* -o $@ $<

test-ver-r%.c: $(srcdir)/scanner.l
	$(FLEX) -L --reentrant --tables-file="test-ver-r$*.tables" --tables-verify $*  -o $@ $<

test-ver-nr%.c: $(srcdir)/scanner.l
	$(FLEX) -L --tables-file="test-ver-nr$*.tables" --tables-verify $* -o $@ $<

test-opt%$(EXEEXT): test-opt%.o
	$(CC) -o $@ $(LDFLAGS) $< $(LOADLIBES)

test-ser%$(EXEEXT): test-ser%.o
	$(CC) -o $@ $(LDFLAGS) $< $(LOADLIBES)

test-ver%$(EXEEXT): test-ver%.o
	$(CC) -o $@ $(LDFLAGS) $< $(LOADLIBES)

test: $(alltestexe)
	for t in $(alltestexe) ; do \
		./$$t `basename $$t $(EXEEXT)`.tables < $(srcdir)/test.input || exit 1 ; \
	done

.c.o:
	$(CC) -c -o $@ $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $<

.PHONY: test-table-opts
.SECONDARY: $(alltestobj) $(alltestsrc)
