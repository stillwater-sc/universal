file (GLOB API_SRC "./api/*.cpp")

compile_all("true" "gfp" "Internal/limited/gfp/api" "${API_SRC}")
