#
# flex
#

if (WIN32)
    return()
endif()

execute_process(COMMAND date +%d\ %B\ %Y OUTPUT_VARIABLE my)
execute_process(COMMAND date +%B\ %Y OUTPUT_VARIABLE dmy)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/version.texi
"
@set UPDATED ${dmy}
@set UPDATED-MONTH ${my}
@set EDITION ${VERSION_PLAIN}
@set VERSION ${VERSION_PLAIN}
"
)

add_custom_target(info ALL
    COMMAND makeinfo ${CMAKE_CURRENT_SOURCE_DIR}/flex.texi
)

add_custom_target(man ALL
    COMMAND help2man --name='the fast lexical analyser generator' --section=1 $<TARGET_FILE:flexmain> > flex.1
    DEPENDS flexmain
)

add_custom_target(pdf
    COMMAND makeinfo --pdf -o flex.pdf ${CMAKE_CURRENT_SOURCE_DIR}/flex.texi
)

add_custom_target(html
    COMMAND makeinfo --html -o flex.html ${CMAKE_CURRENT_SOURCE_DIR}/flex.texi
)

