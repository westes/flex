AM_YFLAGS = -d
AM_CPPFLAGS = -I$(srcdir)/src -DLOCALEDIR=\"$(localedir)\"
AM_CFLAGS = $(WARNINGFLAGS)
LIBS = @LIBS@
pkgconfigdir = @pkgconfigdir@

m4 = @M4@

bin_PROGRAMS = flex

EXTRA_PROGRAMS = stage1flex
if ENABLE_BOOTSTRAP
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

stage1flex_SOURCES += \
	lib/malloc.c \
	lib/realloc.c

stage1flex_CPPFLAGS = -DUSE_CONFIG_FOR_BUILD $(AM_CPPFLAGS)

# This also overrides AM_LIBTOOLFLAGS and AM_LDFLAGS
stage1flex_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC $(LIBTOOLFLAGS) \
	--mode=link $(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) \
	$(LDFLAGS_FOR_BUILD) -o $@

stage1flex_LDADD =

else

stage1flex_CPPFLAGS = $(AM_CPPFLAGS)

stage1flex_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC $(LIBTOOLFLAGS) \
	--mode=link $(CCLD) $(CFLAGS) $(LDFLAGS) -o $@

stage1flex_LDADD = $(LDADD)

endif

# Suppress build warnings when compiling and linking stage1flex.
stage1flex_CFLAGS =

# Override the 'stage1flex$(EXEEXT)' target automake would generate.
# If the compiler is not $(CC_FOR_BUILD) when this target is invoked,
# delete the object files that would be linked to 'stage1flex' to force
# rebuild all of them.

stage1flex$(EXEEXT): $(stage1flex_OBJECTS) $(stage1flex_DEPENDENCIES) $(EXTRA_stage1flex_DEPENDENCIES)
	@rm -f stage1flex$(EXEEXT)
	{ test 'x$(CC)' = 'x$(CC_FOR_BUILD)' && \
	  test 'x$(CPP)' = 'x$(CPP_FOR_BUILD)'; } || \
	  { rm -f src/stage1flex-*.$(OBJEXT) \
	    src/$(DEPDIR)/stage1flex-*.Po || :; }
	@{ test 'x$(CC)' = 'x$(CC_FOR_BUILD)' && test 'x$(CPP)' = 'x$(CPP_FOR_BUILD)'; } || { \
	  echo 'error: stage1flex must be built with a native compiler' 1>&2; \
	  exit 1; \
	}
	$(AM_V_CCLD)$(stage1flex_LINK) $(stage1flex_OBJECTS) $(stage1flex_LDADD) $(LIBS)

# The 'stage1bin/flex$(BUILD_EXEEXT)' target is not managed by
# automake. We use a different file name from the automake-managed
# 'stage1flex$(EXEEXT)' target to avoid clashing.

stage1bin/flex$(BUILD_EXEEXT):
	@if test 'x$(EXEEXT)' != 'x$(BUILD_EXEEXT)'; then rm -f stage1flex$(EXEEXT); else :; fi
	@if test 'x$(OBJEXT)' != 'x$(BUILD_OBJEXT)'; then rm -f src/stage1flex-*.$(OBJEXT); else :; fi
	$(MAKE) $(AM_MAKEFLAGS) \
	  CC='$(CC_FOR_BUILD)' \
	  CPP='$(CPP_FOR_BUILD)' \
	  CFLAGS='$(CFLAGS_FOR_BUILD)' \
	  CPPFLAGS='$(CPPFLAGS_FOR_BUILD)' \
	  LDFLAGS='$(LDFLAGS_FOR_BUILD)' \
	  EXEEXT='$(BUILD_EXEEXT)' \
	  OBJEXT='$(BUILD_OBJEXT)' \
	  stage1flex$(BUILD_EXEEXT)
	$(MKDIR_P) stage1bin
	$(INSTALL) -m 700 stage1flex$(BUILD_EXEEXT) $@

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
	src/stage1flex-*.$(BUILD_OBJEXT) \
	src/stage1scan.c \
	src/stage2scan.c \
	src/stage2compare

CLEANFILES += \
	stage1bin/flex$(BUILD_EXEEXT) \
	stage1flex$(BUILD_EXEEXT) \
	stage1flex$(EXEEXT)

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
src/stage1scan.c: src/scan.l stage1bin/flex$(BUILD_EXEEXT)
	( cd $(srcdir)/src && $(abs_builddir)/stage1bin/flex$(BUILD_EXEEXT) \
	  $(AM_LFLAGS) $(LFLAGS) -o scan.c -t scan.l ) >$@ || \
	{ s=$$?; rm -f $@; exit $$s; }

# Unlike stage1scan.c, we leave stage2scan.c intact when the generation
# fails. This allow users to examine generation errors.
src/stage2scan.c: src/scan.l flex$(EXEEXT)
	( cd $(srcdir)/src && $(abs_builddir)/flex$(EXEEXT) \
	  $(AM_LFLAGS) $(LFLAGS) -o scan.c -t scan.l ) >$@


src/stage2compare: src/stage1scan.c src/stage2scan.c
	@echo Comparing stage1scan.c and stage2scan.c; \
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

indentfiles += \
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

.PHONY: dist-hook-src src/stage2scan.c
