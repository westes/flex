#!/bin/sh
echo 'set -euvx'
set -euvx

# testwrapper-direct.sh: run some specialized flex tests that care where
# they're run from.

while getopts :b:s: OPTION ; do
    case $OPTION in
        b) BINARY_DIR=$OPTARG ;;
        s) SOURCE_DIR=$OPTARG ;;
    esac
    done

shift $(($OPTIND-1))
TESTNAME=$1

INPUT_NAME=`basename "${TESTNAME%.exe}"`.txt

cd ${SOURCE_DIR} && ${BINARY_DIR}/${TESTNAME} ${INPUT_NAME}
