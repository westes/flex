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

FLEX = $(top_builddir)/flex

builddir = @builddir@

EXTRA_DIST = scanner.l test.input
CLEANFILES = l.out c.out  scanner.c scanner.h $(testname)$(EXEEXT) OUTPUT $(OBJS)
OBJS = scanner.o

AM_CPPFLAGS = -I$(srcdir) -I$(builddir) -I$(top_srcdir) -I$(top_builddir)

testname = test-quotes

scanner.c: $(srcdir)/scanner.l
	$(FLEX) $(LFLAGS) $<

$(testname)$(EXEEXT): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LOADLIBES)

test: $(testname)$(EXEEXT)
	grep TEST_XXX < $(srcdir)/scanner.l | sed 's/^ *//' > l.out
	grep TEST_XXX < scanner.c | sed 's/^ *//' > c.out
	diff -q l.out c.out
	./$(testname)$(EXEEXT) < $(srcdir)/test.input

.c.o:
	$(CC) -c -o $@ $(AM_CPPFLAGS) $(CPPFLAGS) $(CFLAGS) $<
