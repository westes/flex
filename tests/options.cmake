#
# flex
#

message(STATUS "${EXE}")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E echo %% | "${EXE}" -cn
    RESULT_VARIABLE r
)

if (NOT ${r} EQUAL 0)
    message(FATAL_ERROR "Error: ${r}")
endif()
