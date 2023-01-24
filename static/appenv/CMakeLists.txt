file (GLOB MULTIFILE_SOURCES "./multifile/*.cpp")

compile_multifile_target("true" "multifile" "Applications/Environment/Multifile" ${MULTIFILE_SOURCES})
