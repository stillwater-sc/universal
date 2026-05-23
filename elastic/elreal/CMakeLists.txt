# elastic/elreal/CMakeLists.txt
#
# Phase 1 (#925): block<FpType> representation.
# Phase 2 (#926): ZBCL<FpType> co-list of blocks.
# Phase 3+ (#927-#933): arithmetic, math suite, real-FP conversion.

file (GLOB BLOCK_SRCS "./block/*.cpp")
file (GLOB ZBCL_SRCS  "./zbcl/*.cpp")

compile_all("true" "el_block" "Number Systems/elastic/floating-point/binary/elreal/block" "${BLOCK_SRCS}")
compile_all("true" "el_zbcl"  "Number Systems/elastic/floating-point/binary/elreal/zbcl"  "${ZBCL_SRCS}")
