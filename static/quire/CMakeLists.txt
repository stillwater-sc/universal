file (GLOB API_SRC "./api/*.cpp")
file (GLOB ARITHMETIC_SRC "./arithmetic/*.cpp")

compile_all("true" "quire" "Number Systems/static/quire/api" "${API_SRC}")
compile_all("true" "quire" "Number Systems/static/quire/arithmetic" "${ARITHMETIC_SRC}")
