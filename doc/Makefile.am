help2man = @HELP2MAN@

info_TEXINFOS =	flex.texi
dist_man_MANS = flex.1

CLEANFILES = \
	flex.hks \
	flex.ops

$(dist_man_MANS): $(top_srcdir)/main.c
	for i in $(dist_man_MANS) ; do \
	$(help2man) --name='fast lexical analyzer generator' --section=`echo $$i|rev|cut -f 1 -d .|rev` ../flex > $$i ; \
	done
