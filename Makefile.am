# This file is part of flex.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:

# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.

# Neither the name of the University nor the names of its contributors
# may be used to endorse or promote products derived from this software
# without specific prior written permission.

# THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE.

# Notes on building:

# Possible values for DEFS:
#
# By default, flex generates 8-bit scanners when using table compression,
# and 7-bit scanners when using uncompressed tables (-f or -F options).
# For flex to always generate 8-bit scanners, add "-DDEFAULT_CSIZE=256"
# to DEFS.

ACLOCAL_AMFLAGS = -I m4

dist_doc_DATA = \
	AUTHORS \
	COPYING \
	NEWS \
	ONEWS \
	README.md

EXTRA_DIST = \
	.indent.pro \
	INSTALL.md \
	CONTRIBUTING.md

dist_noinst_SCRIPTS = \
	autogen.sh \
	po/update_linguas.sh

SUBDIRS = \
	. \
	po \
	tests

MOSTLYCLEANFILES =

CLEANFILES =

indentfiles =

include doc/local.mk
include examples/local.mk
include src/local.mk
include tools/local.mk

# Convenience targets to build libfl only
# These are actually wrappers around automake- and libtool-generated targets

libfl: libfl.la src/libfl.pc

install-libfl:
	$(MAKE) $(AM_MAKEFLAGS) lib_LTLIBRARIES=libfl.la \
	pkgconfig_DATA=src/libfl.pc install-libLTLIBRARIES install-pkgconfigDATA

uninstall-libfl:
	$(MAKE) $(AM_MAKEFLAGS) \
	lib_LTLIBRARIES=libfl.la pkgconfig_DATA=src/libfl.pc \
	uninstall-libLTLIBRARIES uninstall-pkgconfigDATA

# libfl.pc is cleaned via 'distclean' target
clean-libfl:
	$(MAKE) $(AM_MAKEFLAGS) lib_LTLIBRARIES=libfl.la clean-libLTLIBRARIES \
	clean-libtool

# Create the ChangeLog, but only if we're inside a git working directory

ChangeLog: $(srcdir)/tools/git2cl
	if [ -d $(srcdir)/.git ] ; then \
		$(srcdir)/tools/git2cl > $@ \
	; fi

# Run GNU indent on sources. Don't run this unless all the sources compile cleanly.
#
# Whole idea:
#   1. Check for .indent.pro, otherwise indent will use unknown
#      settings, or worse, the GNU defaults.)
#   2. Check that this is GNU indent.
#   3. Make sure to process only the NON-generated .c and .h files.
#   4. Run indent twice per file. The first time is a test.
#      Otherwise, indent overwrites your file even if it fails!

indent: .indent.pro
	for f in $(indentfiles); do \
		echo indenting $$f; \
		INDENT_PROFILE=.indent.pro $(INDENT) <$$f >/dev/null && \
		INDENT_PROFILE=.indent.pro $(INDENT) $$f || \
		echo $$f FAILED to indent; \
	done;

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
		$(LN_S) -f flex$(EXEEXT) flex++$(EXEEXT)

lint:
	shellcheck -f gcc tests/*.sh src/*.sh *.sh

.PHONY: libfl install-libfl uninstall-libfl clean-libfl \
	ChangeLog indent lint
