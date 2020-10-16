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
# in the suite end in .reject, .table and the like so that we can pass
# different arguments to the test runner. We list those extensions so
# automake knows how to distinguish between the various kinds of tests
# we have.

TEST_EXTENSIONS = .reject .table .direct .cn .i3 .pthread .one .opt .ser .ver

LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper.sh
AM_LOG_FLAGS = -d $(srcdir) -r

REJECT_LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper.sh
AM_REJECT_LOG_FLAGS = -d $(srcdir) -i $(srcdir)/reject.txt -r

TABLE_LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper.sh
AM_TABLE_LOG_FLAGS = -d $(builddir) -i $(srcdir)/reject.txt -t

DIRECT_LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper-direct.sh
AM_DIRECT_LOG_FLAGS = -b $(abs_builddir) -s $(srcdir)

CN_LOG_COMPILER = $(SHELL) $(srcdir)/options.cn
AM_CN_LOG_FLAGS = $(FLEX)

I3_LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper.sh
AM_I3_LOG_FLAGS = -i $(srcdir)/cxx_yywrap.txt -i $(srcdir)/cxx_yywrap.txt -i $(srcdir)/cxx_yywrap.txt

PTHREAD_LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper.sh
AM_PTHREAD_LOG_FLAGS = -i $(srcdir)/pthread_1.txt -i $(srcdir)/pthread_2.txt -i $(srcdir)/pthread_3.txt -i $(srcdir)/pthread_4.txt -i $(srcdir)/pthread_5.txt

ONE_LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper.sh
AM_ONE_LOG_FLAGS = -1 -d $(srcdir)

AM_YFLAGS = -d -p test
AM_CPPFLAGS = -I$(top_builddir)/src -I$(top_srcdir)/src

check_PROGRAMS = $(simple_tests) $(reject_tests) $(TABLE_TESTS) $(DIRECT_TESTS) $(I3_TESTS) $(PTHREAD_TESTS) $(ONE_TESTS) $(TABLEOPTS_TESTS)

simple_tests = \
	alloc_extra_cpp \
	alloc_extra_c99 \
	array_nr \
	array_r \
	array_c99 \
	basic_nr \
	basic_r \
	basic_c99 \
	bison_nr \
	bison_yylloc \
	bison_yylval \
	c_cxx_nr \
	c_cxx_r \
	ccl_cpp \
	ccl_c99 \
	cxx_basic \
	cxx_multiple_scanners \
	cxx_restart \
	debug_nr \
	debug_r \
	debug_c99 \
	extended_cpp \
	extended_c99 \
	header_nr \
	header_r \
	mem_nr \
	mem_r \
	mem_c99 \
	multiple_scanners_nr \
	multiple_scanners_r \
	posix_cpp \
	posix_c99 \
	posixly_correct_cpp \
	posixly_correct_c99 \
	prefix_nr \
	prefix_r \
	prefix_c99 \
	quote_in_comment_cpp \
	quote_in_comment_c99 \
	quotes_cpp \
	quotes_c99 \
	string_nr \
	string_c99 \
	top \
	yyextra_cpp \
	yyextra_c99

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

PTHREAD_TESTS = \
	pthread.pthread

ONE_TESTS = \
	lineno_nr.one \
	lineno_r.one \
	lineno_trailing.one

