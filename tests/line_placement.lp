#!/bin/sh

# Test where flex puts the #line directives in a generated scanner.
#
# The code flex adds around a rule's action must be attributed to the
# generated file, not to the scanner file, so every action is followed
# by a directive that switches back, and the directive comes before the
# code that ends the rule's case (YY_BREAK).  The default rule has no
# line in the scanner file, so it gets no directive of its own, and a
# rule whose action is '|' has no action text, so a directive for it
# would only mislabel the next rule's case label.  Nothing is
# attributed to a line past the end of the scanner file.
#
# These are properties of the generated text, which a running scanner
# cannot show, so the test reads the generated file.

FLEX=$1
case $FLEX in
    /*) ;;
    *) FLEX=$(pwd)/$FLEX ;;
esac

work=$(mktemp -d "${TMPDIR:-/tmp}/lp.XXXXXX") || exit 99
trap 'rm -rf "$work"' EXIT HUP INT QUIT TERM

failures=0

fail () {
    echo "FAIL: $*" >&2
    failures=$((failures + 1))
}

cat > "$work/lp.l" <<'EOT'
%option noyywrap nounput noinput
%%
a	{ }
b	;
c	{
		/* a multi-line action */
	}
d	|
e	{ }
EOT

cd "$work" || exit 99
"$FLEX" -o lp.c lp.l || { echo "flex failed" >&2; exit 99; }

# Number the lines of the generated file by what they are attributed
# to, the way a compiler would, and report problems.  Lines inside the
# scanner file are those after a directive naming lp.l.  Only the rules
# are checked for directives in a row: an empty definitions section
# leaves one before the rules, which misattributes nothing.
awk -v nlines="$(wc -l < lp.l)" '
    /^[ \t]*case 1:/ { in_rules = 1 }
    /^#line [0-9]+ "/ {
        if (in_rules && prev_directive) {
            printf "two directives in a row at lp.c:%d\n", NR
        }
        prev_directive = 1
        in_l = ($3 == "\"lp.l\"")
        n = $2
        next
    }
    {
        prev_directive = 0
        if (in_l) {
            if (n > nlines)
                printf "lp.c:%d is attributed to lp.l:%d, past its end\n", NR, n
            if ($0 ~ /^[ \t]*(YY_BREAK|case )/)
                printf "lp.c:%d (%s) is attributed to lp.l:%d\n", NR, $0, n
        }
        n++
    }
' lp.c > problems.txt

if [ -s problems.txt ]; then
    fail "generated line directives are misplaced"
    sed 's/^/    /' problems.txt >&2
fi

if [ "$failures" -ne 0 ]; then
    exit 1
fi
echo "line directive placement test passed"
