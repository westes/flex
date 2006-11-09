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

EXTRA_DIST = scanner-1.l scanner-2.l main.c
CLEANFILES = scanner-1.c scanner-1.h $(testname)$(EXEEXT) OUTPUT $(OBJS) scanner-2.c scanner-2.h
OBJS = scanner-1.o scanner-2.o main.o

AM_CPPFLAGS = -I$(srcdir) -I$(top_srcdir) -I$(top_builddir) -I$(builddir)
#LDFLAGS = $(top_srcdir)/libfl.a 
#YFLAGS = --defines --output=parser.c

testname = test-multiple-scanners-nr

scanner-1.c: $(srcdir)/scanner-1.l
	$(FLEX) $(LFLAGS) --header=scanner-1.h $<

scanner-2.c: $(srcdir)/scanner-2.l
	$(FLEX) $(LFLAGS) --header=scanner-2.h $<

$(testname)$(EXEEXT): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LOADLIBES)

test: $(testname)$(EXEEXT)
	./$(testname)$(EXEEXT)

.c.o:
	$(CC) -c -o $@ $(AM_CPPFLAGS) $(CPPFLAGS) $(CFLAGS) $<

main.o: scanner-1.h scanner-2.h
scanner-1.h: scanner-1.c
scanner-2.h: scanner-2.c
