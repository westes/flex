#! /bin/sh
# Test tools/make-release on small git repositories made for the purpose.
set -u

srcdir=${srcdir:-$(dirname "$0")}
case $srcdir in
    /*) ;;
    *) srcdir=$(pwd)/$srcdir ;;
esac
make_release=$srcdir/make-release
failed=0

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT QUIT TERM

GIT_AUTHOR_NAME=test
GIT_AUTHOR_EMAIL=test@example.com
GIT_COMMITTER_NAME=test
GIT_COMMITTER_EMAIL=test@example.com
export GIT_AUTHOR_NAME GIT_AUTHOR_EMAIL GIT_COMMITTER_NAME GIT_COMMITTER_EMAIL

placeholder='* Noteworthy changes in release  ?.? (????-??-??) [?]'

fail() {
    echo "FAIL: $*"
    failed=1
}

# repo NAME HEADER: a repository whose configure.ac says 2.6.5-rc1 and
# whose NEWS starts with HEADER, one entry, and an older release.
repo() {
    mkdir "$work/$1"
    (
        cd "$work/$1" || exit 1
        git init -q .
        printf 'AC_INIT([the fast lexical analyser generator],[2.6.5-rc1],[flex-help@lists.sourceforge.net],[flex])\n' >configure.ac
        {
            printf 'flex NEWS\n\n%s\n\n** scanner\n\n*** Something changed.\n\n' "$2"
            printf '* Noteworthy changes in release 2.6.4 (2017-05-06) [stable]\n\n*** Old entry.\n'
        } >NEWS
        echo other >other
        git add . && git commit -q -m start
    )
}

head_of() {
    git -C "$work/$1" rev-parse HEAD
}

# good NAME ARGS...: make-release should succeed.
good() {
    name=$1
    shift
    if ! "$make_release" -C "$work/$name" "$@" >"$work/$name.out" 2>"$work/$name.err"; then
        fail "$name should succeed: $(cat "$work/$name.err")"
    fi
}

# bad NAME ARGS...: make-release should fail and leave HEAD where it was.
bad() {
    name=$1
    shift
    before=$(head_of "$name")
    dirt=$(git -C "$work/$name" status --porcelain --untracked-files=no)
    if "$make_release" -C "$work/$name" "$@" >/dev/null 2>&1; then
        fail "$name should be rejected"
    fi
    [ "$(head_of "$name")" = "$before" ] || fail "$name moved HEAD"
    # Whatever the test itself left uncommitted is expected to stay.
    [ "$(git -C "$work/$name" status --porcelain --untracked-files=no)" = "$dirt" ] ||
        fail "$name changed the work tree"
}

# --- A final release.

repo final "$placeholder"
good final -d 2026-10-26 2.6.5

[ "$(git -C "$work/final" rev-list --count HEAD)" = 3 ] || fail "final: expected two new commits"
[ "$(git -C "$work/final" log -1 --format=%s HEAD)" = 'release: start development after v2.6.5' ] ||
    fail "final: wrong subject on the second commit"
[ "$(git -C "$work/final" log -1 --format=%s HEAD~1)" = 'release: v2.6.5' ] ||
    fail "final: wrong subject on the first commit"
[ "$(git -C "$work/final" rev-parse HEAD~1 HEAD)" = "$(cat "$work/final.out")" ] ||
    fail "final: printed ids do not match the commits, oldest first"
[ "$(git -C "$work/final" diff --name-only HEAD~2 HEAD~1 | tr '\n' ' ')" = 'NEWS configure.ac ' ] ||
    fail "final: first commit should touch NEWS and configure.ac only"
[ "$(git -C "$work/final" diff --name-only HEAD~1 HEAD | tr '\n' ' ')" = 'NEWS ' ] ||
    fail "final: second commit should touch NEWS only"
grep -q '^AC_INIT(\[the fast lexical analyser generator\],\[2.6.5\],\[flex-help' "$work/final/configure.ac" ||
    fail "final: AC_INIT not updated"
expected=$(printf 'flex NEWS\n\n%s\n\n%s\n\n** scanner\n\n*** Something changed.\n\n%s\n\n*** Old entry.' \
    "$placeholder" '* Noteworthy changes in release 2.6.5 (2026-10-26) [stable]' \
    '* Noteworthy changes in release 2.6.4 (2017-05-06) [stable]')
[ "$(cat "$work/final/NEWS")" = "$expected" ] || fail "final: NEWS is not as expected"
(cd "$work/final" && git show HEAD~1:NEWS >"$work/final.release-news" && git show HEAD~1:configure.ac >"$work/final.release-ac") ||
    fail "final: could not read the release commit"
mkdir "$work/final-tree"
cp "$work/final.release-news" "$work/final-tree/NEWS"
cp "$work/final.release-ac" "$work/final-tree/configure.ac"
"$srcdir/check-release" -d "$work/final-tree" -t v2.6.5 ||
    fail "final: the release commit should pass check-release"
# The next release can follow once an entry is added.
if "$srcdir/check-release" -u -d "$work/final" 2>/dev/null; then
    fail "final: a placeholder with no entries should not be ready"
fi

# --- A release candidate.

repo rc "$placeholder"
good rc -r -d 2026-10-26 2.6.5-rc2
[ "$(git -C "$work/rc" rev-list --count HEAD)" = 2 ] || fail "rc: expected one new commit"
[ "$(git -C "$work/rc" log -1 --format=%s)" = 'release: v2.6.5-rc2' ] || fail "rc: wrong subject"
[ "$(wc -l <"$work/rc.out")" = 1 ] || fail "rc: expected one id"
grep -q '^\* Noteworthy changes in release 2.6.5-rc2 (2026-10-26) \[pre-release\]$' "$work/rc/NEWS" ||
    fail "rc: NEWS header is wrong"
grep -q 'release  ?\.?' "$work/rc/NEWS" && fail "rc: should not add a placeholder"
"$srcdir/check-release" -d "$work/rc" -t v2.6.5-rc2 || fail "rc: should pass check-release"

# --- Things it should refuse.

repo notplaceholder '* Noteworthy changes in release 2.6.5 (2026-10-26) [stable]'
bad notplaceholder -d 2026-10-26 2.6.6

repo noentries "$placeholder"
printf 'flex NEWS\n\n%s\n\n%s\n\n*** Old entry.\n' "$placeholder" \
    '* Noteworthy changes in release 2.6.4 (2017-05-06) [stable]' >"$work/noentries/NEWS"
git -C "$work/noentries" commit -q -am "no entries"
bad noentries -d 2026-10-26 2.6.5

repo dirty "$placeholder"
echo more >>"$work/dirty/other"
bad dirty -d 2026-10-26 2.6.5

repo args "$placeholder"
bad args -d 2026-10-26 2.6
bad args -d 2026-10-26 v2.6.5
bad args -d 2026-10-26 2.6.5-rc1
bad args -d 2026-10-26 -r 2.6.5
bad args -d soon 2.6.5
bad args -d 2026-13-01 2.6.5
bad args -d 2999-01-01 2.6.5

# A tree that fails partway is put back: configure.ac has no AC_INIT line
# to change, so the release does not match its own header.
repo noacinit "$placeholder"
printf 'dnl nothing here\n' >"$work/noacinit/configure.ac"
git -C "$work/noacinit" commit -q -am "no AC_INIT"
bad noacinit -d 2026-10-26 2.6.5

exit $failed
