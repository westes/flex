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

# Notes on building:

# Possible values for DEFS:
#
# By default, flex generates 8-bit scanners when using table compression,
# and 7-bit scanners when using uncompressed tables (-f or -F options).
# For flex to always generate 8-bit scanners, add "-DDEFAULT_CSIZE=256"
# to DEFS.
#
# For Vax/VMS, add "-DVMS" to DEFS.
#
# For MS-DOS, add "-DMS_DOS" to DEFS.  See the directory MISC/MSDOS for
# additional info.

YFLAGS = -d
ACLOCAL_AMFLAGS = -I m4
help2man = @HELP2MAN@

AUTOMAKE_OPTIONS = \
	readme-alpha \
	gnits \
	check-news \
	no-texinfo.tex \
	readme-alpha

bin_PROGRAMS = flex
lib_LIBRARIES = libfl.a

flex_SOURCES = \
	ccl.c \
	dfa.c \
	ecs.c \
	gen.c \
	main.c \
	misc.c \
	nfa.c \
	parse.y \
	scan.l \
	skel.c \
	sym.c \
	tblcmp.c \
	yylex.c \
	options.c \
	scanopt.c \
	buf.c

libfl_a_SOURCES = \
	libmain.c \
	libyywrap.c

noinst_HEADERS = \
	flexdef.h \
	parse.h \
	version.h \
	options.h \
	scanopt.h

include_HEADERS = \
	FlexLexer.h

info_TEXINFOS =	flex.texi
man_MANS = flex.1

EXTRA_DIST = \
	AUTHORS \
	COPYING \
	INSTALL \
	NEWS \
	ONEWS \
	README \
	RoadMap \
	THANKS \
	TODO \
	autogen.sh \
	flex.skl \
	mkskel.sh \
	ABOUT-NLS \
	config.rpath \
	gettext.h \
	$(man_MANS)

BUILT_SOURCES = \
	skel.c \
	parse.c \
	scan.c

SUBDIRS = \
	m4  \
	examples \
	po

localedir = $(datadir)/locale
INCLUDES = -DLOCALEDIR=\"$(localedir)\" -I@includedir@
LIBS = @LIBINTL@ @LIBS@

# We have a few dependencies and rules to enumerate. (from the old
# Makefile.in by Vern)

scan.o: scan.c parse.h flexdef.h config.h
yylex.o: yylex.c parse.h flexdef.h config.h

skel.c: flex.skl mkskel.sh
	$(SHELL) $(srcdir)/mkskel.sh $(srcdir)/flex.skl >skel.c

$(man_MANS): $(srcdir)/main.c
	for i in $(man_MANS) ; do \
	$(help2man) --name='fast lexical analyzer generator' --section=`echo $$i|rev|cut -f 1 -d .|rev` $(srcdir)/flex > $$i ; \
	done

main.o: main.c flexdef.h config.h version.h
ccl.o: ccl.c flexdef.h config.h
dfa.o: dfa.c flexdef.h config.h
ecs.o: ecs.c flexdef.h config.h
gen.o: gen.c flexdef.h config.h
misc.o: misc.c flexdef.h config.h
nfa.o: nfa.c flexdef.h config.h
parse.o: parse.c flexdef.h config.h
skel.o: skel.c flexdef.h config.h
sym.o: sym.c flexdef.h config.h
tblcmp.o: tblcmp.c flexdef.h config.h
options.o: options.c options.h scanopt.h flexdef.h config.h
scanopt.o: scanopt.c scanopt.h flexdef.h config.h
buf.o: buf.c flexdef.h

#The below recipe for making the ChangeLog will only work if you have a copy of the cvs tree handy so let it fail gracefully.

ChangeLog:
	-rcs2log -l 72 -u "vern	Vern	vern@ee.lbl.gov" -u "wlestes	W. L. Estes	wlestes@uncg.edu" -u "millaway	John Millaway	johnmillaway@yahoo.com" | sed s:/usr/local/cvsroot/flex/::g >ChangeLog 

.PHONY: ChangeLog
