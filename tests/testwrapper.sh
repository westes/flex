#!/bin/sh
set -vx
set -euvx

# testwrapper.sh: run a flex test, typically called by a Makefile

# Each test will exercise some feature or aspect of flex. Run the test with any
# input it may need.

INPUT_DIRECTORY=""
INPUT_NAME=""
INPUT_COUNT=0
USE_REDIRECT=0
DO_COMPARISON=0

while getopts :d:i:rt1 OPTION ; do
    case $OPTION in
        d) INPUT_DIRECTORY=$OPTARG ;;
        i)
            if [ "$INPUT_NAME" = "" ] ; then
                INPUT_NAME="$OPTARG"
            else
                INPUT_NAME="$INPUT_NAME $OPTARG"
            fi
            INPUT_COUNT=$(($INPUT_COUNT+1))
            ;;
        r) USE_REDIRECT=1 ;;
        t) USE_TABLES=1 ;;
        1) DO_COMPARISON=1 ;;
    esac
done

shift $(($OPTIND-1))
TESTNAME=$1

INPUT_NAME=${INPUT_NAME:-$INPUT_DIRECTORY/`basename "${TESTNAME%.exe}"`.txt}

if [ $DO_COMPARISON = 1 ] ; then
    TEST_OUTPUT=`$TESTNAME < $INPUT_NAME`
    REF_OUTPUT=`$TESTNAME 1 < $INPUT_NAME`
    test "$TEST_OUTPUT" -eq "$REF_OUTPUT"
    exit $?
fi

if [ $INPUT_COUNT -gt 1 ] ; then
    $TESTNAME ${USE_TABLES:+${INPUT_DIRECTORY}/${TESTNAME%.exe}.tables} ${INPUT_NAME}
    exit $?
fi

if [ -f ${INPUT_NAME} ] ; then
    if [ $USE_REDIRECT = 1 ] ; then
        $TESTNAME ${USE_TABLES:+${INPUT_DIRECTORY}/${TESTNAME%.exe}.tables} < $INPUT_NAME
    else
        $TESTNAME ${USE_TABLES:+${INPUT_DIRECTORY}/${TESTNAME%.exe}.tables} $INPUT_NAME
    fi
else
    $TESTNAME
fi
