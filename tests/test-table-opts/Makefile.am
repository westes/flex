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
# ------------------------------------------------

BISON = @BISON@
FLEX = $(top_builddir)/flex

testname  := test-table-opts
allopts   := -Ca -Ce -Cf -CF -Cm -Cem -Cae -Caf -CaF -Cam -Caem
alltests  := $(foreach opt,$(allopts),test-opt-r$(opt) test-opt-nr$(opt))
alltestexe := $(addsuffix $(EXEEXT),$(alltests))
alltestsrc := $(addsuffix .c,$(alltests))
alltestobj := $(addsuffix .o,$(alltests))

EXTRA_DIST = scanner.l test.input
CLEANFILES = scanner.c OUTPUT $(alltestobj) $(alltestsrc) $(alltestexe)
INCLUDES = -I $(srcdir) -I $(top_srcdir) -I $(top_builddir) -I .


test-table-opts: $(alltests)

test-opt-r%.c: $(srcdir)/scanner.l
	$(FLEX) --reentrant $*  -o $@ $<

test-opt-nr%.c: $(srcdir)/scanner.l
	$(FLEX) $* -o $@ $<

test-opt%$(EXEEXT): test-opt%.o
	$(CC) -o $@ $< $(LDFLAGS)

test: $(alltestexe)
	for t in $(alltestexe) ; do \
		./$$t < $(srcdir)/test.input || exit 1 ; \
	done

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $(INCLUDES) $<

.PHONY: test-table-opts
.SECONDARY: $(alltestobj) $(alltestsrc)
