set(INTEGER_SOURCES
    blockbinary_basics.cpp
)
set(FP_SOURCES
    blockfraction_usage.cpp
    blocksignificant_operations.cpp
)
set(FP_SUBTYPE_SOURCES
    blocktriple_arithmetic.cpp
)
set(PERF_SOURCES
    multi_limb_performance.cpp
)

compile_all("true" "internal" "Education/Internal/Multi-Limb Arithmetic/integer" "${INTEGER_SOURCES}")
#compile_all("true" "internal" "Education/Internal/Multi-Limb Arithmetic/fp components" "${FP_SOURCES}")
#compile_all("true" "internal" "Education/Internal/Multi-Limb Arithmetic/fp subtype" "${FP_SUBTYPE_SOURCES}")
#compile_all("true" "internal" "Education/Internal/Multi-Limb Arithmetic/performance" "${PERF_SOURCES}")
