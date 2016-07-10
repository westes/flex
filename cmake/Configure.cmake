################################################################################
#
# configure
#
################################################################################

########################################
# FUNCTION check_includes
########################################
function(check_includes files)
    foreach(F ${${files}})
        set(name ${F})
        string(REPLACE "-" "_" name ${name})
        string(REPLACE "." "_" name ${name})
        string(REPLACE "/" "_" name ${name})
        string(TOUPPER ${name} name)
        check_include_files(${F} HAVE_${name})
        file(APPEND ${AUTOCONFIG_SRC} "/* Define to 1 if you have the <${F}> header file. */\n")
        file(APPEND ${AUTOCONFIG_SRC} "#cmakedefine HAVE_${name} 1\n")
        file(APPEND ${AUTOCONFIG_SRC} "\n")
    endforeach()
endfunction(check_includes)

########################################

file(WRITE ${AUTOCONFIG_SRC})

include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)
include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckPrototypeDefinition)
include(CheckStructHasMember)
include(CheckSymbolExists)
include(CheckTypeSize)
include(TestBigEndian)

set(include_files_list
    netinet/in.h
    pthread.h
    unistd.h
)
check_includes(include_files_list)

if (GETTEXT_FOUND)
    set(ENABLE_NLS 1)
endif()

file(APPEND ${AUTOCONFIG_SRC} "
////////////////////////////////////////////////////////////////////////////////

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
#cmakedefine ENABLE_NLS 1

/* pthread library */
#define HAVE_LIBPTHREAD 1

////////////////////////////////////////////////////////////////////////////////

/* Name of package */
#define PACKAGE \"flex\"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT \"flex-help@lists.sourceforge.net\"

/* Define to the full name of this package. */
#define PACKAGE_NAME \"the fast lexical analyser generator\"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING \"the fast lexical analyser generator @VERSION_PLAIN@\"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME \"flex\"

/* Define to the home page for this package. */
#define PACKAGE_URL \"\"

/* Define to the version of this package. */
#define PACKAGE_VERSION \"@VERSION_PLAIN@\"

/* Version number of package */
#define FLEX_VERSION \"@VERSION_PLAIN@\"
")

########################################

################################################################################
