#!/bin/sh

# Test that tools/git2cl writes valid UTF-8 and prints no warnings when
# an author's name has accented characters.  The script used to print
# names with characters up to U+00FF as single Latin-1 bytes, and those
# above it with a "Wide character" warning, so the ChangeLog mixed two
# encodings.
#
# Skipped (exit status 77) if perl is not available.

srcdir=${srcdir:-$(dirname "$0")}
GIT2CL=$srcdir/git2cl

command -v perl >/dev/null 2>&1 || exit 77

work=$(mktemp -d "${TMPDIR:-/tmp}/g2c.XXXXXX") || exit 99
trap 'rm -rf "$work"' EXIT HUP INT QUIT TERM

# A UTF-8 locale is what exposes the bug; use one if the system has it.
for loc in C.UTF-8 en_US.UTF-8; do
    if [ "$(LC_ALL=$loc locale charmap 2>/dev/null)" = UTF-8 ]; then
        LC_ALL=$loc; export LC_ALL
        break
    fi
done

# The log is in the form git log --pretty --numstat --summary prints.
# u-umlaut is U+00FC, n-acute is U+0144.
umlaut=$(printf 'Gr\303\274ninger')
acute=$(printf 'Pluci\305\204ski')
{
    for name in "$umlaut" "$acute"; do
        printf 'commit 0000000000000000000000000000000000000000\n'
        printf 'Author: %s <a@example.org>\n' "$name"
        printf 'Date:   Mon Nov 9 00:11:25 2026 -0500\n\n'
        printf '    fix something\n\n'
        printf '1\t1\tfile.c\n\n'
    done
} > "$work/log.txt"

perl "$GIT2CL" < "$work/log.txt" > "$work/out.txt" 2> "$work/err.txt"
status=$?

failures=0
fail () {
    echo "FAIL: $*" >&2
    failures=$((failures + 1))
}

[ "$status" -eq 0 ] || fail "git2cl exited with status $status"
[ ! -s "$work/err.txt" ] || fail "git2cl printed: $(cat "$work/err.txt")"
grep -F "$umlaut" "$work/out.txt" >/dev/null || fail "the u-umlaut name is not in UTF-8"
grep -F "$acute" "$work/out.txt" >/dev/null || fail "the n-acute name is not in UTF-8"

[ "$failures" -eq 0 ] || exit 1
echo "git2cl wrote valid UTF-8"
