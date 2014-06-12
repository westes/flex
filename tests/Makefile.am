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

TESTS = $(check_PROGRAMS)

LOG_COMPILER = $(srcdir)/testwrapper.sh
AM_LOG_FLAGS = $(srcdir)

AM_YFLAGS = -d -p test

check_PROGRAMS = \
	alloc_extra \
	array_nr \
	array_r \
	basic_nr \
	basic_r \
	bison_nr \
	bison_yylloc \
	bison_yylval \
	ccl \
	debug_nr \
	debug_r \
	extended \
	header_nr \
	header_r \
	mem_nr \
	mem_r \
	noansi_nr \
	noansi_r \
	posix

alloc_extra_SOURCES = alloc_extra.l
array_nr_SOURCES = array_nr.l
array_r_SOURCES = array_r.l
basic_nr_SOURCES = basic_nr.l
basic_r_SOURCES = basic_r.l
bison_nr_SOURCES = bison_nr_scanner.l bison_nr_parser.y bison_nr_main.c
bison_yylloc_SOURCES = bison_yylloc_scanner.l bison_yylloc_parser.y bison_yylloc_main.c
bison_yylval_SOURCES = bison_yylval_scanner.l bison_yylval_parser.y bison_yylval_main.c
ccl_SOURCES = ccl.l
debug_nr_SOURCES = debug_nr.l
debug_r_SOURCES = debug_r.l
extended_SOURCES = extended.l
header_nr_SOURCES = header_nr_scanner.l header_nr_main.c
header_r_SOURCES = header_r_scanner.l header_r_main.c
mem_nr_SOURCES = mem_nr.l
mem_r_SOURCES = mem_r.l
noansi_nr_SOURCES = noansi_nr.l
noansi_r_SOURCES = noansi_r.l
posix_SOURCES = posix.l

BUILT_SOURCES = \
	bison_nr_scanner.h \
	bison_nr_parser.h \
	bison_yylloc_parser.h \
	bison_yylloc_scanner.h \
	bison_yylval_parser.h \
	bison_yylval_scanner.h \
	header_nr_scanner.h \
	header_r_scanner.h

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
	ccl.txt \
	debug_nr.txt \
	debug_r.txt \
	extended.txt \
	header_nr_scanner.h \
	header_nr.txt \
	header_r_scanner.h \
	header_r.txt \
	mem_nr.txt \
	mem_r.txt \
	noansi_nr.txt \
	noansi_r.txt

# specify how to process .l files in order to test the flex built by make all

FLEX = $(top_builddir)/src/flex

.l.c:
	$(FLEX) -o $@ $<

bison_nr_main($objext): bison_nr_parser.h bison_nr_scanner.h
bison_nr_scanner.h: bison_nr_scanner.c

bison_yylloc_main$(objext): bison_yylloc_parser.h bison_yylloc_scanner.h
bison_yylloc_scanner.h: bison_yylloc_scanner.c

bison_yylval_main$(objext): bison_yylval_parser.h bison_yylval_scanner.h
bison_yylval_scanner.h: bison_yylval_scanner.c

header_nr_main$(objext): header_nr_scanner.h
header_nr_scanner.h: header_nr_scanner.c

header_r_main$(objext): header_r_scanner.h
header_r_scanner.h: header_r_scanner.c
