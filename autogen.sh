#!/bin/sh

#  This file is part of flex.

#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:

#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.

#  Neither the name of the University nor the names of its contributors
#  may be used to endorse or promote products derived from this software
#  without specific prior written permission.

#  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE.

# If you see no configure script, then run ./autogen.sh to create it
# and proceed with the "normal" build procedures.

# use LIBTOOLIZE, if set
if test "x$LIBTOOLIZE" = "x"; then
   for ac_prog in libtoolize glibtoolize; do
      if $ac_prog --version >/dev/null 2>&1; then
         LIBTOOLIZE=$ac_prog
         break
      fi
   done
fi

# test libtoolize
if test "x$LIBTOOLIZE" = "x" || ! $LIBTOOLIZE --version >/dev/null; then
   echo "error: libtoolize not working, re-run with LIBTOOLIZE=/path/to/libtoolize">&2
   echo "       LIBTOOLIZE is currently \"$LIBTOOLIZE\"">&2
   exit 1
fi

#if we pretend to have a ChangeLog, then automake is less
#worried. (Don't worry, we *do* have a ChangeLog, we just need the
#Makefile first.)

if ! test -f ChangeLog; then
   touch ChangeLog
fi
$LIBTOOLIZE --install --force
autoreconf --install --force
