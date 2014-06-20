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


FLEX = $(top_builddir)/src/flex

cases := ISO-8859-1 EBCDIC-500 CP850

variants := r nr
test_variants := $(foreach variant,$(variants), test-charset-$(variant)-test)

LFILES := $(foreach VARIANT, $(VARIANTS), scanner-$(VARIANT).l)
CFILES := $(foreach VARIANT, $(VARIANTS), scanner-$(VARIANT).c)
EXEFILES := $(foreach VARIANT, $(VARIANTS), test-charset-$(VARIANT)$(EXEEXT))

test_case_files := $(foreach case,$(cases), test-$(case).input test-$(case).output)

EXTRA_DIST = scanner.l.in $(test_case_files)
CLEANFILES = $(LFILES) $(CFILES) $(EXEFILES)


AM_CPPFLAGS = -I$(srcdir) -I$(top_srcdir)/src -I$(top_builddir)

scanner-nr.l: $(srcdir)/scanner.l.in
	m4 -P $< > $@

scanner-r.l: $(srcdir)/scanner.l.in
	m4 -P -DVARIANT_REENTRANT=1 $< > $@

scanner-%.c: scanner-%.l
	$(FLEX) -o $@ $<

test-charset-%$(EXEEXT): scanner-%.c
	$(CC) $(AM_CPPFLAGS) $(CPPFLAGS) $(CFLAGS) -o $@ $(LDFLAGS) $< $(LOADLIBES)

test-charset-%-test: test-charset-%$(EXEEXT)
	for c in $(cases) ; do \
		(./$(<) $$c < $(srcdir)/test-$$c.input | diff -au - $(srcdir)/test-$$c.output) || (echo "Test $$exe failed in $$c case"; exit 1 ); \
	done

test: $(test_variants)