# INSTALLING Flex

## Should you be here at all?

If building, developing or compiling C programs is new to you, you
probably want to use your operating system's standard means of
installing software to install flex. If you do not need the latest
features of flex, you probably just want to use your operating
system's standard means of installing software to obtain flex.

## Now that you know you should be here ...

If you are not familiar with bootstrapping C code from a git
repository or if GNU autotools seems like a jumble of tenuous
incantationery to you, then make sure that you downloaded one of the
release tar archives of flex. You can verify this by checking the
filename of what you downloaded. If it is something like
`flex-<version>.tar.<compressiontype>` then you have a release tar
archive. If you have a filename like `flex.tar.gz` or `flex.zip`, you
have a copy of the git repository and you didn't download the thing
you wanted to.

## Building from a release archive

To build flex from a release archive:

```bash
$ ./configure <any configure options you need>
$ make
```

To see what options are available from the configure script:

```bash
$ ./configure --help
```

Optionally run the test suite:

```bash
$ make check
```

To install the flex you just built:

```bash
$ make install
```

Note that you may want to make use of the DESTDIR argument on the
`make install` command line or that you may want to have used the
`--prefix` argument with configure (or mostly equivalently the
`prefix` argument on the make command line).

## Building from the git repository

To build from the git repository:

First, make sure you have a copy of flex installed somewhere on your
path so that configure can find it. You can usually do this with your
operating system's standard means of installing software. Sometimes,
you have to build from a recent release of flex, however. Using a
version of flex built from the flex codebase is always acceptable if
you have already bootstrapped doing so.

You will also need all the programs that flex needs in order to be
built from scratch:

* compiler suite - flex is built with gcc
* bash, or a good Bourne-style shell
* m4 - `m4 -P` needs to work; GNU m4 and a few others are suitable
* GNU bison;  to generate parse.c from parse.y
* autoconf; for handling the build system
* automake; for Makefile generation
* make; for running the generated Makefiles
* gettext; for i18n support
* help2man; to generate the flex man page
* tar, gzip, lzip, etc.; for packaging of the source distribution
* GNU texinfo; to build and test the flex manual. Note that if you want
  to build the dvi/ps/pdf versions of the documentation you will need
  texi2dvi and related programs, along with a sufficiently powerful
  implementation of TeX to process them. See your operating system
  documentation for how to achieve this. The printable versions of the
  manual are not built unless specifically requested, but the targets
  are included by automake.
* GNU indent; for indenting the flex source the way we want it done

In cases where the versions of the above tools matter, the file
configure.ac will specify the minimum required versions.

Then:

```bash
$ ./autogen.sh
```

After autogen.sh finishes successfully, building flex follows the same
steps as building flex from a release archive.

Note that, in addition to `make check`, `make distcheck` builds a
release archive and builds and tests flex from inside a directory
containing only known distributed files.
