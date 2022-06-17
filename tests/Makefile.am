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

AM_CFLAGS = $(WARNINGFLAGS)

TESTS = $(check_PROGRAMS) options.cn

# The script testwrapper.sh will run most tests as is. A couple tests
# in the suite end in .direct, .cn. and the like so that we can pass
# different arguments to the test runner. We list those extensions so
# automake knows how to distinguish between the various kinds of tests
# we have.

TEST_EXTENSIONS = .direct .cn .i3 .pthread .opt .ser .ver

LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper.sh
AM_LOG_FLAGS = -d $(srcdir) -r

DIRECT_LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper-direct.sh
AM_DIRECT_LOG_FLAGS = -b $(abs_builddir) -s $(srcdir)

CN_LOG_COMPILER = $(SHELL) $(srcdir)/options.cn
AM_CN_LOG_FLAGS = $(FLEX)

I3_LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper.sh
AM_I3_LOG_FLAGS = -i $(srcdir)/cxx_yywrap.txt -i $(srcdir)/cxx_yywrap.txt -i $(srcdir)/cxx_yywrap.txt

PTHREAD_LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper.sh
AM_PTHREAD_LOG_FLAGS = -i $(srcdir)/pthread_1.txt -i $(srcdir)/pthread_2.txt -i $(srcdir)/pthread_3.txt -i $(srcdir)/pthread_4.txt -i $(srcdir)/pthread_5.txt

OPT_LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper.sh
AM_OPT_LOG_FLAGS = -d $(srcdir) -i $(srcdir)/tableopts.txt -r

SER_LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper.sh
AM_SER_LOG_FLAGS = -d $(builddir) -i $(srcdir)/tableopts.txt -r

VER_LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper.sh
AM_VER_LOG_FLAGS = -d $(builddir) -i $(srcdir)/tableopts.txt -r

AM_YFLAGS = -d -p test
AM_CPPFLAGS = -I$(top_builddir)/src -I$(top_srcdir)/src

check_PROGRAMS = $(RULESET_TESTS) $(SPORADIC_TESTS) $(DIRECT_TESTS) $(I3_TESTS) $(PTHREAD_TESTS)

SPORADIC_TESTS = \
	alloc_extra_nr \
	alloc_extra_c99 \
	bison_nr \
	bison_yylloc \
	bison_yylval \
	c_cxx_nr \
	c_cxx_r \
	cxx_basic \
	cxx_multiple_scanners \
	cxx_restart \
	header_nr \
	header_r \
	mem_nr \
	mem_r \
	mem_c99 \
	multiple_scanners_nr \
	multiple_scanners_r \
	prefix_nr \
	prefix_r \
	prefix_c99 \
	quotes \
	string_nr \
	string_r \
	string_c99 \
	top \
	yyextra_nr \
	yyextra_c99


DIRECT_TESTS = \
	include_by_buffer.direct \
	include_by_push.direct \
	include_by_reentrant.direct \
	rescan_nr.direct \
	rescan_r.direct \
	state_buf.direct \
	state_buf_multiple.direct

I3_TESTS = \
	cxx_yywrap.i3 \
	yywrap_r.i3

PTHREAD_TESTS = \
	pthread.pthread

