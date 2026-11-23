#!/bin/sh

# Two things that programs written for earlier flex releases do, and that
# must keep compiling:
#
# 1. A program that supplies its own yyscan_t, and so declares
#    struct yyguts_t itself before the scanner, as doxygen does, while
#    using a prefix. Flex must leave the name yyguts_t alone there.
# 2. A reentrant C scanner that reaches into its state and sets the
#    member yy_flex_debug_r, as csound does.

FLEX=$1
CC_ARG=$2
CXX_ARG=$3
case $FLEX in
    /*) ;;
    *) FLEX=$(pwd)/$FLEX ;;
esac

work=$(mktemp -d "${TMPDIR:-/tmp}/nc.XXXXXX") || exit 99
trap 'rm -rf "$work"' EXIT HUP INT QUIT TERM

cat > "$work/own_type.l" <<'EOT'
%top{
struct yyguts_t;
typedef yyguts_t *yyscan_t;
#define YY_TYPEDEF_YY_SCANNER_T
}
%option reentrant prefix="ownYY" extra-type="int *" noyywrap nounput noinput
%{
#define YY_FATAL_ERROR(msg) do { if (static_cast<yyguts_t *>(yyscanner)->yyextra_r) {} } while (0)
%}
%%
.|\n	;
%%
EOT

cat > "$work/debug_member.l" <<'EOT'
%option reentrant prefix="dbgYY" noyywrap nounput noinput debug
%{
#define YY_USER_INIT yyg->yy_flex_debug_r = 1;
%}
%%
.|\n	;
%%
EOT

if [ -n "$CXX_ARG" ]; then
    "$FLEX" -o "$work/own_type.cc" "$work/own_type.l" || { echo "FAIL: flex failed on own_type.l" >&2; exit 1; }
    if ! $CXX_ARG -c -o "$work/own_type.o" "$work/own_type.cc" 2>"$work/cc.txt"; then
        echo "FAIL: a program that declares struct yyguts_t before the scanner did not compile" >&2
        sed 's/^/    /' "$work/cc.txt" >&2
        exit 1
    fi
fi

"$FLEX" -o "$work/debug_member.c" "$work/debug_member.l" || { echo "FAIL: flex failed on debug_member.l" >&2; exit 1; }
if ! $CC_ARG -c -o "$work/debug_member.o" "$work/debug_member.c" 2>"$work/cc.txt"; then
    echo "FAIL: a scanner that sets yy_flex_debug_r did not compile" >&2
    sed 's/^/    /' "$work/cc.txt" >&2
    exit 1
fi

echo "TEST RETURNING OK."
exit 0
