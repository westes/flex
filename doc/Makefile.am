if CROSS
FLEX = $(top_builddir)/src/stage1flex
else
FLEX = $(top_builddir)/src/flex$(EXEEXT)
endif
TEXI2DVI = @TEXI2DVI@ -I $(srcdir)/../examples/manual/
TEXI2PDF = @TEXI2PDF@ -I $(srcdir)/../examples/manual/

info_TEXINFOS =	flex.texi
AM_MAKEINFOFLAGS = -I $(srcdir)/../examples/manual/
dist_man_MANS = flex.1
MAINTAINERCLEANFILES = flex.1

CLEANFILES = *.aux *.cp *.cps *.fn *.fns *.hk *.hks *.ky *.log \
	*.op *.ops *.pg *.toc *.tp *.tps *.vr *.vrs

flex.1: $(top_srcdir)/configure.ac $(top_srcdir)/src/cpp-flex.skl $(top_srcdir)/src/options.c $(top_srcdir)/src/options.h
	@( cd $(top_builddir)/src && \
	  prog_name=`echo '$(FLEX)' | sed 's|^$(top_builddir)/src/||'` && \
	  $(MAKE) $(AM_MAKEFLAGS) $$prog_name \
	)
	$(HELP2MAN) --name='$(PACKAGE_NAME)' --section=1 \
	--source='The Flex Project' --manual='Programming' \
	--output=$@ $(FLEX) \
	|| rm -f $@