alloc_extra_nr_SOURCES = alloc_extra_nr.l
alloc_extra_c99_SOURCES = alloc_extra_c99.l
if HAVE_BISON
bison_nr_SOURCES = bison_nr_scanner.l bison_nr_parser.y bison_nr_main.c
nodist_bison_nr_SOURCES = bison_nr_parser.h bison_nr_scanner.h
bison_yylloc_SOURCES = bison_yylloc_scanner.l bison_yylloc_parser.y bison_yylloc_main.c
nodist_bison_yylloc_SOURCES = bison_yylloc_parser.h bison_yylloc_scanner.h
bison_yylval_SOURCES = bison_yylval_scanner.l bison_yylval_parser.y bison_yylval_main.c
nodist_bison_yylval_SOURCES = bison_yylval_parser.h bison_yylval_scanner.h
else
bison_nr_SOURCES = no_bison_stub.c
bison_yylloc_SOURCES = no_bison_stub.c
bison_yylval_SOURCES = no_bison_stub.c
endif
c_cxx_nr_SOURCES = c_cxx_nr.lll
c_cxx_r_SOURCES = c_cxx_r.lll
cxx_basic_SOURCES = cxx_basic.ll
cxx_restart_SOURCES = cxx_restart.ll
cxx_multiple_scanners_SOURCES = cxx_multiple_scanners_main.cc cxx_multiple_scanners_1.ll cxx_multiple_scanners_2.ll
cxx_yywrap_i3_SOURCES = cxx_yywrap.ll
header_nr_SOURCES = header_nr_scanner.l header_nr_main.c
nodist_header_nr_SOURCES = header_nr_scanner.h
header_r_SOURCES = header_r_scanner.l header_r_main.c
nodist_header_r_SOURCES = header_r_scanner.h
include_by_buffer_direct_SOURCES = include_by_buffer.direct.l
include_by_push_direct_SOURCES = include_by_push.direct.l
include_by_reentrant_direct_SOURCES = include_by_reentrant.direct.l
mem_nr_SOURCES = mem_nr.l
mem_r_SOURCES = mem_r.l
mem_c99_SOURCES = mem_c99.l
multiple_scanners_nr_SOURCES = multiple_scanners_nr_main.c multiple_scanners_nr_1.l multiple_scanners_nr_2.l
nodist_multiple_scanners_nr_SOURCES = multiple_scanners_nr_1.h multiple_scanners_nr_2.h
multiple_scanners_r_SOURCES = multiple_scanners_r_main.c multiple_scanners_r_1.l multiple_scanners_r_2.l
nodist_multiple_scanners_r_SOURCES = multiple_scanners_nr_1.h multiple_scanners_nr_2.h
prefix_nr_SOURCES = prefix_nr.l
prefix_r_SOURCES = prefix_r.l
prefix_c99_SOURCES = prefix_c99.l
pthread_pthread_SOURCES = pthread.l
quotes_SOURCES = quotes.l
rescan_nr_direct_SOURCES = rescan_nr.direct.l
rescan_r_direct_SOURCES = rescan_r.direct.l
string_nr_SOURCES = string_nr.l
string_r_SOURCES = string_r.l
string_c99_SOURCES = string_c99.l
top_SOURCES = top.l top_main.c
nodist_top_SOURCES = top.h
yyextra_nr_SOURCES = yyextra_nr.l
yyextra_c99_SOURCES = yyextra_c99.l
yywrap_r_i3_SOURCES = yywrap_r.i3.l
state_buf_direct_SOURCES = state_buf.direct.lll
state_buf_multiple_direct_SOURCES = state_buf_multiple.direct.lll

# Normally, automake would distribute files built by flex. Since the
# point of the test suite is to test the files that flex builds, and
# since anyone who has the flex distribution can build a flex binary
# without having a flex binary, we want to not distribute the files
# built by flex in the test suite. The dist-hook target accomplishes
# this goal. It's also handy to have the list of files to clean since
# automake doesn't know about the whacky ways in which we're abusing
# it.

CLEANFILES = \
	alloc_extra_nr.c \
	alloc_extra_c99.c \
	bison_nr_parser.c \
	bison_nr_parser.h \
	bison_nr_scanner.c \
	bison_nr_scanner.h \
	bison_yylloc_parser.c \
	bison_yylloc_parser.h \
	bison_yylloc_scanner.c \
	bison_yylloc_scanner.h \
	bison_yylval_parser.c \
	bison_yylval_parser.h \
	bison_yylval_scanner.c \
	bison_yylval_scanner.h \
	c_cxx_nr.cc \
	c_cxx_r.cc \
	cxx_basic.cc \
	cxx_multiple_scanners_1.cc \
	cxx_multiple_scanners_2.cc \
	cxx_restart.cc \
	cxx_yywrap.cc \
	header_nr_scanner.c \
	header_nr_scanner.h \
	header_r_scanner.c \
	header_r_scanner.h \
	include_by_buffer.direct.c \
	include_by_push.direct.c \
	include_by_reentrant.direct.c \
	mem_nr.c \
	mem_r.c \
	mem_c99.c \
	multiple_scanners_nr_1.c \
	multiple_scanners_nr_1.h \
	multiple_scanners_nr_2.c \
	multiple_scanners_nr_2.h \
	multiple_scanners_r_1.c \
	multiple_scanners_r_1.h \
	multiple_scanners_r_2.c \
	multiple_scanners_r_2.h \
	prefix_nr.c \
	prefix_r.c \
	prefix_c99.c \
	pthread.c \
	quotes.c \
	quotes_c99.c \
	rescan_nr.direct.c \
	rescan_r.direct.c \
	string_nr.c \
	string_r.c \
	string_c99.c \
	string_c99.c \
	top.c \
	top.h  \
	yyextra_nr.c \
	yyextra_c99.c \
	yywrap_r.i3.c \
	state_buf.direct.cc \
	state_buf_multiple.direct.cc \
	$(RULESET_REMOVABLES)

