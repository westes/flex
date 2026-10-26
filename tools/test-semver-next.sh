#! /bin/sh
# Test tools/semver-next on small repositories made for the purpose.
set -u

srcdir=${srcdir:-$(dirname "$0")}
case $srcdir in
    /*) ;;
    *) srcdir=$(pwd)/$srcdir ;;
esac
semver=$srcdir/semver-next
failed=0

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT QUIT TERM

fail() {
    echo "FAIL: $*"
    failed=1
}

newrepo() {
    repo=$work/$1
    mkdir "$repo"
    git -C "$repo" init -q
    git -C "$repo" config user.name test
    git -C "$repo" config user.email test@example.invalid
    git -C "$repo" config commit.gpgsign false
    git -C "$repo" config tag.gpgsign false
    git -C "$repo" commit -q --allow-empty -m 'chore: start'
}

commit() {
    git -C "$repo" commit -q --allow-empty -m "$1"
}

# expect NAME KEY=VALUE...: run semver-next in the repository and compare.
expect() {
    name=$1
    shift
    actual=$("$semver" -C "$repo" 2>/dev/null)
    for pair in "$@"; do
        if ! printf '%s\n' "$actual" | grep -qx "$pair"; then
            fail "$name: expected $pair, got: $(printf '%s' "$actual" | tr '\n' ' ')"
        fi
    done
}

# No release tag: exit status 2 and nothing on standard output.
newrepo notag
out=$("$semver" -C "$repo" 2>/dev/null)
status=$?
[ "$status" -eq 2 ] || fail "no tag: exit status $status, expected 2"
[ -z "$out" ] || fail "no tag: expected no output, got $out"

# Only commits that do not release.
newrepo none
git -C "$repo" tag v2.6.5
commit 'docs: update the manual'
commit 'build(deps): bump something'
commit 'Fix the old way'
expect none last=2.6.5 commits=3 bump=none next=

# Nothing since the tag.
newrepo empty
git -C "$repo" tag v2.6.5
expect empty last=2.6.5 commits=0 bump=none next=

# A fix.
newrepo fix
git -C "$repo" tag v2.6.5
commit 'docs: update the manual'
commit 'fix(skel): stop a crash'
expect fix bump=patch next=2.6.6

# perf and revert count as patch.
newrepo perf
git -C "$repo" tag v2.6.5
commit 'perf: faster scan'
expect perf bump=patch next=2.6.6
newrepo revert
git -C "$repo" tag v2.6.5
commit 'revert: undo a change'
expect revert bump=patch next=2.6.6

# A feature beats a fix, and resets the patch number.
newrepo feat
git -C "$repo" tag v2.6.5
commit 'fix: one'
commit 'feat(scanner): two'
commit 'fix: three'
expect feat bump=minor next=2.7.0

# A "!" is a major change, on any type, with or without a scope.
newrepo bang
git -C "$repo" tag v2.6.5
commit 'feat: new'
commit 'refactor(scanner)!: change the interface'
expect bang bump=major next=3.0.0

# So are the footers, written either way.
newrepo footer1
git -C "$repo" tag v2.6.5
git -C "$repo" commit -q --allow-empty -m 'fix: x' -m 'BREAKING-CHANGE: the output changed'
expect footer1 bump=major next=3.0.0
newrepo footer2
git -C "$repo" tag v2.6.5
git -C "$repo" commit -q --allow-empty -m 'fix: x' -m 'BREAKING CHANGE: the output changed'
expect footer2 bump=major next=3.0.0

# The words in the middle of a line are not a footer.
newrepo notfooter
git -C "$repo" tag v2.6.5
git -C "$repo" commit -q --allow-empty -m 'fix: x' -m 'This is not a BREAKING CHANGE: just words'
expect notfooter bump=patch next=2.6.6

# Only vX.Y.Z tags count: release candidates and old-style tags do not,
# and the highest tag wins by number, not by text.
newrepo tags
git -C "$repo" tag flex-2-5-5b
git -C "$repo" tag v2.9.0
git -C "$repo" tag v2.10.0
commit 'fix: after the release'
git -C "$repo" tag v2.11.0-rc1
commit 'feat: after the candidate'
expect tags last=2.10.0 commits=2 bump=minor next=2.11.0

# Only tags reachable from the commit count.
newrepo reach
git -C "$repo" tag v2.6.5
git -C "$repo" checkout -q -b other
commit 'feat: on another branch'
git -C "$repo" tag v3.0.0
git -C "$repo" checkout -q -
commit 'fix: on this branch'
expect reach last=2.6.5 commits=1 bump=patch next=2.6.6

# Merge commits are not counted, but the commits they bring in are.
newrepo merge
git -C "$repo" tag v2.6.5
main=$(git -C "$repo" branch --show-current)
git -C "$repo" checkout -q -b topic
commit 'feat: from the topic'
git -C "$repo" checkout -q "$main"
commit 'docs: on main'
git -C "$repo" merge -q --no-ff -m 'Merge branch topic' topic
expect merge commits=2 bump=minor next=2.7.0

# -t looks at an earlier point in the history.
newrepo upto
git -C "$repo" tag v2.6.5
commit 'fix: early'
git -C "$repo" tag early
commit 'feat: late'
actual=$("$semver" -C "$repo" -t early 2>/dev/null)
printf '%s\n' "$actual" | grep -qx 'next=2.6.6' || fail "-t: got: $actual"

exit $failed
