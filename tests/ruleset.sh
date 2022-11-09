#!/bin/sh
# Generate make productions for testing files from rulesets.  Also set up SOURCES
# variables for link time.  Pass it a list of back-end suffixes.
#
# This script exists because automake isn't able to handle the pattern rules that
# would be natural to use. Output is written to standard output for
# inclusion in a Makefile.am, typically by redirecting the output and
# then an automake include directive.
set -eu

RULESET_TESTS=""
RULESET_REMOVABLES=""
TESTMAKEROPTS=""

while getopts i:t OPTION ; do
    case $OPTION in
	    i) TESTMAKEROPTS="${TESTMAKEROPTS} -i ${OPTARG}" ;;
		t) TESTMAKEROPTS="${TESTMAKEROPTS} -t" ;;
        *) echo "Usage: ${0} [-d]"
           exit 1
           ;;
    esac
done

shift $((OPTIND-1))

printf "\n# Begin generated test rules\n\n"

compatible() {
    mybackend=$1
    myruleset=$2
    # Some options are both a pain to test outside the default back end and really don't need to be
    # tested in more than one back end anyway.  An option is in this category if it doesn't affect
    # any conditionals in the code generation, just the way the Flex scanner generates its NDFSA tables.
    [ "${mybackend}" = "nr" ] || [ "${myruleset}" != "lexcompat.rules" -a "${myruleset}" != "posixlycorrect.rules" ]
}

for backend in "$@" ; do
    for ruleset in *.rules; do
	if compatible "${backend}" "${ruleset}" ; then
	    testname="${ruleset%.*}_${backend}"
	    echo "${testname}_SOURCES = ${testname}.l"
	    echo "${testname}.l: \$(srcdir)/${ruleset} \$(srcdir)/testmaker.sh \$(srcdir)/testmaker.m4"
	    # we're deliberately single-quoting this because we _don't_ want those variables to be expanded yet
	    # shellcheck disable=2016
	    printf '\t$(SHELL) $(srcdir)/testmaker.sh -i $(srcdir) $@\n\n'
	    RULESET_TESTS="${RULESET_TESTS} ${testname}"
	    RULESET_REMOVABLES="${RULESET_REMOVABLES} ${testname} ${testname}.c ${testname}.l"
	fi
    done
    for kind in opt ser ver ; do
        for opt in -Ca -Ce -Cf -CF -Cm -Cem -Cae -Caef -CaeF -Cam -Caem ; do
            bare_opt=${opt#-}
            # The filenames must work on case-insensitive filesystems.
            bare_opt=$(echo ${bare_opt}| sed 's/F$/xF/')
            testname=tableopts_${kind}_${backend}-${bare_opt}.${kind}
            RULESET_TESTS="${RULESET_TESTS} ${testname}"
            RULESET_REMOVABLES="${RULESET_REMOVABLES} ${testname} ${testname}.c ${testname}.l ${testname}.tables"
            cat << EOF
tableopts_${kind}_${backend}_${bare_opt}_${kind}_SOURCES = ${testname}.l
${testname}.l: \$(srcdir)/tableopts.rules \$(srcdir)/testmaker.sh \$(srcdir)/testmaker.m4
	\$(SHELL) \$(srcdir)/testmaker.sh -i \$(srcdir) \$@

EOF
        done
    done
done

# posixlycorrect is a special case because we need to set POSIXLY_CORRECT
# in Flex's environment while these .l files are being processed.
for backend in "$@" ; do
    case $backend in
	nr|r|c99) ext="c" ;;
	*) ext=${backend} ;;
    esac
    # shellcheck disable=SC2059
    printf "posixlycorrect_${backend}.${ext}: posixlycorrect_${backend}.l \$(FLEX)\n"
    printf "\t\$(AM_V_LEX)POSIXLY_CORRECT=1 \$(FLEX) \$(TESTOPTS) -o \$@ \$<\n"
    echo ""

    echo "test_yydecl_${backend}_sh_SOURCES ="
    echo "test-yydecl-${backend}.sh\$(EXEEXT): \$(srcdir)/test-yydecl-gen.sh"
    # shellcheck disable=SC2059
    printf "\t\$(SHELL) \$(srcdir)/test-yydecl-gen.sh ${backend} \$(FLEX) >test-yydecl-${backend}.sh\$(EXEEXT)\n"
    # shellcheck disable=SC2059
    printf "\tchmod a+x test-yydecl-${backend}.sh\$(EXEEXT)\n"
    echo ""

    RULESET_TESTS="${RULESET_TESTS} test-yydecl-${backend}.sh\$(EXEEXT)"
    RULESET_REMOVABLES="${RULESET_REMOVABLES} test-yydecl-${backend}.sh\$(EXEEXT)"
done

echo ""
printf "# End generated test rules\n"

echo RULESET_TESTS = "${RULESET_TESTS}"
echo RULESET_REMOVABLES = "${RULESET_REMOVABLES}"
echo


