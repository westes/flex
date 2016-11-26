AM_YFLAGS = -d
localedir = $(datadir)/locale
AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\" -I$(top_srcdir)/intl
LIBS = @LIBS@

m4 = @M4@

bin_PROGRAMS = flex
if ENABLE_BOOTSTRAP
noinst_PROGRAMS = stage1flex
endif

if ENABLE_LIBFL
lib_LTLIBRARIES = libfl.la
endif
libfl_la_SOURCES = \
	libmain.c \
	libyywrap.c
libfl_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@

stage1flex_SOURCES = \
	scan.l \
	$(COMMON_SOURCES)

stage1flex_CFLAGS = $(AM_CFLAGS)

flex_SOURCES = \
	$(COMMON_SOURCES)

nodist_flex_SOURCES = \
	stage1scan.c

flex_CFLAGS = $(AM_CFLAGS) $(WARNINGFLAGS)

COMMON_SOURCES = \
	buf.c \
	ccl.c \
	dfa.c \
	ecs.c \
	filter.c \
	flexdef.h \
	flexint.h \
	gen.c \
	main.c \
	misc.c \
	nfa.c \
	options.c \
	options.h \
	parse.y \
	regex.c \
	scanflags.c \
	scanopt.c \
	scanopt.h \
	skel.c \
	sym.c \
	tables.c \
	tables.h \
	tables_shared.c \
	tables_shared.h \
	tblcmp.c \
	version.h \
	yylex.c

LDADD = $(LIBOBJS) @LIBINTL@

include_HEADERS = \
	FlexLexer.h

EXTRA_DIST = \
	flex.skl \
	mkskel.sh \
	gettext.h

CLEANFILES = stage1scan.c stage1flex$(EXEEXT)

MAINTAINERCLEANFILES = skel.c

skel.c: flex.skl mkskel.sh flexint.h tables_shared.h tables_shared.c
	sed 's/4_/a4_/g; s/m4preproc_/m4_/g' $< | \
	  $(m4) -P -I $(srcdir)						   \
	    -DFLEX_MAJOR_VERSION=`   echo $(VERSION)|cut -f 1 -d .`	   \
	    -DFLEX_MINOR_VERSION=`   echo $(VERSION)|cut -f 2 -d .`	   \
	    -DFLEX_SUBMINOR_VERSION=`echo $(VERSION)|cut -f 3 -d .` |	   \
	  $(SHELL) $(srcdir)/mkskel.sh > $@.tmp
	mv $@.tmp $@

if ENABLE_BOOTSTRAP
stage1scan.c: scan.l stage1flex$(EXEEXT)
	./stage1flex$(EXEEXT) $(AM_LFLAGS) $(LFLAGS) -o $@ $<
else
stage1scan.c: scan.c
	sed 's|^\(#line .*\)"'`printf %s $< | sed 's|[][\\\\.*]|\\\\&|g'`'"|\1"$@"|g' $< > $@
endif

# make needs to be told to make parse.h so that parallelized runs will
# not fail.

main.c: parse.h
yylex.c: parse.h
stage1flex-scan.$(OBJEXT): parse.h
flex-stage1scan.$(OBJEXT): parse.h

# Run GNU indent on sources. Don't run this unless all the sources compile cleanly.
#
# Whole idea:
#   1. Check for .indent.pro, otherwise indent will use unknown
#      settings, or worse, the GNU defaults.)
#   2. Check that this is GNU indent.
#   3. Make sure to process only the NON-generated .c and .h files.
#   4. Run indent twice per file. The first time is a test.
#      Otherwise, indent overwrites your file even if it fails!
indentfiles = \
	buf.c \
	ccl.c \
	dfa.c \
	ecs.c \
	scanflags.c \
	filter.c \
	flexdef.h \
	gen.c \
	libmain.c \
	libyywrap.c \
	main.c \
	misc.c \
	nfa.c \
	options.c \
	options.h \
	regex.c \
	scanopt.c \
	scanopt.h \
	sym.c \
	tables.c \
	tables.h \
	tables_shared.c \
	tables_shared.h \
	tblcmp.c

indent:
	if [ -f .indent.pro ] ; then \
	for f in $(indentfiles);\
	do\
		echo indenting $$f ;\
		f='$(srcdir)'/$$f; \
		$(indent) < $$f >/dev/null && indent $$f || echo $$f FAILED to indent ;\
	done \
	fi
