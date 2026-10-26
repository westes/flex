#! /bin/sh
# Test tools/check-release on small source trees made for the purpose.
set -u

srcdir=${srcdir:-$(dirname "$0")}
case $srcdir in
    /*) ;;
    *) srcdir=$(pwd)/$srcdir ;;
esac
check=$srcdir/check-release
failed=0

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT QUIT TERM

# tree NAME VERSION HEADER: a tree whose configure.ac says VERSION and
# whose NEWS starts with HEADER, one entry, and an older release.
tree() {
    mkdir "$work/$1"
    printf 'AC_INIT([the fast lexical analyser generator],[%s],[flex-help@lists.sourceforge.net],[flex])\n' "$2" >"$work/$1/configure.ac"
    {
        printf 'flex NEWS\n\n%s\n\n** scanner\n\n*** Something changed.\n\n' "$3"
        printf '* Noteworthy changes in release 2.6.4 (2017-05-06) [stable]\n\n*** Old entry.\n'
    } >"$work/$1/NEWS"
}

good() {
    name=$1
    shift
    if ! "$check" -d "$work/$name" "$@" >/dev/null 2>&1; then
        echo "FAIL: $name should pass"
        failed=1
    fi
}

bad() {
    name=$1
    shift
    if "$check" -d "$work/$name" "$@" >/dev/null 2>&1; then
        echo "FAIL: $name should be rejected"
        failed=1
    fi
}

placeholder='* Noteworthy changes in release  ?.? (????-??-??) [?]'

tree ready 2.6.5 '* Noteworthy changes in release 2.6.5 (2026-10-26) [stable]'
good ready
good ready -t v2.6.5
bad ready -t v2.6.6
bad ready -u

tree rc 2.6.5-rc1 '* Noteworthy changes in release 2.6.5-rc1 (2026-10-26) [pre-release]'
good rc -t v2.6.5-rc1

tree open 2.6.4 "$placeholder"
good open -u
bad open

tree mismatch 2.6.4 '* Noteworthy changes in release 2.6.5 (2026-10-26) [stable]'
bad mismatch

tree future 2.6.5 '* Noteworthy changes in release 2.6.5 (2999-01-01) [stable]'
bad future

tree baddate 2.6.5 '* Noteworthy changes in release 2.6.5 (soon) [stable]'
bad baddate

tree twopart 2.6 '* Noteworthy changes in release 2.6 (2026-10-26) [stable]'
bad twopart

# A section with no entries.
mkdir "$work/empty"
printf 'AC_INIT([x],[2.6.5],[y],[flex])\n' >"$work/empty/configure.ac"
printf 'flex NEWS\n\n* Noteworthy changes in release 2.6.5 (2026-10-26) [stable]\n\n* Noteworthy changes in release 2.6.4 (2017-05-06) [stable]\n\n*** Old entry.\n' >"$work/empty/NEWS"
bad empty
mkdir "$work/emptyopen"
printf 'AC_INIT([x],[2.6.4],[y],[flex])\n' >"$work/emptyopen/configure.ac"
printf 'flex NEWS\n\n%s\n\n* Noteworthy changes in release 2.6.4 (2017-05-06) [stable]\n\n*** Old entry.\n' "$placeholder" >"$work/emptyopen/NEWS"
bad emptyopen -u

# No release section at all.
mkdir "$work/none"
printf 'AC_INIT([x],[2.6.5],[y],[flex])\n' >"$work/none/configure.ac"
printf 'flex NEWS\n' >"$work/none/NEWS"
bad none

exit $failed