quote_in_comment_cpp_SOURCES = quote_in_comment_cpp.l
quote_in_comment_c99_SOURCES = quote_in_comment_c99.l
alloc_extra_cpp_SOURCES = alloc_extra_cpp.l
alloc_extra_c99_SOURCES = alloc_extra_c99.l
array_nr_SOURCES = array_nr.l
array_r_SOURCES = array_r.l
array_c99_SOURCES = array_c99.l
basic_nr_SOURCES = basic_nr.l
basic_r_SOURCES = basic_r.l
basic_c99_SOURCES = basic_c99.l
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
ccl_cpp_SOURCES = ccl_cpp.l
ccl_c99_SOURCES = ccl_c99.l
cxx_basic_SOURCES = cxx_basic.ll
cxx_restart_SOURCES = cxx_restart.ll
cxx_multiple_scanners_SOURCES = cxx_multiple_scanners_main.cc cxx_multiple_scanners_1.ll cxx_multiple_scanners_2.ll
cxx_yywrap_i3_SOURCES = cxx_yywrap.ll
debug_nr_SOURCES = debug_nr.l
debug_r_SOURCES = debug_r.l
debug_c99_SOURCES = debug_c99.l
extended_cpp_SOURCES = extended_cpp.l
extended_c99_SOURCES = extended_c99.l
header_nr_SOURCES = header_nr_scanner.l header_nr_main.c
nodist_header_nr_SOURCES = header_nr_scanner.h
header_r_SOURCES = header_r_scanner.l header_r_main.c
nodist_header_r_SOURCES = header_r_scanner.h
include_by_buffer_direct_SOURCES = include_by_buffer.direct.l
include_by_push_direct_SOURCES = include_by_push.direct.l
include_by_reentrant_direct_SOURCES = include_by_reentrant.direct.l
lineno_nr_one_SOURCES = lineno_nr.l
lineno_r_one_SOURCES = lineno_r.l
lineno_trailing_one_SOURCES = lineno_trailing.l
mem_nr_SOURCES = mem_nr.l
mem_r_SOURCES = mem_r.l
mem_c99_SOURCES = mem_c99.l
multiple_scanners_nr_SOURCES = multiple_scanners_nr_main.c multiple_scanners_nr_1.l multiple_scanners_nr_2.l
nodist_multiple_scanners_nr_SOURCES = multiple_scanners_nr_1.h multiple_scanners_nr_2.h
multiple_scanners_r_SOURCES = multiple_scanners_r_main.c multiple_scanners_r_1.l multiple_scanners_r_2.l
nodist_multiple_scanners_r_SOURCES = multiple_scanners_nr_1.h multiple_scanners_nr_2.h
posix_cpp_SOURCES = posix_cpp.l
posix_c99_SOURCES = posix_c99.l
posixly_correct_cpp__SOURCES = posixly_correct_cp.l
posixly_correct_c99_SOURCES = posixly_correct_c99.l
prefix_nr_SOURCES = prefix_nr.l
prefix_r_SOURCES = prefix_r.l
prefix_c99_SOURCES = prefix_c99.l
pthread_pthread_SOURCES = pthread.l
quotes_cpp_SOURCES = quotes_cpp.l
quotes_c99_SOURCES = quotes_c99.l
reject_nr_reject_SOURCES = reject.l4
reject_r_reject_SOURCES = reject.l4
reject_ver_table_SOURCES = reject.l4
reject_ser_table_SOURCES = reject.l4
rescan_nr_direct_SOURCES = rescan_nr.direct.l
rescan_r_direct_SOURCES = rescan_r.direct.l
string_nr_SOURCES = string_nr.l
string_r_SOURCES = string_r.l
string_c99_SOURCES = string_c99.l
top_SOURCES = top.l top_main.c
nodist_top_SOURCES = top.h
yyextra_cpp_SOURCES = yyextra_cpp.l
yyextra_c99_SOURCES = yyextra_c99.l

# Normally, automake would distribute files built by flex. Since the
# point of the test suite is to test the files that flex builds, and
# since anyone who has the flex distribution can build a flex binary
# without having a flex binary, we want to not distribute the files
# built by flex in the test suite. The dist-hook target accomplishes
# this goal. It's also handy to have the list of files to clean since
# automake doesn't know about the whacky ways in which we're abusing
# it.

CLEANFILES = \
	alloc_extra_cpp.c \
	alloc_extra_c99.c \
	array_nr.c \
	array_r.c \
	array_c99.c \
	basic_nr.c \
	basic_r.c \
	basic_c99.c \
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
	ccl_cpp.c \
	cxx_basic.cc \
	cxx_multiple_scanners_1.cc \
	cxx_multiple_scanners_2.cc \
	cxx_restart.cc \
	cxx_yywrap.cc \
	debug_nr.c \
	debug_r.c \
	debug_c99.c \
	extended_cpp.c \
	extended_c99.c \
	header_nr_scanner.c \
	header_nr_scanner.h \
	header_r_scanner.c \
	header_r_scanner.h \
	include_by_buffer.direct.c \
	include_by_push.direct.c \
	include_by_reentrant.direct.c \
	lineno_nr.c \
	lineno_r.c \
	lineno_trailing.c \
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
	posix_cpp.c \
	posix_c99.c \
	posixly_correct_cpp.c \
	posixly_correct_c99.c \
	prefix_nr.c \
	prefix_r.c \
	prefix_c99.c \
	pthread.c \
	quote_in_comment_cpp.c \
	quote_in_comment_c99.c \
	quotes_cpp.c \
	reject_nr.reject.c \
	reject_r.reject.c \
	reject_ser.table.c \
	reject_ser.table.tables \
	reject_ver.table.c \
	reject_ver.table.tables \
	rescan_nr.direct.c \
	rescan_r.direct.c \
	string_nr.c \
	string_r.c \
	string_c99.c \
	top.c \
	top.h  \
	yyextra_cpp.c \
	yyextra_c99.c \
	$(tableopts_c) \
	$(tableopts_tables)

