#!/bin/bash -vx

# testwrapper.sh: run a flex test, typically called by a Makefile

# Each test will exercise some feature or aspect of flex. Run the test with any input it may need.

SRCDIR="$1"
TESTNAME="$2"
INPUTNAME=`basename $TESTNAME`.txt

if [ -f $SRCDIR/$INPUTNAME ] ; then
    $TESTNAME < $SRCDIR/$INPUTNAME
else
    $TESTNAME
fi
