#! /bin/sh
# Test tools/check-commit-msg.
set -u

srcdir=${srcdir:-$(dirname "$0")}
check=$srcdir/check-commit-msg
failed=0

ok() {
    if ! printf '%s\n' "$1" | "$check" >/dev/null 2>&1; then
        echo "FAIL: should be accepted: $1"
        failed=1
    fi
}

bad() {
    if printf '%s\n' "$1" | "$check" >/dev/null 2>&1; then
        echo "FAIL: should be rejected: $1"
        failed=1
    fi
}

ok 'fix: stop writing past the buffer'
ok 'fix(skel): end yymore text at a buffer switch with %array'
ok 'feat(scanner)!: drop the old option'
ok 'feat!: drop the old option'
ok 'docs: update the manual'
ok 'build(deps): bump the github-actions group'
ok 'chore: tidy'
ok 'release: v2.6.6'
ok 'test(skel): add a test'
ok 'revert: undo the earlier change'
ok 'Merge pull request #753 from someone/branch'
ok 'fixup! fix: stop writing past the buffer'
ok 'squash! fix: stop writing past the buffer'

bad 'Fix the buffer overflow'
bad 'fix the buffer overflow'
bad 'fix:no space after the colon'
bad 'fix: '
bad 'fix(): empty scope'
bad 'doc: the old spelling'
bad 'tests: the old spelling'
bad 'wibble: not a type'
bad 'Fix: capitalised type'
bad ''
bad '# only a comment'

# Comment lines and blank lines before the subject are skipped, as in the
# file git passes to a commit-msg hook.
msgfile=$(mktemp)
trap 'rm -f "$msgfile"' EXIT HUP INT QUIT TERM
printf '\n# a comment\nfix(skel): a subject\n\nbody\n# another comment\n' >"$msgfile"
if ! "$check" "$msgfile" >/dev/null 2>&1; then
    echo "FAIL: message file with comments should be accepted"
    failed=1
fi

# A rejected subject is shown in the message.
if ! printf 'oops\n' | "$check" 2>&1 | grep -q 'oops'; then
    echo "FAIL: the rejected subject should be shown"
    failed=1
fi

exit $failed
