# Copyright (C) 2017 Stillwater Supercomputing, Inc.
# SPDX-License-Identifier: MIT

file (GLOB SOURCES "./*.cpp")

foreach (source ${SOURCES})
    get_filename_component (util ${source} NAME_WE)
    string(REPLACE " " ";" new_source ${source})
    add_executable (${util} ${new_source})
    install(TARGETS ${util} DESTINATION bin)

    # visual organization for VS
    set_target_properties(${util} PROPERTIES FOLDER "Tools/Utilities")

    if (UNIVERSAL_CMAKE_TRACE)
        message(STATUS "testing: ${util} ${RUNTIME_OUTPUT_DIRECTORY}/${util}")
    endif()
    #message(STATUS "Add test ${util} from source ${new_source}")
    add_test(${util} ${RUNTIME_OUTPUT_DIRECTORY}/${util})
endforeach (source)
