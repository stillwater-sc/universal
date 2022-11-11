file (GLOB API_SRC "./api/*.cpp")

compile_all("true" "gfp" "Internal/fast/gfp/api" "${API_SRC}")
