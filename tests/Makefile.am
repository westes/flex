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

TESTS = $(check_PROGRAMS) options.cn

# The script testwrapper.sh will run most tests as is. A couple tests
# in the suite end in .reject, .table and the like so that we can pass
# different arguments to the test runner. We list those extensions so
# automake knows how to distinguish between the various kinds of tests
# we have.

TEST_EXTENSIONS = .reject .table .direct .cn .i3

LOG_COMPILER = $(srcdir)/testwrapper.sh
AM_LOG_FLAGS = -d $(srcdir) -r

REJECT_LOG_COMPILER = $(srcdir)/testwrapper.sh
AM_REJECT_LOG_FLAGS = -d $(srcdir) -i reject.txt -r

TABLE_LOG_COMPILER = $(srcdir)/testwrapper.sh
AM_TABLE_LOG_FLAGS = -d $(srcdir) -i reject.txt -t

DIRECT_LOG_COMPILER=$(srcdir)/testwrapper.sh
AM_DIRECT_LOG_FLAGS = -d $(srcdir)

CN_LOG_COMPILER=$(srcdir)/options.cn
AM_CN_LOG_FLAGS=$(FLEX)

I3_LOG_COMPILER=$(srcdir)/testwrapper.sh
AM_I3_LOG_FLAGS=-i $(srcdir)/cxx_yywrap.txt -i $(srcdir)/cxx_yywrap.txt -i $(srcdir)/cxx_yywrap.txt

AM_YFLAGS = -d -p test

check_PROGRAMS = $(simple_tests) $(reject_tests) $(TABLE_TESTS) $(DIRECT_TESTS) $(I3_TESTS)

simple_tests = \
	alloc_extra \
	array_nr \
	array_r \
	basic_nr \
	basic_r \
	bison_nr \
	bison_yylloc \
	bison_yylval \
	c_cxx_nr \
	c_cxx_r \
	ccl \
	cxx_basic \
	cxx_multiple_scanners \
	debug_nr \
	debug_r \
	extended \
	header_nr \
	header_r \
	mem_nr \
	mem_r \
	multiple_scanners_nr \
	multiple_scanners_r \
	noansi_nr \
	noansi_r \
	posix \
	posixly_correct \
	prefix_nr \
	prefix_r \
	quotes \
	string_nr \
string_r \
	top \
	yyextra

reject_tests = \
	reject_nr.reject \
	reject_r.reject

TABLE_TESTS = \
	reject_ver.table \
	reject_ser.table

DIRECT_TESTS = \
	include_by_buffer.direct \
	include_by_push.direct \
	include_by_reentrant.direct \
	rescan_nr.direct \
	rescan_r.direct

 I3_TESTS = \
	cxx_yywrap.i3

alloc_extra_SOURCES = alloc_extra.l
array_nr_SOURCES = array_nr.l
array_r_SOURCES = array_r.l
basic_nr_SOURCES = basic_nr.l
basic_r_SOURCES = basic_r.l
bison_nr_SOURCES = bison_nr_scanner.l bison_nr_parser.y bison_nr_main.c
bison_yylloc_SOURCES = bison_yylloc_scanner.l bison_yylloc_parser.y bison_yylloc_main.c
bison_yylval_SOURCES = bison_yylval_scanner.l bison_yylval_parser.y bison_yylval_main.c
c_cxx_nr_SOURCES = c_cxx_nr.lll
c_cxx_r_SOURCES = c_cxx_r.lll
ccl_SOURCES = ccl.l
cxx_basic_SOURCES = cxx_basic.ll
cxx_multiple_scanners_SOURCES = cxx_multiple_scanners_main.cc cxx_multiple_scanners_1.ll cxx_multiple_scanners_2.ll
cxx_yywrap_i3_SOURCES = cxx_yywrap.ll
debug_nr_SOURCES = debug_nr.l
debug_r_SOURCES = debug_r.l
extended_SOURCES = extended.l
header_nr_SOURCES = header_nr_scanner.l header_nr_main.c
header_r_SOURCES = header_r_scanner.l header_r_main.c
include_by_buffer_direct_SOURCES = include_by_buffer.direct.l
include_by_push_direct_SOURCES = include_by_push.direct.l
include_by_reentrant_direct_SOURCES = include_by_reentrant.direct.l
mem_nr_SOURCES = mem_nr.l
mem_r_SOURCES = mem_r.l
multiple_scanners_nr_SOURCES = multiple_scanners_nr_main.c multiple_scanners_nr_1.l multiple_scanners_nr_2.l
multiple_scanners_r_SOURCES = multiple_scanners_r_main.c multiple_scanners_r_1.l multiple_scanners_r_2.l
noansi_nr_SOURCES = noansi_nr.l
noansi_r_SOURCES = noansi_r.l
posix_SOURCES = posix.l
posixly_correct_SOURCES = posixly_correct.l
prefix_nr_SOURCES = prefix_nr.l
prefix_r_SOURCES = prefix_r.l
quotes_SOURCES = quotes.l
reject_nr_reject_SOURCES = reject.l4
reject_r_reject_SOURCES = reject.l4
reject_ver_table_SOURCES = reject.l4
reject_ser_table_SOURCES = reject.l4
rescan_nr_direct_SOURCES = rescan_nr.direct.l
rescan_r_direct_SOURCES = rescan_r.direct.l
string_nr_SOURCES = string_nr.l
string_r_SOURCES = string_r.l
top_SOURCES = top.l top_main.c
yyextra_SOURCES = yyextra.l

