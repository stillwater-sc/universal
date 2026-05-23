# elastic/elreal/CMakeLists.txt
#
# Phase 1 (#925): block<FpType> representation.
# Phase 2 (#926): ZBCL<FpType> co-list of blocks.
# Phase 3 (#927): block-level EFTs (twoSum / twoMult / twoDiv).
# Phase 4 (#928): threeAdd helper + add() lazy ZBCL combinator.
# Phase 5+ (#929-#933): infinite summation, math suite, real-FP conversion.

file (GLOB BLOCK_SRCS       "./block/*.cpp")
file (GLOB ZBCL_SRCS        "./zbcl/*.cpp")
file (GLOB BLOCK_EFT_SRCS   "./block_eft/*.cpp")
file (GLOB ARITHMETIC_SRCS  "./arithmetic/*.cpp")

compile_all("true" "el_block"      "Number Systems/elastic/floating-point/binary/elreal/block"      "${BLOCK_SRCS}")
compile_all("true" "el_zbcl"       "Number Systems/elastic/floating-point/binary/elreal/zbcl"       "${ZBCL_SRCS}")
compile_all("true" "el_block_eft"  "Number Systems/elastic/floating-point/binary/elreal/block_eft"  "${BLOCK_EFT_SRCS}")
compile_all("true" "el_arith"      "Number Systems/elastic/floating-point/binary/elreal/arithmetic" "${ARITHMETIC_SRCS}")
