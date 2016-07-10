if (NOT GETTEXT_FOUND)
    return()
endif()

set(sdir ${CMAKE_SOURCE_DIR}/src)
set(src_list
    ${sdir}/ccl.cpp
    ${sdir}/dfa.cpp
    ${sdir}/ecs.cpp
    ${sdir}/gen.cpp
    ${sdir}/main.cpp
    ${sdir}/misc.cpp
    ${sdir}/nfa.cpp
    ${sdir}/options.cpp
    ${sdir}/scanflags.cpp
    ${sdir}/scanopt.cpp
    ${sdir}/sym.cpp
    ${sdir}/tables.cpp
    ${sdir}/tables_shared.cpp
    ${sdir}/tblcmp.cpp
    ${sdir}/yylex.cpp
    ${sdir}/flex.skl
    ${sdir}/parse.y
    ${sdir}/scan.l
)

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/)

set(pot_file ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pot)
add_custom_command(OUTPUT ${pot_file}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/
    COMMAND chmod a+w ${CMAKE_CURRENT_BINARY_DIR}/*.po
    COMMAND xgettext --keyword=_ --keyword=N_ -o ${pot_file} ${src_list}
    DEPENDS ${src_list}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Extract translatable messages to ${pot_file}"
)

GETTEXT_PROCESS_POT_FILE(${pot_file}
    INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/share/locale/"
    LANGUAGES
        ca
        da
        de
        #en@quot
        #en@boldquot
        eo
        es
        fi
        fr
        ga
        hr
        ko
        nl
        pl
        pt_BR
        ro
        ru
        sr
        sv
        tr
        vi
        zh_CN
        zh_TW
)

#MSGID_BUGS_ADDRESS = flex-devel@lists.sourceforge.net

# getpo:
#	rsync -Ltvz  translationproject.org::tp/latest/flex/*.po $(top_srcdir)/po