BUILT_SOURCES = \
	bison_nr_scanner.h \
	bison_nr_parser.h \
	bison_yylloc_parser.h \
	bison_yylloc_scanner.h \
	bison_yylval_parser.h \
	bison_yylval_scanner.h \
	c_cxx_nr.cc \
	c_cxx_r.cc \
	header_nr_scanner.h \
	header_r_scanner.h \
	multiple_scanners_nr_1.h \
	multiple_scanners_nr_2.h \
	multiple_scanners_r_1.h \
	multiple_scanners_r_2.h \
	reject_nr.reject.c \
	reject_r.reject.c \
	reject_ver.table.c \
	reject_ser.table.c \
	top.h

EXTRA_DIST = \
	README \
	testwrapper.sh \
	alloc_extra.txt \
	array_nr.txt \
	array_r.txt \
basic_nr.txt \
	basic_r.txt \
	bison_nr_scanner.h \
	bison_nr.txt \
	bison_yylloc_scanner.h \
	bison_yylloc.txt \
	bison_yylval_scanner.h \
	bison_yylval.txt \
	c_cxx_nr.cc \
	c_cxx_nr.txt \
	c_cxx_r.cc \
	c_cxx_r.txt \
	ccl.txt \
	cxx_basic.txt \
	cxx_multiple_scanners.txt \
	cxx_yywrap.txt \
	debug_nr.txt \
	debug_r.txt \
	extended.txt \
	header_nr_scanner.h \
	header_nr.txt \
	header_r_scanner.h \
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
	mem_nr.txt \
	mem_r.txt \
	noansi_nr.txt \
	multiple_scanners_nr_1.h \
	multiple_scanners_nr_2.h \
	multiple_scanners_r_1.h \
	multiple_scanners_r_2.h \
	noansi_r.txt \
	prefix_nr.txt \
	prefix_r.txt \
	reject.txt \
	reject_nr.reject.c \
	reject_r.reject.c \
	reject_ver.table.c \
	reject_ver.table.tables \
	reject_ser.table.c \
	reject_ser.table.tables \
	rescan_nr.direct.txt \
	rescan_r.direct.txt \
	quotes.txt \
	top.h \
	top.txt \
	yyextra.txt \
	options.cn

# specify how to process .l files in order to test the flex built by make all

FLEX = $(top_builddir)/src/flex

.l.c:
	$(FLEX) -o $@ $<

.ll.cc:
	$(FLEX) -+ -o $@ $<

bison_nr_main.($OBJEXT): bison_nr_parser.h bison_nr_scanner.h
bison_nr_scanner.h: bison_nr_scanner.c

bison_yylloc_main.$(OBJEXT): bison_yylloc_parser.h bison_yylloc_scanner.h
bison_yylloc_scanner.h: bison_yylloc_scanner.c

bison_yylval_main.$(OBJEXT): bison_yylval_parser.h bison_yylval_scanner.h
bison_yylval_scanner.h: bison_yylval_scanner.c

# automake does not support compiling flex scanners output in C as C++
# so we explicitly sayhow, using the .lll suffix for the lex input file

.lll.cc:
	$(FLEX) -o $@ $<

header_nr_main.$(OBJEXT): header_nr_scanner.h
header_nr_scanner.h: header_nr_scanner.c

header_r_main.$(OBJEXT): header_r_scanner.h
header_r_scanner.h: header_r_scanner.c

include_by_buffer.direct$(EXE): include_by_buffer.direct.txt include_by_buffer.direct_2.txt include_by_buffer.direct_3.txt

include_by_push.direct$(EXE): include_by_push.direct.txt include_by_push.direct_2.txt include_by_push.direct_3.txt

include_by_reentrant.direct$(EXE): include_by_reentrant.direct.txt include_by_reentrant.direct_2.txt include_by_reentrant.direct_3.txt

multiple_scanners_nr_main.$(OBJEXT): multiple_scanners_nr_1.h multiple_scanners_nr_2.h
multiple_scanners_nr_1.h: multiple_scanners_nr_1.c
multiple_scanners_nr_2.h: multiple_scanners_nr_2.c

multiple_scanners_r_main.$(OBJEXT): multiple_scanners_r_1.h multiple_scanners_r_2.h
multiple_scanners_r_1.h: multiple_scanners_r_1.c
multiple_scanners_r_2.h: multiple_scanners_r_2.c

posixly_correct.c: posixly_correct.l
	POSIXLY_CORRECT=1 $(FLEX) -o $@ $<

reject_nr.reject.c: reject.l4
	$(FLEX) -o $@ $<

reject_nr.reject$(EXEEXT): reject_nr.reject.$(OBJEXT)
	$(LINK) $^

reject_r.reject.c: reject.l4
	$(FLEX) --reentrant -o $@ $<

reject_r.reject.$(OBJEXT): reject_r.reject.c
	$(COMPILE) -DTEST_IS_REENTRANT -c -o $@ $<

reject_r.reject$(EXEEXT): reject_r.reject.$(OBJEXT)
	$(LINK) $^

reject_ver.table.c: reject.l4
	$(FLEX) -o $@ --tables-verify --tables-file=$(basename $@).tables $<

reject_ver.table.$(OBJEXT): reject_ver.table.c
	$(COMPILE) -DTEST_HAS_TABLES_EXTERNAL -c -o $@ $<

reject_ver.table$(EXEEXT): reject_ver.table.$(OBJEXT)
	$(LINK) $^

reject_ser.table.c: reject.l4
	$(FLEX) -o $@ --tables-file=$(basename $@).tables $<

reject_ser.table.$(OBJEXT): reject_ser.table.c
	$(COMPILE) -DTEST_HAS_TABLES_EXTERNAL -c -o $@ $<

reject_ser.table$(EXEEXT): reject_ser.table.$(OBJEXT)
	$(LINK) $^

top_main.$(OBJEXT): top.h
top.h: top.c
