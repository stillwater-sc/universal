file(GLOB API_SRC "./api/*.cpp")
file(GLOB ARITHMETIC_SRC "./arithmetic/*.cpp")
file(GLOB GROWTH_SRC "./growth/*.cpp")
file(GLOB PERFORMANCE_SRC "./performance/*.cpp")

compile_all("true" "exp_api" "Internal/multi-component/expansion/api" "${API_SRC}")
compile_all("true" "exp_arith" "Internal/multi-component/expansion/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "exp_growth" "Internal/multi-component/expansion/growth" "${GROWTH_SRC}")
compile_all("true" "exp_perf" "Internal/multi-component/expansion/performance" "${PERFORMANCE_SRC}")
