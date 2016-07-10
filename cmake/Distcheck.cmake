# - adds support for the 'make distcheck' command      -*- cmake -*-
# Dependencies:
# 1. CPack generating ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.gz.
# 2. Having a "dist" target, e.g:
#    add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
# Usage:
#   add_distcheck()    ... called exactly once per project in the top-level
#                         CMakeLists.txt; it adds the 'dist' and 'distcheck'
#                         targets
#
# This module implements the 'make dist' and 'make distcheck'
# commands.
# It supports the following variables:
#
#   DISTCHECK_TMPDIR   ... directory for temporary files
#   DISTCHECK_FILENAME ... basename of existing tar.gz.; defaults to
#                         ${CPACK_SOURCE_PACKAGE_FILE_NAME}
#   DISTCHECK_CMAKEFLAGS
#                     ... flags which are given to 'cmake' by 'make distcheck'
#   DISTCHECK_BUILDTARGETS
#                     ... the build-targets tried by 'make distcheck';
#                         defaults to nothing (--> all)
#   DISTCHECK_INSTALLTARGETS
#                     ... the install-targets tried by 'make distcheck';
#                         defaults to 'install'
#
# Example:
#   --- top-level CMakeLists.txt ---
#   add_subdirectory(foo)
#   ...
#   ...
#   set(CPACK_PACKAGE_VERSION_MAJOR ${ECRIRE_VERSION_MAJOR})
#   set(CPACK_PACKAGE_VERSION_MINOR ${ECRIRE_VERSION_MINOR})
#   set(CPACK_PACKAGE_VERSION_PATCH ${ECRIRE_VERSION_MICRO})
#   set(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
#   set(CPACK_SOURCE_PACKAGE_FILE_NAME
#      "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
#   set(CPACK_SOURCE_IGNORE_FILES
#      "/build/;/.git/;~$;${CPACK_SOURCE_IGNORE_FILES}")
#   include(CPack)
#   add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
#
#   find_package(Distcheck)
#   add_distcheck()
#
#
# Copyright (C) 2012 Tom Hacohen <tom@stosb.com>
# Based on the work done by:
# Copyright (C) 2006 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
#
# Redistribution and use, with or without modification, are permitted
# provided that the following conditions are met:
# 
#    1. Redistributions must retain the above copyright notice, this
#       list of conditions and the following disclaimer.
#    2. The name of the author may not be used to endorse or promote
#       products derived from this software without specific prior
#       written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

macro(add_distcheck)
    set(MakeDist_FOUND 1)

    set(DISTCHECK_TMPDIR         "${CMAKE_BINARY_DIR}/.make-dist"   CACHE PATH "directory for temporary files created by'make dist*'")
    set(DISTCHECK_FILENAME       ${CPACK_SOURCE_PACKAGE_FILE_NAME}  CACHE PATH "basename of the tarball created by 'make dist'")
    set(DISTCHECK_CMAKEFLAGS     -DWITH_TESTS=1                     CACHE STRING "flags which are given to 'cmake' by 'make distcheck'")
    set(DISTCHECK_BUILDTARGETS   ""                                 CACHE STRING "build-target(s) tried by 'make distcheck'")
    set(DISTCHECK_CHECKTARGETS   "check"                            CACHE STRING "check-target(s) tried by 'make distcheck'")
    set(DISTCHECK_INSTALLTARGETS install                            CACHE STRING "install-target(s) tried by 'make distcheck'")

    mark_as_advanced(DISTCHECK_TMPDIR DISTCHECK_FILENAME DISTCHECK_CMAKEFLAGS DISTCHECK_BUILDTARGETS DISTCHECK_INSTALLTARGETS)

    set(DISTCHECK_BASESOURCEDIR "${DISTCHECK_TMPDIR}/source")
    set(DISTCHECK_SOURCEDIR "${DISTCHECK_BASESOURCEDIR}/${DISTCHECK_FILENAME}")
    set(DISTCHECK_BUILDDIR "${DISTCHECK_TMPDIR}/build")
    set(DISTCHECK_INSTALLTARGETS "install")

    add_custom_target(distcheck
        # Create the tarball
        COMMAND ${CMAKE_MAKE_PROGRAM} dist

        # Create the temp dir.
        COMMAND chmod -Rf a+w "${DISTCHECK_TMPDIR}" 2>/dev/null || :
        COMMAND rm -rf "${DISTCHECK_TMPDIR}"
        COMMAND mkdir -p "${DISTCHECK_SOURCEDIR}" "${DISTCHECK_BUILDDIR}"

        COMMAND echo
        COMMAND echo ls -l
        COMMAND ls -l
        
        # extract tarball
        COMMAND tar -xzvf ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.gz -C "${DISTCHECK_BASESOURCEDIR}"
        
        # write-protect sources to detect modifies-sourcetree bugs
        COMMAND chmod -R a-w "${DISTCHECK_SOURCEDIR}"

        COMMAND cd "${DISTCHECK_BUILDDIR}" && ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX:PATH="${DISTCHECK_TMPDIR}/install" ${DISTCHECK_CMAKEFLAGS} "${DISTCHECK_SOURCEDIR}"

        # execute 'make build' and 'make check'
        COMMAND cd "${DISTCHECK_BUILDDIR}" && ${CMAKE_MAKE_PROGRAM} ${DISTCHECK_BUILDTARGETS}
        COMMAND cd "${DISTCHECK_BUILDDIR}" && ${CMAKE_MAKE_PROGRAM} ${DISTCHECK_CHECKTARGETS}

        # execute 'make install' without DESTDIR
        COMMAND cd "${DISTCHECK_BUILDDIR}"  && ${CMAKE_MAKE_PROGRAM} ${DISTCHECK_INSTALLTARGETS} DESTDIR=
        # write protect installation path to detect writing outside of DESTDIR
        COMMAND chmod -R a-w "${DISTCHECK_TMPDIR}/install"
        # execute 'make install' with DESTDIR and move the files to a better location
        COMMAND cd "${DISTCHECK_BUILDDIR}"  && ${CMAKE_MAKE_PROGRAM} ${DISTCHECK_INSTALLTARGETS} DESTDIR="${DISTCHECK_TMPDIR}/install-tmp"
        COMMAND mv "${DISTCHECK_TMPDIR}/install-tmp/${DISTCHECK_TMPDIR}/install" "${DISTCHECK_TMPDIR}/install-destdir"

        # generate list of files which were installed by the both 'make
        # install' commands above and compare them
        COMMAND cd "${DISTCHECK_TMPDIR}/install"         && find -type f | sort > ../files.install
        COMMAND cd "${DISTCHECK_TMPDIR}/install-destdir" && find -type f | sort > ../files.destdir
        COMMAND cd "${DISTCHECK_TMPDIR}" && diff files.install files.destdir

        # cleanup tmpdir
        COMMAND chmod -R u+Xw "${DISTCHECK_TMPDIR}" 2>/dev/null || :
        COMMAND rm -rf "${DISTCHECK_TMPDIR}"
    )
endmacro(add_distcheck)

