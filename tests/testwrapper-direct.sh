#!/bin/sh
echo 'set -euvx'
set -euvx

# testwrapper-direct.sh: run some specialized flex tests that care where
# they're run from.
BINARY_DIR="."
SOURCE_DIR="."

while getopts :b:s: OPTION ; do
    case $OPTION in
        b) BINARY_DIR=$OPTARG ;;
        s) SOURCE_DIR=$OPTARG ;;
        *) echo "Usage: ${0} [-b BINARY_DIR] [-s SOURCE_DIR] TESTNAME"
           exit 1
           ;;
    esac
    done

shift $((OPTIND-1))
TESTNAME=$1

INPUT_NAME=$(basename "${TESTNAME%.exe}").txt

cd "${SOURCE_DIR}" && "${BINARY_DIR}/${TESTNAME}" "${INPUT_NAME}"