dist-hook:
	chmod u+w $(distdir) && \
	for file in $(CLEANFILES) ; do \
	rm -f $(distdir)/$$file \
	; done

EXTRA_DIST = \
	README \
	alloc_extra_cpp.txt \
	alloc_extra_c99.txt \
	array_nr.txt \
	array_r.txt \
	array_c99.txt \
	basic_nr.txt \
	basic_r.txt \
	bison_nr.txt \
	bison_yylloc.txt \
	bison_yylval.txt \
	c_cxx_nr.txt \
	c_cxx_r.txt \
	ccl_cpp.txt \
	cxx_basic.txt \
	cxx_multiple_scanners.txt \
	cxx_restart.txt \
	cxx_yywrap.txt \
	debug_nr.txt \
	debug_r.txt \
	debug_c99.txt \
	extended_cpp.txt \
	extended_c99.txt \
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
	lineno_nr.one.txt \
	lineno_r.one.txt \
	lineno_trailing.one.txt \
	mem_nr.txt \
	mem_r.txt \
	mem_c99.txt \
	prefix_nr.txt \
	prefix_r.txt \
	prefix_c99.txt \
	pthread_1.txt \
	pthread_2.txt \
	pthread_3.txt \
	pthread_4.txt \
	pthread_5.txt \
	reject.txt \
	rescan_nr.direct.txt \
	rescan_r.direct.txt \
	quote_in_comment_cpp.txt \
	quote_in_comment_c99.txt \
	quotes_cpp.txt \
	quotes_c99.txt \
	top.txt \
	yyextra.txt \
	tableopts.txt

dist_noinst_SCRIPTS = \
	tableopts.sh

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

posixly_correct_cpp.c: posixly_correct_cpp.l $(FLEX)
	$(AM_V_LEX)POSIXLY_CORRECT=1 $(FLEX) $(TESTOPTS) -o $@ $<

posixly_correct_c99.c: posixly_correct_c99.l $(FLEX)
	$(AM_V_LEX)POSIXLY_CORRECT=1 $(FLEX) $(TESTOPTS) -o $@ $<

reject_nr.reject.c: reject.l4 $(FLEX)
	$(AM_V_LEX)$(FLEX) $(TESTOPTS) --unsafe-no-m4-sect3-escape -o $@ $<

reject_nr.reject$(EXEEXT): reject_nr.reject.$(OBJEXT)
	$(AM_V_CCLD)$(LINK) $^

reject_r.reject.c: reject.l4 $(FLEX)
	$(AM_V_LEX)$(FLEX) $(TESTOPTS) --unsafe-no-m4-sect3-escape --reentrant -o $@ $<

reject_r.reject.$(OBJEXT): reject_r.reject.c
	$(AM_V_CC)$(COMPILE) -DTEST_IS_REENTRANT -c -o $@ $<

reject_r.reject$(EXEEXT): reject_r.reject.$(OBJEXT)
	$(AM_V_CCLD)$(LINK) $^

reject_ver.table.c: reject.l4 $(FLEX)
	$(AM_V_LEX)$(FLEX) $(TESTOPTS) --unsafe-no-m4-sect3-escape -o $@ --tables-verify --tables-file=$(basename $@).tables $<

reject_ver.table.$(OBJEXT): reject_ver.table.c
	$(AM_V_CC)$(COMPILE) -DTEST_HAS_TABLES_EXTERNAL -c -o $@ $<

reject_ver.table$(EXEEXT): reject_ver.table.$(OBJEXT)
	$(AM_V_CCLD)$(LINK) $^

reject_ser.table.c: reject.l4 $(FLEX)
	$(AM_V_LEX)$(FLEX) $(TESTOPTS) -o $@ --unsafe-no-m4-sect3-escape --tables-file=$(basename $@).tables $<

reject_ser.table.$(OBJEXT): reject_ser.table.c
	$(AM_V_CC)$(COMPILE) -DTEST_HAS_TABLES_EXTERNAL -c -o $@ $<

reject_ser.table$(EXEEXT): reject_ser.table.$(OBJEXT)
	$(AM_V_CCLD)$(LINK) $^

top_main.$(OBJEXT): top.h
top.h: top.c

# We separate out the tableopts _SOURCES variables and the linking
# rules for those programs because automake has no way to specify such
# things with a loop in a variable (even though make can do such
# things) and the resultant list is both long an unenlightening. And
# it can be / is generated by a shell script, tableopts.sh.

$(srcdir)/tableopts.am: tableopts.sh
	$(SHELL) $(srcdir)/tableopts.sh > $(srcdir)/tableopts.am

include $(srcdir)/tableopts.am

