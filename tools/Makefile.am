# These test the maintainer scripts in this directory, not the build of
# flex, and some need git, so they are not part of "make check". Run them
# with "make check-tools" in a git checkout.
TOOL_TESTS = \
	test-check-commit-msg.sh \
	test-check-release.sh \
	test-git2cl.sh \
	test-make-announcement.sh \
	test-make-release.sh \
	test-news-to-markdown.sh \
	test-semver-next.sh

EXTRA_DIST = \
	bootstrap-flex \
	check-commit-msg \
	check-release \
	git2cl \
	make-announcement \
	make-release \
	news-to-markdown \
	semver-next \
	$(TOOL_TESTS)

check-tools:
	@failed=0; for t in $(TOOL_TESTS); do \
	  if srcdir=$(srcdir) $(SHELL) $(srcdir)/$$t; then echo "PASS: $$t"; \
	  else echo "FAIL: $$t"; failed=1; fi; \
	done; exit $$failed

.PHONY: check-tools
