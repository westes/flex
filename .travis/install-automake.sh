#!/bin/bash -ex

wget -nv https://ftp.gnu.org/gnu/automake/automake-1.15.1.tar.gz

# Verify tarball against hard-coded hashes. GPG signatures require an external
# keyserver which might be offline, which is undesirable for build server use.
# It's equally secure to just hard-code hashes, provided they're trusted (i.e.
# you verify a hash against a GPG signature once).
echo 'd3cd5fc9bbea9f977b51799180cde5d253dcba96 *automake-1.15.1.tar.gz'|
    sha1sum -c || :
printf '%s *automake-1.15.1.tar.gz\n' \
f0d4717ebe2c76cec5d487de090f6e1c0f784b0d382fd964ffa846287e2a364a\
52531a26ab98b7033ac04ed302a247b3b114299def54819a03439bfc962ff61b|
    sha512sum -c || :

tar xf automake-1.15.1.tar.gz
cd automake-1.15.1
# Don't flood Travis CI build log with dependency packages unless error occurs.
./configure --quiet --prefix="$HOME" ||
    { s=$? && cat config.log && (exit $s); }
make -s V=0 >/dev/null 2>&1 || make -s V=1
make -s install >make_install.log 2>&1 ||
    { s=$? && cat make_install.log && (exit $s); }
rm make_install.log || :
