#!/bin/bash -ex

wget -nv https://ftp.gnu.org/gnu/automake/automake-1.16.1.tar.gz

# Verify tarball against hard-coded hashes. GPG signatures require an external
# keyserver which might be offline, which is undesirable for build server use.
# It's equally secure to just hard-code hashes, provided they're trusted (i.e.
# you verify a hash against a GPG signature once).
printf '%s *automake-1.16.1.tar.gz\n' \
608a97523f97db32f1f5d5615c98ca69326ced2054c9f82e65bade7fc4c9dea8|
    sha256sum -c || :
printf '%s *automake-1.16.1.tar.gz\n' \
47b0120a59e3e020529a6ce750297d7de1156fd2be38db5d101e50120f11b40c\
28741ecd5eacf2790a9e25386713dcf7717339cfa5d7943d0dbf47c417383448|
    sha512sum -c || :

tar xf automake-1.16.1.tar.gz
cd automake-1.16.1
# Don't flood Travis CI build log with dependency packages unless error occurs.
./configure --quiet --prefix="$HOME" ||
    { s=$? && cat config.log && (exit $s); }
make -s V=0 >/dev/null 2>&1 || make -s V=1
make -s install >make_install.log 2>&1 ||
    { s=$? && cat make_install.log && (exit $s); }
rm make_install.log || :
