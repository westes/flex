AUTOMAKE_OPTIONS = \
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
	yylex.c

libfl_a_SOURCES = \
	libmain.c \
	libyywrap.c

noinst_HEADERS = \
	flexdef.h \
	version.h

include_HEADERS = \
	FlexLexer.H

info_TEXINFOS = \
	flex.texi
