AM_YFLAGS = -d
AM_CPPFLAGS = -I$(srcdir)/src -DLOCALEDIR=\"$(localedir)\"
AM_CFLAGS = $(WARNINGFLAGS)
LIBS = @LIBS@
pkgconfigdir = @pkgconfigdir@

m4 = @M4@

bin_PROGRAMS = flex
if ENABLE_BOOTSTRAP
noinst_PROGRAMS = stage1flex
if !CROSS
noinst_DATA = src/stage2compare
endif
endif

if ENABLE_LIBFL
lib_LTLIBRARIES = libfl.la
pkgconfig_DATA = src/libfl.pc
endif
libfl_la_SOURCES = \
	src/libmain.c \
	src/libyywrap.c
libfl_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@

stage1flex_SOURCES = \
	src/scan.l \
	$(COMMON_SOURCES)

nodist_stage1flex_SOURCES = \
	$(SKELINCLUDES)

if CROSS
stage1flex_LDADD =
stage1flex_SOURCES += \
	lib/malloc.c \
	lib/realloc.c
stage1flex_LINK = $(LIBTOOL) --tag=CC --mode=link $(CC_FOR_BUILD) \
		  $(CFLAGS_FOR_BUILD) $(LDFLAGS_FOR_BUILD) -o $@

$(stage1flex_OBJECTS): CC=$(CC_FOR_BUILD)
$(stage1flex_OBJECTS): CFLAGS=$(CFLAGS_FOR_BUILD) -DUSE_CONFIG_FOR_BUILD
$(stage1flex_OBJECTS): CPP=$(CPP_FOR_BUILD)
$(stage1flex_OBJECTS): CPPFLAGS=$(CPPFLAGS_FOR_BUILD)
$(stage1flex_OBJECTS): LDFLAGS=$(LDFLAGS_FOR_BUILD)
else
stage1flex_LDADD = $(LDADD)
stage1flex_LINK = $(LINK)
stage1flex_CFLAGS = $(AM_CFLAGS)
endif

flex_SOURCES = \
	$(COMMON_SOURCES)

nodist_flex_SOURCES = \
	$(SKELINCLUDES)

if ENABLE_BOOTSTRAP
nodist_flex_SOURCES += src/stage1scan.c
else
flex_SOURCES += src/scan.l
endif

COMMON_SOURCES = \
	src/buf.c \
	src/ccl.c \
	src/dfa.c \
	src/ecs.c \
	src/filter.c \
	src/flexdef.h \
	src/flexint.h \
	src/flexint_shared.h \
	src/gen.c \
	src/main.c \
	src/misc.c \
	src/nfa.c \
	src/options.c \
	src/options.h \
	src/parse.y \
	src/regex.c \
	src/scanflags.c \
	src/scanopt.c \
	src/scanopt.h \
	src/skeletons.c \
	src/sym.c \
	src/tables.c \
	src/tables.h \
	src/tables_shared.c \
	src/tables_shared.h \
	src/tblcmp.c \
	src/version.h \
	src/yylex.c

LDADD = $(LIBOBJS) @LIBINTL@

$(LIBOBJS): $(LIBOBJDIR)$(am__dirstamp)

include_HEADERS = \
	src/FlexLexer.h

EXTRA_DIST += \
	src/c99-flex.skl \
	src/cpp-flex.skl \
	src/go-flex.skl \
	src/mkskel.sh \
	src/gettext.h \
	src/chkskel.sh

MOSTLYCLEANFILES += \
	$(SKELINCLUDES) \
	src/stage1scan.c \
	src/stage2scan.c \
	src/stage2compare

CLEANFILES += stage1flex$(EXEEXT)

SKELINCLUDES = \
	src/cpp-flex.h \
	src/c99-flex.h \
	src/go-flex.h

src/cpp-flex.h: src/cpp-flex.skl src/mkskel.sh src/flexint_shared.h src/tables_shared.h src/tables_shared.c
	$(SHELL) $(srcdir)/src/mkskel.sh cpp $(srcdir)/src $(m4) $(VERSION) > $@.tmp
	$(SHELL) $(srcdir)/src/chkskel.sh $@.tmp
	mv -f $@.tmp $@

