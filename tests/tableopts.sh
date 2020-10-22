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
    for threading in nr r c99 ; do
        for opt in -Ca -Ce -Cf -CF -Cm -Cem -Cae -Caef -CaeF -Cam -Caem ; do
            bare_opt=${opt#-}
            # The filenames must work on case-insensitive filesystems.
            bare_opt=$(echo ${bare_opt}| sed 's/F$/xF/')

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

            cat << EOF
tableopts_${kind}_${threading}_${bare_opt}_${kind}_SOURCES = ${testname}.l
${testname}.l: \$(srcdir)/tableopts.rules \$(srcdir)/testmaker.sh \$(srcdir)/testmaker.m4
	\$(SHELL) \$(srcdir)/testmaker.sh \$@

EOF
        done
    done
done

echo TABLEOPTS_TESTS = "${TABLEOPTS_TESTS}"
echo
echo tableopts_tables = "${tableopts_tables}"
# User can pass in a list of tesrs for which ti geneate build productions
printf "\n# Begin simple test rules\n\n"
for test in "$@"; do
    stem=$(echo "${test}" | sed s/_.*//)
    echo "${test}_SOURCES = ${test}.l"
    echo "${test}.l: \$(srcdir)/${stem}.rules \$(srcdir)/testmaker.sh \$(srcdir)/testmaker.m4"
    # we're deliberately single-quoting this because we _don't_ want those variables to be expanded yet
    # shellcheck disable=2016
    printf '\t$(SHELL) $(srcdir)/testmaker.sh $@\n\n'
done
printf "# End simple test rules\n"
