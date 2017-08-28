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
	autogen.sh

SUBDIRS = \
	src \
	doc \
	examples \
	po \
	tests \
	tools

# Convenience targets to build libfl only
# These are actually wrappers around automake- and libtool-generated targets

libfl:
	cd src && $(MAKE) $(AM_MAKEFLAGS) libfl.la libfl.pc

install-libfl:
	cd src && \
	$(MAKE) $(AM_MAKEFLAGS) lib_LTLIBRARIES=libfl.la \
	pkgconfig_DATA=libfl.pc install-libLTLIBRARIES install-pkgconfigDATA

uninstall-libfl:
	cd src && \
	$(MAKE) $(AM_MAKEFLAGS) \
	lib_LTLIBRARIES=libfl.la pkgconfig_DATA=libfl.pc \
	uninstall-libLTLIBRARIES uninstall-pkgconfigDATA

# libfl.pc is cleaned via 'distclean' target
clean-libfl:
	cd src && \
	$(MAKE) $(AM_MAKEFLAGS) lib_LTLIBRARIES=libfl.la clean-libLTLIBRARIES \
	clean-libtool

# Create the ChangeLog, but only if we're inside a git working directory

ChangeLog: $(srcdir)/tools/git2cl
	if [ -d $(srcdir)/.git ] ; then \
		$(srcdir)/tools/git2cl > $@ \
	; fi

indent:
	cd src && $(MAKE) $(AM_MAKEFLAGS) indent

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
		$(LN_S) -f flex$(EXEEXT) flex++$(EXEEXT)

.PHONY: libfl install-libfl uninstall-libfl clean-libfl \
	ChangeLog indent
