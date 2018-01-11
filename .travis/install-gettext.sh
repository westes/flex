#!/bin/bash -ex

wget -nv https://ftp.gnu.org/gnu/gettext/gettext-0.19.8.1.tar.lz{,.sig}
gpg2 --keyserver hkp://keys.gnupg.net --recv-keys D7E69871
gpg2 gettext-0.19.8.1.tar.lz.sig 
tar xf gettext-0.19.8.1.tar.lz
cd gettext-0.19.8.1
./configure --prefix=$HOME
make
make install
