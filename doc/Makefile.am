help2man = @HELP2MAN@

info_TEXINFOS =	flex.texi
man_MANS = flex.1

MAINTAINERCLEANFILES = \
	flex.1

CLEANFILES = \
	flex.hks \
	flex.ops

EXTRA_DIST = \
	$(man_MANS)

$(man_MANS): $(top_srcdir)/main.c
	for i in $(man_MANS) ; do \
	$(help2man) --name='fast lexical analyzer generator' --section=`echo $$i|rev|cut -f 1 -d .|rev` ../flex > $$i ; \
	done
