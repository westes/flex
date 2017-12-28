#!/bin/bash -ex

wget -nv https://ftp.gnu.org/gnu/automake/automake-1.15.1.tar.gz{,.sig}
gpg2 --keyserver hkp://keys.gnupg.net --recv-keys 94604D37
gpg2 automake-1.15.1.tar.gz.sig 
tar xf automake-1.15.1.tar.gz
cd automake-1.15.1
./configure --prefix=$HOME
make
make install
