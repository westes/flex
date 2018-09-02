include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckPrototypeDefinition)


check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stddef.h HAVE_STDDEF_H)
check_include_files(inttypes.h HAVE_INITTYPES_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)

if(HAVE_DLFCN_H AND HAVE_STDINT_H AND HAVE_STDDEF_H AND HAVE_INITTYPES_H AND
   HAVE_STDLIB_H AND HAVE_STRINGS_H AND HAVE_STRING_H )
	check_prototype_exists(memchr string.h memchrExists)
	if(memchrExists)
    check_prototype_exists(free stdlib.h freeExists)
		if(freeExists)
			message(STATUS "ANSI C header files - found")
      set(STDC_HEADERS 1 CACHE INTERNAL "System has ANSI C header files")
			#set(HAVE_STRINGS_H 1)
			#set(HAVE_STRING_H 1)
			#set(HAVE_STDLIB_H 1)
			#set(HAVE_STDDEF_H 1)
			#set(HAVE_STDINT_H 1)
			#set(HAVE_INTTYPES_H 1)
			#set(HAVE_DLFCN_H 1)
		endif(freeExists)
	endif(memchrExists)
endif()



check_function_exists(alloca HAVE_ALLOCA)
check_include_files(alloca.h HAVE_ALLOCA_H_CHECK)
if(ENABLE_C_ALLOCA AND HAVE_ALLOCA_H_CHECK)
  set(HAVE_ALLOCA_H TRUE)
  if(NOT STACK_DIRECTION)
    set(STACK_DIRECTION 0) # auto - direction of growth unknown
  endif()
endif()

check_function_exists("available." HAVE_AVAILABLE_)
check_function_exists(by HAVE_BY)

if(APPLE)
  check_function_exists(CFLocaleCopyCurrent HAVE_CFLOCALECOPYCURRENT)
  check_function_exists(CFPreferencesCopyAppValue HAVE_CFPREFERENCESCOPYAPPVALUE)
endif()

check_function_exists(dcgettext HAVE_DCGETTEXT)
check_function_exists(dnl HAVE_DNL)
check_function_exists(dup2 HAVE_DUP2)
check_function_exists(enabled HAVE_ENABLED)
check_function_exists(fork HAVE_FORK)
check_function_exists(function. HAVE_FUNCTION_)
check_function_exists(gettext HAVE_GETTEXT)
check_function_exists(have HAVE_HAVE)
check_function_exists(iconv HAVE_ICONV)
check_function_exists(if HAVE_IF)
check_function_exists(is HAVE_IS)
check_include_files(libintl.h HAVE_LIBINTL_H)
check_library_exists(m log10 "" HAVE_LIBM)
check_include_files(limits.h HAVE_LIMITS_H)
check_include_files(locale.h HAVE_LOCALE_H)
check_function_exists(malloc HAVE_MALLOC)
check_include_files(malloc.h HAVE_MALLOC_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_function_exists(memset HAVE_MEMSET)
check_function_exists(Needed HAVE_NEEDED)
check_include_files(netinet/in.h HAVE_NETINET_IN_H)
check_include_files("arpa/inet.h" HAVE_ARPA_INET_H)
if(NOT HAVE_ARPA_INET_H)
  configure_file(${CMAKE_SOURCE_DIR}/cmake/swap.c ${CMAKE_SOURCE_DIR}/src/swap.c COPYONLY)
endif()
check_function_exists(NLS HAVE_NLS)
check_function_exists("not" HAVE_NOT)
check_function_exists(only HAVE_ONLY)
check_function_exists(OpenBSD HAVE_OPENBSD)
check_function_exists(pow HAVE_POW)
check_include_files(pthread.h HAVE_PTHREAD)
check_function_exists(realloc HAVE_REALLOC)
check_function_exists(reallocarray HAVE_REALLOCARRAY)
check_function_exists(regcomp HAVE_REGCOMP)
check_include_files(regex.h HAVE_REGEX_H)
if(NOT HAVE_REGX_H)
  #add_definitions(-D_LIBC)
  configure_file(${CMAKE_SOURCE_DIR}/cmake/regex.h ${CMAKE_SOURCE_DIR}/src/regex.h COPYONLY)
  #configure_file(${CMAKE_SOURCE_DIR}/cmake/regex.c ${CMAKE_SOURCE_DIR}/src/regex.c COPYONLY)
endif()
check_function_exists(replacement HAVE_REPLACEMENT)
check_function_exists(setlocale HAVE_SETLOCALE)
check_include_files(stdbool.h HAVE_STDBOOL_H)
check_function_exists(strcasecmp HAVE_STRCASECMP)
check_function_exists(_stricmp HAVE__STRICMP)
check_function_exists(strchr HAVE_STRCHR)
check_function_exists(strdup HAVE_STRDUP)
check_function_exists(strtol HAVE_STRTOL)
check_include_files("sys/stat.h" HAVE_SYS_STAT_H)
check_include_files("sys/types.h" HAVE_SYS_TYPES_H)
check_include_files("sys/wait.h" HAVE_SYS_WAIT_H)
check_include_files(unistd.h HAVE_UNISTD_H)
if(NOT HAVE_UNISTD_H)
  add_definitions(-DYY_NO_UNISTD_H)
endif()
check_function_exists(Used HAVE_USED)
check_function_exists(vfork HAVE_VFORK)
check_include_files(vfork.h HAVE_VFORK_H)
check_function_exists(We HAVE_WE)

# Define to 1 if `fork' works.
set(HAVE_WORKING_FORK FALSE)

# Define to 1 if `vfork' works.
set(HAVE_WORKING_VFORK FALSE)

check_type_size(_Bool HAVE__BOOL_SIZE)
if(HAVE__BOOL_SIZE)
  set(HAVE__BOOL 1)
else()
  set(HAVE__BOOL 0)
endif()


# Define to the m4 executable name.
set(M4 "/usr/bin/m4")


# Parse package info from configure.ac
file(STRINGS "configure.ac" flex_ACINIT_LINE LIMIT_COUNT 1 REGEX "^AC_INIT")

#Before: "AC_INIT([the fast lexical analyser generator],[2.6.4],[flex-help@lists.sourceforge.net],[flex])"
string(REPLACE "(" "_" flex_ACINIT_LINE ${flex_ACINIT_LINE})
string(REPLACE ")" "_" flex_ACINIT_LINE ${flex_ACINIT_LINE})
string(REPLACE "[" "_" flex_ACINIT_LINE ${flex_ACINIT_LINE})
string(REPLACE "]" "_" flex_ACINIT_LINE ${flex_ACINIT_LINE})
#After: "AC_INIT__the fast lexical analyser generator_,_2.6.4_,_flex-help@lists.sourceforge.net_,_flex__"

# Parse name
string(REGEX REPLACE "^AC_INIT__([^_]+).*$" "\\1" flex_PACKAGE_NAME "${flex_ACINIT_LINE}")
MESSAGE(STATUS "PACAKGE_NAME=${flex_PACKAGE_NAME}")

# Parse version string
string(REGEX REPLACE "^AC_INIT__${flex_PACKAGE_NAME}_,_([^_]+).*$" "\\1" flex_VERSION_STRING "${flex_ACINIT_LINE}")
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+).*$" "\\1"
  flex_VERSION_MAJOR "${flex_VERSION_STRING}")
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+).*$" "\\2"
  flex_VERSION_MINOR "${flex_VERSION_STRING}")
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+).*$" "\\3"
  flex_VERSION_PATCH "${flex_VERSION_STRING}")

