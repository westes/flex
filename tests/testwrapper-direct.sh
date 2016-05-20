#!/bin/bash -vx
set -euo pipefail

# testwrapper-direct.sh: run some specialized flex tests that care where they're run from.

while getopts :b:s: OPTION ; do
    case $OPTION in
        b)  BINARY_DIR=$OPTARG ;;
        s) SOURCE_DIR=$OPTARG ;;
    esac
    done

TESTNAME="${!OPTIND}"

INPUT_NAME=`basename ${TESTNAME%.exe}`.txt

cd ${SOURCE_DIR} && ${BINARY_DIR}/${TESTNAME} ${INPUT_NAME}
