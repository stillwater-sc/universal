# Copyright (C) 2017 Stillwater Supercomputing, Inc.
# SPDX-License-Identifier: MIT

file (GLOB SOURCES "./*.cpp")

foreach (source ${SOURCES})
    get_filename_component (closure ${source} NAME_WE)
    string(REPLACE " " ";" new_source ${source})
    add_executable (${closure} ${new_source})
    install(TARGETS ${closure} DESTINATION bin)

    # visual organization for VS
    set_target_properties(${closure} PROPERTIES FOLDER "Tools/Closure")

    if (UNIVERSAL_CMAKE_TRACE)
        message(STATUS "testing: ${closure} ${RUNTIME_OUTPUT_DIRECTORY}/${closure}")
    endif()
    #message(STATUS "Add test ${closure} from source ${new_source}")
    add_test(${closure} ${RUNTIME_OUTPUT_DIRECTORY}/${closure})
endforeach (source)