dist-hook:
	chmod u+w $(distdir) && \
	for file in $(CLEANFILES) ; do \
	chmod u+w $(distdir)/$$file 2>/dev/null; \
	rm -f $(distdir)/$$file; \
	done

EXTRA_DIST = \
	README \
	testmaker.m4 \
	alloc_extra.txt \
	bison_nr.txt \
	bison_yylloc.txt \
	bison_yylval.txt \
	c_cxx_nr.txt \
	c_cxx_r.txt \
	cxx_basic.txt \
	cxx_multiple_scanners.txt \
	cxx_restart.txt \
	cxx_yywrap.txt \
	header_nr.txt \
	header_r.txt \
	include_by_buffer.direct.txt \
	include_by_buffer.direct_2.txt \
	include_by_buffer.direct_3.txt \
	include_by_push.direct.txt \
	include_by_push.direct_2.txt \
	include_by_push.direct_3.txt \
	include_by_reentrant.direct.txt \
	include_by_reentrant.direct_2.txt \
	include_by_reentrant.direct_3.txt \
	mem.txt \
	prefix.txt \
	pthread_1.txt \
	pthread_2.txt \
	pthread_3.txt \
	pthread_4.txt \
	pthread_5.txt \
	rescan_nr.direct.txt \
	rescan_r.direct.txt \
	quotes.txt \
	top.txt \
	yyextra.txt \
	array.txt \
	basic.txt \
	bol.txt \
	ccl.txt \
	debug.txt \
	extended.txt \
	fixedtrailing.txt \
	flexname.txt \
	lexcompat.txt \
	lineno.txt \
	posix.txt \
	posixlycorrect.txt \
	preposix.txt \
	quoteincomment.txt \
	reject.txt \
	tableopts.txt \
	vartrailing.txt \
	yyless.txt \
	yymore.txt \
	yymorearray.txt \
	yymorearraybol.txt \
	yyunput.txt \
	state_buf.direct.txt \
	state_buf_multiple.direct.txt \
	$(RULESETS)

dist_noinst_SCRIPTS = \
	ruleset.sh \
	testmaker.sh \
	test-yydecl-gen.sh

dist_check_SCRIPTS = \
	options.cn \
	testwrapper-direct.sh \
	testwrapper.sh

pthread_pthread_LDADD = @LIBPTHREAD@

# specify how to process .l files in order to test the flex built by make all

TESTOPTS = -L

FLEX = $(top_builddir)/src/flex

.l.c: $(FLEX)
	$(AM_V_LEX)$(FLEX) $(TESTOPTS) -o $@ $<

.ll.cc: $(FLEX)
	$(AM_V_LEX)$(FLEX) $(TESTOPTS) -+ -o $@ $<

bison_nr_main.$(OBJEXT): bison_nr_parser.h bison_nr_scanner.h
bison_nr_scanner.$(OBJEXT): bison_nr_parser.h

bison_nr_scanner.h: bison_nr_scanner.c
	@if test ! -f $@; then rm -f $<; else :; fi
	@if test ! -f $@; then $(MAKE) $(AM_MAKEFLAGS) $<; else :; fi

bison_yylloc_main.$(OBJEXT): bison_yylloc_parser.h bison_yylloc_scanner.h
bison_yylloc_scanner.$(OBJEXT): bison_yylloc_parser.h

bison_yylloc_scanner.h: bison_yylloc_scanner.c
	@if test ! -f $@; then rm -f $<; else :; fi
	@if test ! -f $@; then $(MAKE) $(AM_MAKEFLAGS) $<; else :; fi

