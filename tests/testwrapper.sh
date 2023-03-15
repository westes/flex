#!/bin/sh
echo 'set -euvx'
set -euvx

# testwrapper.sh: run a flex test, typically called by a Makefile

# Each test will exercise some feature or aspect of flex. Run the test with any
# input it may need.

INPUT_DIRECTORY=""
INPUT_NAME=""
INPUT_COUNT=0
USE_REDIRECT=0

while getopts d:i:r OPTION ; do
    case $OPTION in
        d) INPUT_DIRECTORY=$OPTARG ;;
        i)
            if [ "$INPUT_NAME" = "" ] ; then
                INPUT_NAME="$OPTARG"
            else
                INPUT_NAME="$INPUT_NAME $OPTARG"
            fi
            INPUT_COUNT=$((INPUT_COUNT+1))
            ;;
        r) USE_REDIRECT=1 ;;
        *) echo "Usage: ${0} [-d INPUT_DIRECTORY] [-i INPUT_NAME] [-r] TESTNAME"
           exit 1
           ;;
    esac
done

shift $((OPTIND-1))
TESTNAME=$1

# There may be a specific input file for this test
INPUT_NAME=${INPUT_NAME:-$INPUT_DIRECTORY/$(basename "${TESTNAME%.exe}").txt}

# If it doesn't exist, try stripping out a backend suffix.
# There might be a generic input for all tests with this stem.
# For this purpose we consider _r and _nr to be back ends.
inputs=$INPUT_NAME
INPUT_NAME=
for input in $inputs; do
    if [ ! -f "${input}" ] ; then
	input=$(echo "${input}" | sed -e 's/_[a-z0-9]*.txt$/.txt/')
    fi
    if [ "${INPUT_NAME}" = "" ] ; then
        INPUT_NAME="${input}"
    else
        INPUT_NAME="${INPUT_NAME} ${input}"
    fi
done

# Detect if the test wants a table argument.  If it does, the test maker will have generated
# a tables=-file option do that the a table file named afte the test was created when the
# scanner was built. Thus we can assume that it will be looking for the table data at
# the matching path we're about to generate.
case ${TESTNAME} in
    *ver|*ser|*_ver_*|*_ser_*) USE_TABLES=1 ;;
esac

if [ $INPUT_COUNT -gt 1 ] ; then
    # INPUT_NAME has multiple filenames, so we do want word expansion
    # shellcheck disable=SC2086
    $TESTNAME ${USE_TABLES:+"${INPUT_DIRECTORY}/${TESTNAME%.exe}.tables"} ${INPUT_NAME}
    exit $?
fi

# Set up input redirection as required
if [ -f "${INPUT_NAME}" ] ; then
    if [ $USE_REDIRECT = 1 ] ; then
        $TESTNAME ${USE_TABLES:+${INPUT_DIRECTORY}/${TESTNAME%.exe}.tables} < "$INPUT_NAME"
    else
        $TESTNAME ${USE_TABLES:+${INPUT_DIRECTORY}/${TESTNAME%.exe}.tables} "$INPUT_NAME"
    fi
else
    $TESTNAME
fi
