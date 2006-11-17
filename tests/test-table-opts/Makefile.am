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
# 'test-opt' builds non-serialized scanners with various table options.
# 'test-ver' verifies that the serialized tables match the in-code tables.
# 'test-ser' deserializes the tables at runtime.
# 'test-mul' checks that we can store multiple tables in a single file.
# ------------------------------------------------

FLEX = $(top_builddir)/flex

testname  := test-table-opts
allopts   := -Ca -Ce -Cf -CF -Cm -Cem -Cae -Caef -CaeF -Cam -Caem

# the test names themselves
opttests :=  $(foreach opt,$(allopts), test-opt-nr$(opt) test-opt-r$(opt))
sertests :=  $(foreach opt,$(allopts), test-ser-nr$(opt) test-ser-r$(opt))
vertests :=  $(foreach opt,$(allopts), test-ver-nr$(opt) test-ver-r$(opt))
alltests  := $(opttests) $(vertests) $(sertests) test-mul

# the executables to build
optexe := $(addsuffix $(EXEEXT),$(opttests))
verexe := $(addsuffix $(EXEEXT),$(vertests))
serexe := $(addsuffix $(EXEEXT),$(sertests))
allexe := $(optexe) $(verexe) $(serexe)

# the .c files
optsrc := $(addsuffix .c,$(opttests))
versrc := $(addsuffix .c,$(vertests))
sersrc := $(addsuffix .c,$(sertests))
allsrc := $(optsrc) $(versrc) $(sersrc)

# the .o files
optobj := $(addsuffix .o,$(opttests))
verobj := $(addsuffix .o,$(vertests))
serobj := $(addsuffix .o,$(sertests))
allobj := $(optobj) $(verobj) $(serobj)

# the .tables files
sertables  := $(addsuffix .tables,$(sertests))
alltables  := $(addsuffix .tables,$(alltests))

EXTRA_DIST = scanner.l test.input
CLEANFILES = scanner.c OUTPUT $(allobj) $(allsrc) $(alltables) \
             all-ser.tables $(allexe)
AM_CPPFLAGS = -I$(srcdir) -I$(top_srcdir) -I$(top_builddir)

test: test-opt test-ser test-ver test-mul

test-opt-r%.c: $(srcdir)/scanner.l
	$(FLEX) -L -P $(subst -,_,$(basename $(@F))) --reentrant $*  -o $@ $<

test-opt-nr%.c: $(srcdir)/scanner.l
	$(FLEX) -L -P $(subst -,_,$(basename $(@F))) $* -o $@ $<

test-ser-r%.c: $(srcdir)/scanner.l
	$(FLEX) -L -P $(subst -,_,$(basename $(@F))) -R --tables-file="test-ser-r$*.tables" $*  -o $@ $<

test-ser-nr%.c: $(srcdir)/scanner.l
	$(FLEX) -L -P $(subst -,_,$(basename $(@F))) --tables-file="test-ser-nr$*.tables"  $* -o $@ $<

test-ver-r%.c: $(srcdir)/scanner.l
	$(FLEX) -L -P $(subst -,_,$(basename $(@F))) -R --tables-file="test-ver-r$*.tables" --tables-verify $*  -o $@ $<

test-ver-nr%.c: $(srcdir)/scanner.l
	$(FLEX) -L -P $(subst -,_,$(basename $(@F))) --tables-file="test-ver-nr$*.tables" --tables-verify $* -o $@ $<

test-opt%$(EXEEXT): test-opt%.o
	$(CC) -o $@ $(LDFLAGS) $< $(LOADLIBES)

test-ser%$(EXEEXT): test-ser%.o
	$(CC) -o $@ $(LDFLAGS) $< $(LOADLIBES)

test-ver%$(EXEEXT): test-ver%.o
	$(CC) -o $@ $(LDFLAGS) $< $(LOADLIBES)

test-opt: $(optexe)
	for t in $(optexe) ; do \
		./$$t `basename $$t $(EXEEXT)`.tables < $(srcdir)/test.input \
          || { echo $t FAILED ; exit 1 ; } ; \
	done

test-ver: $(verexe)
	for t in $(verexe) ; do \
		./$$t `basename $$t $(EXEEXT)`.tables < $(srcdir)/test.input \
          || { echo $t FAILED ; exit 1 ; } ; \
	done

test-ser: $(serexe)
	for t in $(serexe) ; do \
		./$$t `basename $$t $(EXEEXT)`.tables < $(srcdir)/test.input \
          || { echo $t FAILED ; exit 1 ; } ; \
	done

test-mul: $(serexe)
	$(RM) all-ser.tables
	cat $(sertables) > all-ser.tables
	for t in $(serexe) ; do \
		./$$t all-ser.tables < $(srcdir)/test.input || { echo $$t FAILED; exit 1; } ;  \
	done

test-opt-nr-%.o: test-opt-nr-%.c ; $(CC) -c -o $@ $(AM_CPPFLAGS) $(CPPFLAGS) $(CFLAGS) $<
test-ser-nr-%.o: test-ser-nr-%.c ; $(CC) -c -o $@ $(AM_CPPFLAGS) $(CPPFLAGS) -DTEST_HAS_TABLES_EXTERNAL $(CFLAGS) $<
test-ver-nr-%.o: test-ver-nr-%.c ; $(CC) -c -o $@ $(AM_CPPFLAGS) $(CPPFLAGS) -DTEST_HAS_TABLES_EXTERNAL $(CFLAGS) $<

test-opt-r-%.o:  test-opt-r-%.c  ; $(CC) -c -o $@ $(AM_CPPFLAGS) $(CPPFLAGS) -DTEST_IS_REENTRANT $(CFLAGS) $<
test-ser-r-%.o:  test-ser-r-%.c  ; $(CC) -c -o $@ $(AM_CPPFLAGS) $(CPPFLAGS) -DTEST_HAS_TABLES_EXTERNAL -DTEST_IS_REENTRANT $(CFLAGS) $<
test-ver-r-%.o:  test-ver-r-%.c  ; $(CC) -c -o $@ $(AM_CPPFLAGS) $(CPPFLAGS) -DTEST_HAS_TABLES_EXTERNAL -DTEST_IS_REENTRANT $(CFLAGS) $<

.PHONY: test test-opt test-ser test-ver test-mul
.SECONDARY: $(allobj) $(allsrc)
