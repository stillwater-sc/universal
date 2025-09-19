set(INTEGER_SOURCES
    blockbinary.cpp
)
set(FP_SOURCES
    blockfraction.cpp
    blocksignificand.cpp
)
set(FP_SUBTYPE_SOURCES
    blocktriple.cpp
)
set(PERF_SOURCES
    multi_limb_performance.cpp
)

compile_all("true" "demo" "Education/Internal/Multi-Limb/integer" "${INTEGER_SOURCES}")
compile_all("true" "demo" "Education/Internal/Multi-Limb/floating-point" "${FP_SOURCES}")
#compile_all("true" "demo" "Education/Internal/Multi-Limb/fp subtype" "${FP_SUBTYPE_SOURCES}")
#compile_all("true" "demo" "Education/Internal/Multi-Limb/performance" "${PERF_SOURCES}")
