#! /bin/sh

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

if test ! $# = 4; then
   echo 'Usage: mkskel.sh lang srcdir m4 version' >&2
   exit 1
fi
lang=$1
srcdir=$2
m4=$3
VERSION=$4
case $VERSION in
   *[!0-9.]*) echo 'Invalid version number' >&2; exit 1;;
esac

cr=`printf '\r'`

IFS=.
# we do want word splitting, so we won't put double quotes around it (see IFS above)
# shellcheck disable=2086
set -- $VERSION
sed 's/4_/a4_/g
s/m4preproc_/m4_/g
' "$srcdir/${lang}-flex.skl" |
"$m4" -P -I "$srcdir" "-DFLEX_MAJOR_VERSION=$1" \
   "-DFLEX_MINOR_VERSION=$2" \
   "-DFLEX_SUBMINOR_VERSION=$3" |
sed '/^%#/d
s/m4_/m4preproc_/g
s/a4_/4_/g
s/[\\"]/\\&/g
s/[^'"$cr"']*/  "&",/'
