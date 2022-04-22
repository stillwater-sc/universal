file (GLOB MULTIFILE_SOURCES "./multifile/*.cpp")

compile_multifile_target("true" "multifile" "Application/Environment/Multifile" "${MULTIFILE_SOURCES}")
