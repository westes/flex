#! /bin/sh
# Generate test for the %yydecl option of flex on a specified back end
backend="$1"

cat <<EOF_OUTER
#! /bin/sh
# Test %yydecl option of flex on ${backend} backend.
trap 'rm /tmp/td\$\$' EXIT HUP INT QUIT TERM

teeout=/dev/null
if [ "\$1" = "-d" ]
then
    shift
    teeout=/dev/stderr
fi

cat >/tmp/td\$\$ <<EOF
%option yydecl="int foobar(void)" emit="${backend}"
%%
%%
EOF
if ../src/flex -t /tmp/td\$\$  | tee \${teeout} | grep "int foobar(void)" >/dev/null
then
     echo "%yydecl test on ${backend} SUCCEEDED"
     exit 0
else
     echo "%yydecl test on ${backend} FAILED." >&2;
     exit 1
fi
EOF_OUTER
