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

EXTRA_DIST = \
	README

dist_noinst_SCRIPTS = \
	create-test

DIST_SUBDIRS = \
	test-concatenated-options \
	test-c++-yywrap \
	test-extended \
	test-ccl \
	test-quotes \
	test-rescan-r \
	test-rescan-nr \
	test-basic-nr \
	test-basic-r \
	test-bison-yylloc \
	test-bison-yylval \
	test-bison-nr \
	test-multiple-scanners-nr \
	test-multiple-scanners-r \
	test-header-nr \
	test-header-r \
	test-reject \
	test-c++-multiple-scanners \
	test-c++-basic \
	test-posixly-correct \
	test-posix \
	test-mem-r \
	test-mem-nr \
	test-debug-nr \
	test-debug-r \
	test-lineno-r \
	test-lineno-nr \
	test-linedir-r \
	TEMPLATE \
	test-top \
	test-array-nr \
	test-array-r \
	test-c-cpp-nr \
	test-c-cpp-r \
	test-include-by-buffer \
	test-include-by-push \
	test-include-by-reentrant \
	test-prefix-nr \
	test-prefix-r \
	test-pthread \
	test-string-nr \
	test-string-r \
	test-yyextra \
	test-alloc-extra \
	test-noansi-nr \
	test-noansi-r \
	test-table-opts

SUBDIRS = \
	test-concatenated-options \
	test-c++-yywrap \
	test-extended \
	test-ccl \
	test-quotes \
	test-rescan-r \
	test-rescan-nr \
	test-basic-nr \
	test-basic-r \
	test-bison-yylloc \
	test-bison-yylval \
	test-bison-nr \
	test-multiple-scanners-nr \
	test-multiple-scanners-r \
	test-header-nr \
	test-header-r \
	test-reject \
	test-c++-multiple-scanners \
	test-c++-basic \
	test-posixly-correct \
	test-posix \
	test-mem-r \
	test-mem-nr \
	test-debug-nr \
	test-debug-r \
	test-lineno-r \
	test-lineno-nr \
	test-linedir-r \
	test-array-nr \
	test-array-r \
	test-c-cpp-nr \
	test-c-cpp-r \
	test-include-by-buffer \
	test-include-by-push \
	test-include-by-reentrant \
	test-prefix-nr \
	test-prefix-r \
	test-pthread \
	test-string-nr \
	test-string-r \
	test-yyextra \
	test-alloc-extra \
	test-noansi-nr \
	test-noansi-r \
	test-top \
	test-table-opts

# clean up before running the test suite so we dont test old builds of test code

check-local: clean
	NOK=;\
	NFAIL=;\
	for dir in $(SUBDIRS) ; do \
		echo Executing test "$$dir" ; \
		( cd "$$dir" && $(MAKE) test  > OUTPUT 2>&1 ) ; \
		case $$? in \
			0 )  echo Test "$$dir" succeeded.; \
				NOK=0$$NOK;\
				;; \
			* )  echo Test "$$dir" FAILED. See "$$dir"/OUTPUT for details. ; \
				NFAIL=0$$NFAIL; \
				;; \
		esac; \
	done ; \
	echo Results: ; \
	echo Tests succeeded: `echo @ECHO_N@ "$$NOK@ECHO_C@"|wc -c`; \
	echo Tests FAILED: `echo @ECHO_N@ "$$NFAIL@ECHO_C@"|wc -c` ; \
	test "$$NFAIL" = ""
