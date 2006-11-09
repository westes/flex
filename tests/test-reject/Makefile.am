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

EXTRA_DIST = scanner.l test.input
CLEANFILES = scanner.c $(testname)$(EXEEXT)-* OUTPUT $(OBJS)
OBJS = test-reject-nr.o test-reject-r.o test-reject-ver.o \
       test-reject-ser.o

AM_CPPFLAGS = -I$(srcdir) -I$(top_srcdir) -I$(top_builddir)

tests = test-reject-nr$(EXEEXT) test-reject-r$(EXEEXT) \
        test-reject-ser$(EXEEXT) test-reject-ver$(EXEEXT)
testname = test-reject

test-reject-nr.c: $(srcdir)/scanner.l
	$(FLEX) -o $@ $(LFLAGS) $<

test-reject-ver.c: $(srcdir)/scanner.l
	$(FLEX) -o $@  --tables-verify --tables-file=`basename $@ .c`.tables $(LFLAGS) $<

test-reject-ser.c: $(srcdir)/scanner.l
	$(FLEX) -o $@ --tables-file=`basename $@ .c`.tables $(LFLAGS) $<

test-reject-r.c: $(srcdir)/scanner.l
	$(FLEX) --reentrant -o $@ $(LFLAGS) $<

test-reject-nr$(EXEEXT): test-reject-nr.o
	$(CC) -o $@ $(LDFLAGS) $< $(LOADLIBES)

test-reject-ver$(EXEEXT): test-reject-ver.o
	$(CC) -o $@ $(LDFLAGS) $< $(LOADLIBES)

test-reject-ser$(EXEEXT): test-reject-ser.o
	$(CC) -o $@ $(LDFLAGS) $< $(LOADLIBES)

test-reject-r$(EXEEXT): test-reject-r.o
	$(CC) -o $@ $(LDFLAGS) $< $(LOADLIBES)

$(testname)$(EXEEXT): $(OBJS)

test: $(tests)
	./$(testname)-nr$(EXEEXT) < $(srcdir)/test.input
	./$(testname)-r$(EXEEXT) < $(srcdir)/test.input
	./$(testname)-ver$(EXEEXT) $(testname)-ver.tables < $(srcdir)/test.input
	./$(testname)-ser$(EXEEXT) $(testname)-ser.tables < $(srcdir)/test.input

test-reject-nr.o: test-reject-nr.c
	$(CC) -c -o $@ $(AM_CPPFLAGS) $(CPPFLAGS) $(CFLAGS) $<

test-reject-ver.o: test-reject-ver.c
	$(CC) -c -o $@ $(AM_CPPFLAGS) $(CPPFLAGS) -DTEST_HAS_TABLES_EXTERNAL $(CFLAGS) $<

test-reject-ser.o: test-reject-ser.c
	$(CC) -c -o $@ $(AM_CPPFLAGS) $(CPPFLAGS) -DTEST_HAS_TABLES_EXTERNAL $(CFLAGS) $<

test-reject-r.o: test-reject-r.c
	$(CC) -c -o $@ $(AM_CPPFLAGS) $(CPPFLAGS) -DTEST_IS_REENTRANT $(CFLAGS) $<


