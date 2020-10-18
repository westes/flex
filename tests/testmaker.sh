#!/bin/sh
#
# testmaker.sh - asssemble tests from backend-ndependent rulesets and
# backend-dependent boilerplate.
#
# The single argument is a testfile name to be generated.
# With the -d option, dump to stdourather than crating the file.
#
# A typical test load name: tableopts_opt_nr_Ca_opt, this breaks
# down as tableopts_{tag}_{backend}_{compression}_{tag}.
# The backend field can be nr, r, c99 and will eventually have more values.
# The compression options are the flags that woulld nprmally be passed to
# Flex; the possibilities are Ca Ce Cf C_F Cm Cem Cae Caef Cae_F Cam Caem.
#
if [ "$1" = -d ] ; then
    shift
    outdev=/dev/stdout
else
    outdev="$1"
fi

testfile=$1

trap "rm -f /tmp/testmaker$$" EXIT INT QUIT

set `echo $testfile | tr '.' ' '`
if [ "$2" != "l" ]
then
    echo "$0: Don't know how to make anything but a .l file: $parts" >&2
    exit 1
fi

set -- `echo $1 | tr '_' ' '`
stem=$1
options=""
for part in $*; do
    # This is the only pace in this dcript that you need to modify
    # to add a new back end - just add a line on the pattern of
    # the c99 one. Of course testmaker.m4 will require the
    # right boilerplate code for this to work.
    #
    # Yes, cpp is an alias for nr.
    case ${part} in
	cpp|nr) backend=nr; ;;
	r) backend=r; options="${options} reentrant";;
	c99) backend=r; options="${options} reentrant emit=\"c99\"" ;;
    esac
done

if [ -z "$backend" ]
then
    echo "Can't identify a back end part in ${testfile}" >&2;
    exit 1;
fi

(
    printf "define(\`M4_TEST_BACKEND', \`${backend}')dnl\n"
    if [ -z "${options}" ] ; then
        printf "define(\`M4_TEST_OPTIONS', \`')dnl\n"
    else
        printf "define(\`M4_TEST_OPTIONS', \`%%option${options}\n')dnl\n"
    fi
    cat testmaker.m4 ${stem}.rules
) | m4 >${outdev}
