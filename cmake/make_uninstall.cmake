
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt")
    message(FATAL_ERROR "Cannot find install manifest: ${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt")
endif()

file(READ "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt" files)
string(REGEX REPLACE "[\r\n]" ";" files "${files}")

foreach(file ${files})
    message(STATUS "Uninstalling ${file}")
    if(EXISTS "${file}")
        file(REMOVE ${file})
        if (EXISTS "${file}")
            message(FATAL_ERROR "Problem when removing ${file}, please check your permissions")
        endif()
    else()
        message(STATUS "File ${file} does not exist.")
    endif()
endforeach()
