file (GLOB API_SRC "./api/*.cpp")

compile_all("true" "cm" "Internal/constexpr_math/api" "${API_SRC}")
