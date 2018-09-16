#!/bin/sh
set -eu
# IFS=$'\n\t'
IFS='
''	'

# This script is present to generate the automake _SOURCES variables
# for the tableopts_* tests. It also generates the linking rules for
# each test since automake isn't able to handle the pattern rules that
# would be natural to use. Output is written to standard output for
# inclusion in a Makefile.am, typically by redirecting the output and
# then an automake include directive.

TABLEOPTS_TESTS=""
tableopts_tables=""

for kind in opt ser ver ; do
    for threading in nr r ; do
        for opt in -Ca -Ce -Cf -CF -Cm -Cem -Cae -Caef -CaeF -Cam -Caem ; do
            bare_opt=${opt#-}
            # The filenames must work on case-insensitive filesystems.
            bare_opt=`echo ${bare_opt}| sed 's/F$/_F/'`

            testname=tableopts_${kind}_${threading}-${bare_opt}.${kind}
            if [ "${TABLEOPTS_TESTS}" = "" ] ;then
                TABLEOPTS_TESTS=${testname}
                if [ "$kind" = "ser" ] || [ "$kind" = "ver" ] ; then
                    tableopts_tables=${testname}.tables
                fi
            else
                TABLEOPTS_TESTS="${TABLEOPTS_TESTS} ${testname}"
                if [ "$kind" = "ser" ] || [ "$kind" = "ver" ] ; then
                    tableopts_tables="${tableopts_tables} ${testname}.tables"
                fi
            fi
            TABLEOPTS_TESTS="${TABLEOPTS_TESTS}"'$(EXEEXT)'

            cat << EOF
tableopts_${kind}_${threading}_${bare_opt}_${kind}_SOURCES = tableopts.l4

${testname}\$(EXEEXT): tableopts_${kind}_${threading}-${bare_opt}.\$(OBJEXT)
	\$(AM_V_CCLD)\$(LINK) \$<

EOF
        done
    done
done

echo TABLEOPTS_TESTS = ${TABLEOPTS_TESTS}
echo
echo tableopts_tables = ${tableopts_tables}