bison_yylval_main.$(OBJEXT): bison_yylval_parser.h bison_yylval_scanner.h
bison_yylval_scanner.$(OBJEXT): bison_yylval_parser.h

bison_yylval_scanner.h: bison_yylval_scanner.c
	@if test ! -f $@; then rm -f $<; else :; fi
	@if test ! -f $@; then $(MAKE) $(AM_MAKEFLAGS) $<; else :; fi

# automake does not support compiling flex scanners output in C as C++
# so we explicitly sayhow, using the .lll suffix for the lex input file

.lll.cc: $(FLEX)
	$(AM_V_LEX)$(FLEX) $(TESTOPTS) -o $@ $<

header_nr_main.$(OBJEXT): header_nr_scanner.h

header_nr_scanner.h: header_nr_scanner.c
	@if test ! -f $@; then rm -f $<; else :; fi
	@if test ! -f $@; then $(MAKE) $(AM_MAKEFLAGS) $<; else :; fi

header_r_main.$(OBJEXT): header_r_scanner.h

header_r_scanner.h: header_r_scanner.c
	@if test ! -f $@; then rm -f $<; else :; fi
	@if test ! -f $@; then $(MAKE) $(AM_MAKEFLAGS) $<; else :; fi

multiple_scanners_nr_main.$(OBJEXT): multiple_scanners_nr_1.h multiple_scanners_nr_2.h

multiple_scanners_nr_1.h: multiple_scanners_nr_1.c
	@if test ! -f $@; then rm -f $<; else :; fi
	@if test ! -f $@; then $(MAKE) $(AM_MAKEFLAGS) $<; else :; fi

multiple_scanners_nr_2.h: multiple_scanners_nr_2.c
	@if test ! -f $@; then rm -f $<; else :; fi
	@if test ! -f $@; then $(MAKE) $(AM_MAKEFLAGS) $<; else :; fi

multiple_scanners_r_main.$(OBJEXT): multiple_scanners_r_1.h multiple_scanners_r_2.h

multiple_scanners_r_1.h: multiple_scanners_r_1.c
	@if test ! -f $@; then rm -f $<; else :; fi
	@if test ! -f $@; then $(MAKE) $(AM_MAKEFLAGS) $<; else :; fi

multiple_scanners_r_2.h: multiple_scanners_r_2.c
	@if test ! -f $@; then rm -f $<; else :; fi
	@if test ! -f $@; then $(MAKE) $(AM_MAKEFLAGS) $<; else :; fi

top_main.$(OBJEXT): top.h
top.h: top.c

# Build rules for non-C back ends

.l.go:
	$(AM_V_LEX)$(FLEX) $(TESTOPTS) -o $@ $<

# This is a temporary fake rule for use while the Go back end still
# actually generates C.
.go:
	$(CC) $< -o $*_go

# Most test productions can be autogenerated from ruleset files, but
# automake has no way to specify such things with a loop in a variable
# (even though make can do such things) and the resultant list is both
# long and unenlightening. And it can be / is generated by a shell
# script, ruleset.sh.

RULESETS = \
	$(srcdir)/array.rules \
	$(srcdir)/basic.rules \
	$(srcdir)/bol.rules \
	$(srcdir)/ccl.rules \
	$(srcdir)/debug.rules \
	$(srcdir)/extended.rules \
	$(srcdir)/fixedtrailing.rules \
	$(srcdir)/flexname.rules \
	$(srcdir)/lexcompat.rules \
	$(srcdir)/lineno.rules \
	$(srcdir)/posix.rules \
	$(srcdir)/posixlycorrect.rules \
	$(srcdir)/preposix.rules \
	$(srcdir)/quoteincomment.rules \
	$(srcdir)/reject.rules \
	$(srcdir)/tableopts.rules \
	$(srcdir)/vartrailing.rules \
	$(srcdir)/yyless.rules \
	$(srcdir)/yymore.rules \
	$(srcdir)/yymorearray.rules \
	$(srcdir)/yymorearraybol.rules \
	$(srcdir)/yyunput.rules

$(srcdir)/ruleset.am: $(srcdir)/ruleset.sh $(RULESETS)
	$(SHELL) $(srcdir)/ruleset.sh nr r c99 go > $(srcdir)/ruleset.am

include $(srcdir)/ruleset.am


