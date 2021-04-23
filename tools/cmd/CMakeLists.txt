file (GLOB SOURCES "./*.cpp")

foreach (source ${SOURCES})
    get_filename_component (cmd ${source} NAME_WE)
    string(REPLACE " " ";" new_source ${source})
    add_executable (${cmd} ${new_source})
    install(TARGETS ${cmd} DESTINATION bin)

    # visual organization for VS
    set_target_properties(${cmd} PROPERTIES FOLDER "Command Line Utilities")

    if (UNIVERSAL_CMAKE_TRACE)
        message(STATUS "testing: ${cmd} ${RUNTIME_OUTPUT_DIRECTORY}/${cmd}")
    endif()
    #message(STATUS "Add test ${cmd} from source ${new_source}")
    add_test(${cmd} ${RUNTIME_OUTPUT_DIRECTORY}/${cmd})
endforeach (source)
