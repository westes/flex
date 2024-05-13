if CROSS
FLEX_FOR_DOC = stage1bin/flex
else
FLEX_FOR_DOC = flex
endif

TEXI2DVI = @TEXI2DVI@ -I $(srcdir)/examples/manual/
TEXI2PDF = @TEXI2PDF@ -I $(srcdir)/examples/manual/

info_TEXINFOS = doc/flex.texi
AM_MAKEINFOFLAGS = -I $(srcdir)/examples/manual/
dist_man_MANS = doc/flex.1
MAINTAINERCLEANFILES = doc/flex.1

CLEANFILES += \
	doc/*.aux \
	doc/*.cp \
	doc/*.cps \
	doc/*.fn \
	doc/*.fns \
	doc/*.hk \
	doc/*.hks \
	doc/*.ky \
	doc/*.log \
	doc/*.op \
	doc/*.ops \
	doc/*.pg \
	doc/*.toc \
	doc/*.tp \
	doc/*.tps \
	doc/*.vr \
	doc/*.vrs \
	doc/flex

# Use a fixed program name, without extension (such as ".exe"), for man
# page generation. 'help2man' strips directory prefix from the usage
# string, but not file extensions.
#
# Note: Do NOT run the '$(FLEX_FOR_DOC)$(BUILD_EXEEXT)' target in
# another instance of 'make'! The scanner code also depends on the
# target and two 'make' instances may contest building the same file.

if AUTO_BUILD_MAN_PAGE
doc/flex.1: $(FLEX_FOR_DOC)$(BUILD_EXEEXT)
	$(MKDIR_P) doc
	$(HELP2MAN) \
	--name='$(PACKAGE_NAME)' \
	--section=1 \
	--source='The Flex Project' \
	--manual='Programming' \
	--output=$@ \
	./$(FLEX_FOR_DOC)
else
doc/flex.1: $(srcdir)/configure.ac $(srcdir)/src/cpp-flex.skl $(srcdir)/src/options.c $(srcdir)/src/options.h
	@option_text=''; \
	{ test '$(FLEX_FOR_DOC)' = flex || \
	  : $${option_text=" with --enable-bootstrap'"}; }; \
	echo "error: flex man page outdated; please install help2man and rerun 'configure'$${option_text}" 1>&2;
	@false
endif

distclean-local: distclean-local-doc

# Clean the man page in the build directory only if it is not the
# source directory.

distclean-local-doc:
	test '$(srcdir)' = . || { \
	  if mv -f $(srcdir)/doc/flex.1 $(srcdir)/doc/flex.1.tmp; then \
	  s=0; else s=$$? || :; fi; \
	  rm -f doc/flex.1 || :; \
	  test "$$s" -ne 0 || \
	    mv -f $(srcdir)/doc/flex.1.tmp $(srcdir)/doc/flex.1; \
	}

.PHONY: distclean-local-doc
