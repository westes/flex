#
# flex
#

execute_process(
    COMMAND "${EXE}" -o ${BDIR}/${FN}.c ${FN}.l
    OUTPUT_VARIABLE o
    ERROR_VARIABLE e
    RESULT_VARIABLE r
)

if (NOT ${r} EQUAL 0 AND NOT FAIL)
    message(FATAL_ERROR "Error: ${r}")
endif()

if ("${e}" STREQUAL "nodefault_warn.l:7: warning, -s option given but default rule can be matched\n")
    message(FATAL_ERROR "Error: ${e}")
endif()
