#! /bin/sh
# Generate test for the %yydecl option of flex on a specified back end
backend="$1"
FLEX="$2"

cat <<EOF_OUTER
#! /bin/sh
# Test %yydecl option of flex on ${backend} backend.
set -eu

trap 'rm /tmp/td\$\$' EXIT HUP INT QUIT TERM

teeout=/dev/null

DEBUG=0

while getopts d OPTION ; do
    case \$OPTION in
        d) DEBUG=1 ;;
        *) echo "Usage: \${0} [-d]"
           exit 1
           ;;
    esac
done

shift \$((OPTIND-1))

if [ "\${DEBUG}" = "1" ] ; then
    teeout=/dev/stderr
fi

cat >/tmp/td\$\$ <<EOF
%option yydecl="int foobar(void)" emit="${backend}"
%%
%%
EOF
if "${FLEX}" -t /tmp/td\$\$  | tee \${teeout} | grep "int foobar(void)" >/dev/null
then
     echo "%yydecl test on ${backend} SUCCEEDED"
     exit 0
else
     echo "%yydecl test on ${backend} FAILED." >&2;
     exit 1
fi
EOF_OUTER