# Package version
set(flex_VERSION "${flex_VERSION_MAJOR}.${flex_VERSION_MINOR}.${flex_VERSION_PATCH}")

# Name of package
set(PACKAGE "flex")

# Define to the address where bug reports for this package should be sent.
set(PACKAGE_BUGREPORT ${flex_BUGREPORT_STRING})
MESSAGE(STATUS "PACKAGE_BUGREPORT=${PACKAGE_BUGREPORT}")

# Define to the full name of this package.
set(PACKAGE_NAME ${flex_NAME})
MESSAGE(STATUS "PACKAGE_NAME=${PACKAGE_NAME}")

# Define to the full name and version of this package.
set(PACKAGE_STRING "")

# Define to the one symbol short name of this package.
set(PACKAGE_TARNAME "")

# Define to the home page for this package.
set(PACKAGE_URL "")

# Define to the version of this package.
set(PACKAGE_VERSION ${flex_VERSION})
MESSAGE(STATUS "PACKAGE_VERSION=${PACKAGE_VERSION}")

# If using the C implementation of alloca, define if you know the
#   direction of stack growth for your system; otherwise it will be
#   automatically deduced at runtime.
#	STACK_DIRECTION > 0 => grows toward higher addresses
#	STACK_DIRECTION < 0 => grows toward lower addresses
#	STACK_DIRECTION = 0 => direction of growth unknown
#cmakedefine STACK_DIRECTION


# Version number of package
set(VERSION "\"${flex_VERSION_MAJOR}.${flex_VERSION_MINOR}.${flex_VERSION_PATCH}\";")

# Define to 1 if `lex' declares `yytext' as a `char *' by default, not a `char[]'.
#cmakedefine YYTEXT_POINTER

# Define to empty if `const' does not conform to ANSI C.
#cmakedefine const

# Define to rpl_malloc if the replacement function should be used.
#cmakedefine malloc

# Define to `int' if <sys/types.h> does not define.
check_type_size(pid_t PID_T_TYPE_SIZE)
if(NOT PID_T_TYPE_SIZE)
  MESSAGE(STATUS "Setting pid_t to int")
  set(pid_t "int")
endif()

# Define to rpl_realloc if the replacement function should be used.
#cmakedefine realloc

# Define to `unsigned int' if <sys/types.h> does not define.
check_type_size(size_t SIZE_T_TYPE_SIZE)
if(NOT SIZE_T_TYPE_SIZE)
  MESSAGE(STATUS "Setting size_t to unsigned int")
  set(size_t "unsigned int")
endif()

# Define as `fork' if `vfork' does not work.
#cmakedefine vfork

check_include_files(libgen.h HAVE_LIBGEN_H)
check_include_files(assert.h HAVE_ASSERT_H)

include(TestBigEndian)
test_big_endian(HOST_IS_BIG_ENDIAN)
if(NOT HOST_IS_BIG_ENDIAN)
  add_definitions(-DHOST_IS_LITTLE_ENDIAN)
endif()


CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake/config.h.in ${CMAKE_SOURCE_DIR}/src/config.h @ONLY)
add_definitions(-DHAVE_CONFIG_H)