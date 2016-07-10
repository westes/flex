#
# flex
#

if (OUT)
    execute_process(
        COMMAND "${EXE}" "${ARGS}"
        INPUT_FILE "${TXT}"
        OUTPUT_FILE ${OUT}
        RESULT_VARIABLE r
    )
else()
    execute_process(
        COMMAND "${EXE}" "${ARGS}"
        INPUT_FILE "${TXT}"
        RESULT_VARIABLE r
    )
endif()

if (NOT ${r} EQUAL 0)
    message(FATAL_ERROR "Error: ${r}")
endif()
