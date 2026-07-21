file (GLOB SOURCES "./*.cpp")

# "false": build the characterization tool but do not register it as a ctest --
# it is a measurement utility (its output is data, not pass/fail) and a full
# sweep of the elreal transcendentals takes seconds.
compile_all("false" "accuracy" "Benchmarks/Accuracy/adaptive" "${SOURCES}")
