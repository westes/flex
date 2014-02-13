help2man = @HELP2MAN@

info_TEXINFOS =	flex.texi
dist_man_MANS = flex.1
dist_doc_DATA= flex.pdf

CLEANFILES = \
	flex.aux \
	flex.cp \
	flex.cps \
	flex.fn \
	flex.fns \
	flex.hk \
	flex.hks \
	flex.ky \
	flex.log \
	flex.op \
	flex.ops \
	flex.pg \
	flex.toc \
	flex.tp \
	flex.tps \
	flex.vr \
	flex.vrs

$(dist_man_MANS): $(top_srcdir)/main.c
	for i in $(dist_man_MANS) ; do \
	$(help2man) --name='$(PACKAGE_NAME)' \
	--section=`echo $$i | sed -e 's/.*\.\([^.]*\)$$/\1/'` \
	 ../flex$(EXEEXT) > $$i || rm -f $$i ; \
	done
