if CROSS
FLEX_FOR_DOC = $(top_builddir)/stage1flex
else
FLEX_FOR_DOC = $(top_builddir)/flex$(EXEEXT)
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
# page generation. 'help2man' strips directory prefix ("./") from the
# usage string, but not file extensions.

doc/flex.1: $(top_srcdir)/configure.ac $(top_srcdir)/src/cpp-flex.skl $(top_srcdir)/src/options.c $(top_srcdir)/src/options.h
	( cd $(top_builddir) && \
	  prog_name=`echo '$(FLEX_FOR_DOC)' | sed 's|^$(top_builddir)/||'` && \
	  $(MAKE) $(AM_MAKEFLAGS) $$prog_name \
	)
	$(INSTALL) -m 700 $(FLEX_FOR_DOC) doc/flex$(EXEEXT)
	$(HELP2MAN) \
	--name='$(PACKAGE_NAME)' \
	--section=1 \
	--source='The Flex Project' \
	--manual='Programming' \
	--output=$@ \
	doc/flex
