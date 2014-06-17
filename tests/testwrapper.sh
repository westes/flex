#!/bin/bash -vx

# testwrapper.sh: run a flex test, typically called by a Makefile

# Each test will exercise some feature or aspect of flex. Run the test with any input it may need.

INPUT_DIRECTORY=""
INPUT_NAME=""
USE_REDIRECT=0

while getopts :d:i:rt OPTION ; do
    case $OPTION in
        d) INPUT_DIRECTORY=$OPTARG ;;
        i) INPUT_NAME="$OPTARG" ;;
        r) USE_REDIRECT=1 ;;
        t) USE_TABLES=1 ;;
    esac
    done

TESTNAME="${!OPTIND}"

INPUT_NAME=${INPUT_NAME:-`basename $TESTNAME`.txt}

if [ -f ${INPUT_DIRECTORY}/${INPUT_NAME} ] ; then
    if [ $USE_REDIRECT == 1 ] ; then
        $TESTNAME ${USE_TABLES:+${INPUT_DIRECTORY}/${TESTNAME}.tables} < $INPUT_DIRECTORY/$INPUT_NAME
    else
        $TESTNAME ${USE_TABLES:+${INPUT_DIRECTORY}/${TESTNAME}.tables} $INPUT_DIRECTORY/$INPUT_NAME
    fi
else
    $TESTNAME
fi
