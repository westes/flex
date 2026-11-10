#! /bin/sh
# Test tools/make-announcement on a small git repository made for the purpose.
set -u

srcdir=${srcdir:-$(dirname "$0")}
case $srcdir in
    /*) ;;
    *) srcdir=$(pwd)/$srcdir ;;
esac
make_announcement=$srcdir/make-announcement
failed=0

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT QUIT TERM

fail() {
    echo "FAIL: $*"
    failed=1
}

# commit AUTHOR SUBJECT: an empty commit by AUTHOR.
commit() {
    GIT_AUTHOR_NAME=$1 GIT_AUTHOR_EMAIL=a@example.com \
    GIT_COMMITTER_NAME=$1 GIT_COMMITTER_EMAIL=a@example.com \
        git -C "$work/repo" commit -q --allow-empty -m "$2"
}

mkdir "$work/repo" "$work/files"
git -C "$work/repo" init -q .
commit Old 'fix: an old fix'
git -C "$work/repo" tag v2.6.3
commit Old 'fix: another old fix'
git -C "$work/repo" tag v2.6.4
commit Alice 'fix: one'
commit Alice 'fix: two'
commit Bob 'feat: three'
commit 'dependabot[bot]' 'build(deps): bump a thing'
commit 'github-actions[bot]' 'release: v2.6.5-rc1'
git -C "$work/repo" tag v2.6.5-rc1
commit Alice 'fix: four'
commit 'github-actions[bot]' 'release: v2.6.5-rc2'

for e in tar.gz tar.lz; do
    echo data > "$work/files/flex-2.6.5-rc2.$e"
    echo sig > "$work/files/flex-2.6.5-rc2.$e.sig"
done
(cd "$work/files" && sha256sum flex-2.6.5-rc2.tar.gz flex-2.6.5-rc2.tar.lz > SHA256SUMS)
echo sig > "$work/files/SHA256SUMS.sig"
printf '* Noteworthy changes in release 2.6.5-rc2 (2026-11-09) [pre-release]\n\n*** A change.\n' > "$work/files/notes.md"
printf 'pub   ed25519 2026-11-09 [C]\n      AAAA BBBB\nuid   Flex Releases\n' > "$work/key.txt"
printf 'autoconf 2.71\n' > "$work/tools.txt"

run() {
    "$make_announcement" -C "$work/repo" -c HEAD -d 2026-11-09 \
        -f "$work/files" -k "$work/key.txt" -t "$work/tools.txt" "$@"
}

if ! run 2.6.5-rc2 > "$work/out" 2> "$work/err"; then
    fail "should succeed: $(cat "$work/err")"
else
    out=$work/out
    [ "$(head -n 1 "$out")" = 'Subject: flex-2.6.5-rc2 released [pre-release]' ] ||
        fail "subject: $(head -n 1 "$out")"
    # Counted since v2.6.4, the final release, not v2.6.5-rc1: fix one, two,
    # three and four; not the bot's commits.
    grep -q '^There have been 4 commits by 2 people in 1 week since 2.6.4\.$' "$out" ||
        fail "counts: $(grep '^There have been' "$out")"
    grep -q 'Alice' "$out" || fail "Alice is not thanked"
    grep -q 'Bob' "$out" || fail "Bob is not thanked"
    if grep -q 'bot\]' "$out"; then fail "a bot is thanked"; fi
    if grep -q 'Old' "$out"; then fail "someone from before 2.6.4 is thanked"; fi
    grep -q 'compare/v2.6.4\.\.\.v2.6.5-rc2$' "$out" || fail "compare link"
    grep -q 'with commit '"$(git -C "$work/repo" rev-parse HEAD)"' tagged as v2.6.5-rc2\.' "$out" ||
        fail "commit hash"
    grep -q '^\*\*\* A change\.$' "$out" || fail "NEWS section missing"
    grep -q 'releases/download/v2.6.5-rc2/flex-2.6.5-rc2.tar.gz\.sig$' "$out" ||
        fail "signature link"
    grep -q '^  autoconf 2.71$' "$out" || fail "tools"
fi

# A final release: no rc wording, and the previous final is still v2.6.4.
for e in tar.gz tar.lz; do
    cp "$work/files/flex-2.6.5-rc2.$e" "$work/files/flex-2.6.5.$e"
    cp "$work/files/flex-2.6.5-rc2.$e.sig" "$work/files/flex-2.6.5.$e.sig"
done
if run 2.6.5 > "$work/out2" 2> "$work/err"; then
    [ "$(head -n 1 "$work/out2")" = 'Subject: flex-2.6.5 released [stable]' ] ||
        fail "final subject: $(head -n 1 "$work/out2")"
    grep -q 'since 2.6.4\.$' "$work/out2" || fail "final comparison point"
else
    fail "final should succeed: $(cat "$work/err")"
fi

# Missing or empty input is refused.
: > "$work/files/notes.md"
if run 2.6.5-rc2 > /dev/null 2>&1; then fail "empty notes accepted"; fi
if run 2.6.6 > /dev/null 2>&1; then fail "version without files accepted"; fi
if run banana > /dev/null 2>&1; then fail "bad version accepted"; fi

exit $failed
