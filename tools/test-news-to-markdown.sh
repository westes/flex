#! /bin/sh
# Test tools/news-to-markdown.
set -u

srcdir=${srcdir:-$(dirname "$0")}
case $srcdir in
    /*) ;;
    *) srcdir=$(pwd)/$srcdir ;;
esac
news_to_markdown=$srcdir/news-to-markdown
failed=0

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT QUIT TERM

fail() {
    echo "FAIL: $*"
    failed=1
}

cat > "$work/in" <<'NEWS'
* Noteworthy changes in release 2.6.5 (2026-11-10) [stable]

** build

*** The first entry, with a continuation
    line and a mention of -Wextra and (#12).

*** The second entry has <config.h> and a rule with <<EOF>> in it.

    Thanks to Someone.

** scanner

*** A * in the middle, or ** at the end of a line **
NEWS

cat > "$work/want" <<'MD'
# Noteworthy changes in release 2.6.5 (2026-11-10) [stable]

## build

*   The first entry, with a continuation
    line and a mention of -Wextra and (#12).

*   The second entry has `<config.h>` and a rule with `<<EOF>>` in it.

    Thanks to Someone.

## scanner

*   A * in the middle, or ** at the end of a line **
MD

"$news_to_markdown" < "$work/in" > "$work/out" || fail "it exited with an error"
diff "$work/want" "$work/out" > "$work/diff" || fail "wrong output: $(cat "$work/diff")"

# Nothing but the markers and the backticks may change, in the real NEWS too.
NEWS=$srcdir/../NEWS
if [ -f "$NEWS" ]; then
    "$news_to_markdown" < "$NEWS" > "$work/all"
    [ "$(wc -l < "$NEWS")" = "$(wc -l < "$work/all")" ] || fail "NEWS changed length"
    sed -e 's/^\*\{1,3\} \{1,\}//' -e 's/`//g' "$NEWS" > "$work/a"
    sed -e 's/^\(#\{1,2\}\|\*\) \{1,\}//' -e 's/`//g' "$work/all" > "$work/b"
    cmp -s "$work/a" "$work/b" || fail "NEWS text changed"
fi

exit $failed
