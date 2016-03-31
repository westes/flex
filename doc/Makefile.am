help2man = @HELP2MAN@

info_TEXINFOS =	flex.texi
dist_man_MANS = flex.1
DISTCLEANFILES = flex.1

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

flex.1: ../src/flex$(EXEEXT)
	$(help2man) --name='$(PACKAGE_NAME)' --section=1 \
	--source='The Flex Project' --manual='Programming' \
	--output=$@ $< \
	|| rm -f $@