tableopts   := -Ca -Ce -Cf -CF -Cm -Cem -Cae -Caef -CaeF -Cam -Caem
tableopts_opt_tests :=  $(foreach opt,$(tableopts), tableopts_opt_nr$(opt) tableopts_opt_r$(opt))
tableopts_sertests :=  $(foreach opt,$(tableopts), tableopts_ser_nr$(opt) tableopts_ser_r$(opt))
tableopts_vertests :=  $(foreach opt,$(tableopts), tableopts_ver_nr$(opt) tableopts_ver_r$(opt))
tableopts_tests  := $(tableopts_opttests) $(tableopts_vertests) $(tableopts_sertests)

tableopts_c := $(addsuffix .c,$(tableopts_tests))

OPT_LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper.sh
AM_OPT_LOG_FLAGS = -d $(srcdir) -i $(srcdir)/tableopts.txt -r

tableopts_opt_nr%.c: tableopts.l4 $(FLEX)
	$(AM_V_LEX)$(FLEX) $(TESTOPTS) --unsafe-no-m4-sect3-escape -P $(subst -,_,$(basename $(*F))) $(*:_F=F) -o $@ $<

tableopts_opt_nr%.$(OBJEXT): tableopts_opt_nr%.c 
	$(AM_V_CC)$(COMPILE) -c -o $@ $<

tableopts_opt_r%.c: tableopts.l4 $(FLEX)
	$(AM_V_LEX)$(FLEX) $(TESTOPTS) --unsafe-no-m4-sect3-escape -P $(subst -,_,$(basename $(*F))) --reentrant $(subst _F,F,$*)  -o $@ $<

tableopts_opt_r%.$(OBJEXT):  tableopts_opt_r%.c 
	$(AM_V_CC)$(COMPILE) -DTEST_IS_REENTRANT -c -o $@ $<

SER_LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper.sh
AM_SER_LOG_FLAGS = -d $(builddir) -i $(srcdir)/tableopts.txt -r -t

tableopts_ser_nr%.c: tableopts.l4 $(FLEX)
	$(AM_V_LEX)$(FLEX) $(TESTOPTS) --unsafe-no-m4-sect3-escape -P $(subst -,_,$(basename $(*F))) --tables-file="tableopts_ser_nr$*.ser.tables"  $(subst _F,F,$*) -o $@ $<

tableopts_ser_nr%.$(OBJEXT): tableopts_ser_nr%.c 
	$(AM_V_CC)$(COMPILE) -DTEST_HAS_TABLES_EXTERNAL -c -o $@ $<

tableopts_ser_r%.c: tableopts.l4 $(FLEX)
	$(AM_V_LEX)$(FLEX) $(TESTOPTS) --unsafe-no-m4-sect3-escape -P $(subst -,_,$(basename $(*F))) -R --tables-file="tableopts_ser_r$*.ser.tables" $(subst _F,F,$*)  -o $@ $<

tableopts_ser_r%.$(OBJEXT):  tableopts_ser_r%.c
	$(AM_V_CC)$(COMPILE) -DTEST_HAS_TABLES_EXTERNAL -DTEST_IS_REENTRANT -c -o $@ $<

VER_LOG_COMPILER = $(SHELL) $(srcdir)/testwrapper.sh
AM_VER_LOG_FLAGS = -d $(builddir) -i $(srcdir)/tableopts.txt -r -t

tableopts_ver_nr%.c: tableopts.l4 $(FLEX)
	$(AM_V_LEX)$(FLEX) $(TESTOPTS) --unsafe-no-m4-sect3-escape -P $(subst -,_,$(basename $(*F))) --tables-file="tableopts_ver_nr$*.ver.tables" --tables-verify $(subst _F,F,$*) -o $@ $<

tableopts_ver_nr%.$(OBJEXT): tableopts_ver_nr%.c 
	$(AM_V_CC)$(COMPILE) -DTEST_HAS_TABLES_EXTERNAL -c -o $@ $<

tableopts_ver_nr%.ver$(EXEEXT): tableopts_ver_nr%.$(OBJEXT)
	$(AM_V_CCLD)$(LINK) -o $@ $^

tableopts_ver_r%.c: tableopts.l4 $(FLEX)
	$(AM_V_LEX)$(FLEX) $(TESTOPTS) --unsafe-no-m4-sect3-escape -P $(subst -,_,$(basename $(*F))) -R --tables-file="tableopts_ver_r$*.ver.tables" --tables-verify $(subst _F,F,$*)  -o $@ $<

tableopts_ver_r%.$(OBJEXT):  tableopts_ver_r%.c  
	$(AM_V_CC)$(COMPILE) -DTEST_HAS_TABLES_EXTERNAL -DTEST_IS_REENTRANT -c -o $@ $<
