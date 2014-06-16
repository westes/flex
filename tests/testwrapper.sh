#!/bin/bash -vx

# testwrapper.sh: run a flex test, typically called by a Makefile

# Each test will exercise some feature or aspect of flex. Run the test with any input it may need.

INPUT_DIRECTORY=""
INPUT_NAME=""

while getopts :d:i:t OPTION ; do
    case $OPTION in
        d) INPUT_DIRECTORY=$OPTARG ;;
        i) INPUTNAME="$OPTARG" ;;
        t) USE_TABLES=1 ;;
    esac
    done

TESTNAME="${!$OPTIND}"

INPUTNAME=${INPUT_NAME:-`basename $TESTNAME`.txt}

if [ -f $INPUT_DIRECTORY/$INPUT_NAME ] ; then
    $TESTNAME ${USE_TABLES:+${INPUT_DIRECTORY}${TESTNAME}} < $INPUT_DIRECTORY/$INPUT_NAME
else
    $TESTNAME
fi