src/c99-flex.h: src/c99-flex.skl src/mkskel.sh
	$(SHELL) $(srcdir)/src/mkskel.sh c99 $(srcdir)/src $(m4) $(VERSION) > $@.tmp
	$(SHELL) $(srcdir)/src/chkskel.sh $@.tmp
	mv -f $@.tmp $@

src/go-flex.h: src/go-flex.skl src/mkskel.sh
	$(SHELL) $(srcdir)/src/mkskel.sh go $(srcdir)/src $(m4) $(VERSION) > $@.tmp
	$(SHELL) $(srcdir)/src/chkskel.sh $@.tmp
	mv -f $@.tmp $@

# The input and output file names are fixed for deterministic scanner
# generation. If scan.l is not modified by builders, stage1scan.c should
# be bit-identical to the scan.c pregenerated on release.
src/stage1scan.c: src/scan.l stage1flex$(EXEEXT)
	( cd $(srcdir)/src && $(abs_builddir)/stage1flex$(EXEEXT) \
	  $(AM_LFLAGS) $(LFLAGS) -o scan.c -t scan.l ) >$@ || \
	{ s=$$?; rm -f $@; exit $$s; }

# Unlike stage1scan.c, we leave stage2scan.c intact when the generation
# fails. This allow users to examine generation errors.
src/stage2scan.c: src/scan.l flex$(EXEEXT) src/stage1scan.c
	( cd $(srcdir)/src && $(abs_builddir)/flex$(EXEEXT) \
	  $(AM_LFLAGS) $(LFLAGS) -o scan.c -t scan.l ) >$@

src/stage2compare: src/stage1scan.c
	@rm -f src/stage2scan.c; \
	$(MAKE) $(AM_MAKEFLAGS) src/stage2scan.c; \
	echo Comparing stage1scan.c and stage2scan.c; \
	cmp src/stage1scan.c src/stage2scan.c || { \
	  s=$$?; \
	  echo "Bootstrap comparison failure!"; \
	  exit $$s; \
	}; \
	echo Comparison successful.; \
	echo success >$@

dist-hook: dist-hook-src

dist-hook-src: src/scan.l flex$(EXEEXT)
	chmod u+w $(distdir)/src && \
	chmod u+w $(distdir)/src/scan.c && \
	( cd $(srcdir)/src && $(abs_builddir)/flex$(EXEEXT) \
	  -o scan.c -t scan.l ) >src/scan.c && \
	mv -f src/scan.c $(distdir)/src/scan.c

# make needs to be told to make inclusions so that parallelized runs will
# not fail.

src/stage1flex-skeletons.$(OBJEXT): $(SKELINCLUDES)
src/skeletons.$(OBJEXT): $(SKELINCLUDES)

src/stage1flex-main.$(OBJEXT): src/parse.h
src/main.$(OBJEXT): src/parse.h

src/stage1flex-yylex.$(OBJEXT): src/parse.h
src/yylex.$(OBJEXT): src/parse.h

src/stage1flex-scan.$(OBJEXT): src/parse.h
src/stage1scan.$(OBJEXT): src/parse.h
src/scan.$(OBJEXT): src/parse.h

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
	src/buf.c \
	src/ccl.c \
	src/dfa.c \
	src/ecs.c \
	src/scanflags.c \
	src/filter.c \
	src/flexdef.h \
	src/gen.c \
	src/libmain.c \
	src/libyywrap.c \
	src/main.c \
	src/misc.c \
	src/nfa.c \
	src/options.c \
	src/options.h \
	src/regex.c \
	src/scanopt.c \
	src/scanopt.h \
	src/sym.c \
	src/tables.c \
	src/tables.h \
	src/tables_shared.c \
	src/tables_shared.h \
	src/tblcmp.c

indent: $(top_srcdir)/.indent.pro
	cd $(top_srcdir) && \
	for f in $(indentfiles); do \
		echo indenting $$f; \
		INDENT_PROFILE=.indent.pro $(INDENT) <$$f >/dev/null && \
		INDENT_PROFILE=.indent.pro $(INDENT) $$f || \
		echo $$f FAILED to indent; \
	done;

.PHONY: dist-hook-src indent
