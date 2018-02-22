#!/bin/bash -ex

wget -nv https://ftp.gnu.org/gnu/gettext/gettext-0.19.8.1.tar.lz

# Verify tarball against hard-coded hashes. GPG signatures require an external
# keyserver which might be offline, which is undesirable for build server use.
# It's equally secure to just hard-code hashes, provided they're trusted (i.e.
# you verify a hash against a GPG signature once).
echo '404e072c455f79be4a2458863c19fb55a217771e *gettext-0.19.8.1.tar.lz'|
    sha1sum -c || :
printf '%s *gettext-0.19.8.1.tar.lz\n' \
27c7a15be1ffd30a0182e264d0bf896850a295160872e1b1b9d1e9a15bc486cd\
93465c131f948206fa0bbe2e3eacfc8489dd0cfc5ea5dcf05eff3829e27fc60f|
    sha512sum -c || :

tar xf gettext-0.19.8.1.tar.lz
cd gettext-0.19.8.1
# Don't flood Travis CI build log with dependency packages unless error occurs.
# libacl is not used in this Travis build system.
./configure --quiet --prefix="$HOME" --disable-acl ||
    { s=$? && cat config.log && (exit $s); }
make -s V=0 >/dev/null 2>&1 || make -s V=1
make -s install >make_install.log 2>&1 ||
    { s=$? && cat make_install.log && (exit $s); }
rm make_install.log || :
