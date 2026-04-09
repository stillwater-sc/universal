file (GLOB API_SRC         "api/*.cpp")
file (GLOB CONVERSION_SRC  "conversion/*.cpp")

compile_all("true" "bisection" "Number Systems/static/bisection/api" "${API_SRC}")
compile_all("true" "bisection" "Number Systems/static/bisection/conversion" "${CONVERSION_SRC}")
