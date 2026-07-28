file (GLOB SOURCES "./*.c*")

####
# macro to read all source files in a directory
# and create a test target for each source file
macro (compile_and_link_all testing prefix folder)
    # cycle through the sources
    # For the according directories, we assume that each cpp file is a separate test
    # so, create a executable target and an associated test target
    foreach (source ${ARGN})
        get_filename_component (test ${source} NAME_WE)
        string(REPLACE " " ";" new_source ${source})
        set(test_name ${prefix}_${test})
        #message(STATUS "Add test ${test_name} from source ${new_source}.")
        add_executable (${test_name} ${new_source})
        set_target_properties(${test_name} PROPERTIES FOLDER ${folder})
        target_link_libraries(${test_name} posit_c_api_pure)
        if (UNIX)
            target_link_libraries(${test_name} m)
        endif(UNIX)
        if (${testing} STREQUAL "true")
            if (UNIVERSAL_CMAKE_TRACE)
                message(STATUS "testing: ${test_name}")
            endif()
            add_test(NAME ${test_name} COMMAND ${test_name})
        endif()
    endforeach (source)
endmacro (compile_and_link_all)

compile_and_link_all("true" "c_api_pure" "Shims/C API" "${SOURCES}")
