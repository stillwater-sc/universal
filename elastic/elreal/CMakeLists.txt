# elastic/elreal/CMakeLists.txt
#
# Phase 1 (#925): block<FpType> representation.
# Phase 2 (#926): ZBCL<FpType> co-list of blocks.
# Phase 3 (#927): block-level EFTs (twoSum / twoMult / twoDiv).
# Phase 4 (#928): threeAdd helper + add() lazy ZBCL combinator.
# Phase 5 (#929): infinite summation (series<FpType> co-list + sum()).
# Phase 6 (#930): negation/mul/div.
# Phase 7 (#931): math suite (7.2 sqrt/hypot, ...).
# Phase 8+ (#932-#933): real-FP conversion, evaluation.

file (GLOB API_SRCS         "./api/*.cpp")
file (GLOB BLOCK_SRCS       "./block/*.cpp")
file (GLOB ZBCL_SRCS        "./zbcl/*.cpp")
file (GLOB BLOCK_EFT_SRCS   "./block_eft/*.cpp")
file (GLOB CONVERSION_SRCS  "./conversion/*.cpp")
file (GLOB LOGIC_SRCS       "./logic/*.cpp")
file (GLOB ARITHMETIC_SRCS  "./arithmetic/*.cpp")
file (GLOB SUMMATION_SRCS   "./summation/*.cpp")
file (GLOB MATH_SRCS        "./math/*.cpp")
file (GLOB ROUNDING_SRCS    "./rounding/*.cpp")
file (GLOB ORACLE_SRCS      "./oracle/*.cpp")

compile_all("true" "el_api"        "Number Systems/elastic/floating-point/binary/elreal/api"        "${API_SRCS}")
compile_all("true" "el_block"      "Number Systems/elastic/floating-point/binary/elreal/block"      "${BLOCK_SRCS}")
compile_all("true" "el_zbcl"       "Number Systems/elastic/floating-point/binary/elreal/zbcl"       "${ZBCL_SRCS}")
compile_all("true" "el_block_eft"  "Number Systems/elastic/floating-point/binary/elreal/block_eft"  "${BLOCK_EFT_SRCS}")
compile_all("true" "el_conversion" "Number Systems/elastic/floating-point/binary/elreal/conversion" "${CONVERSION_SRCS}")
compile_all("true" "el_logic"      "Number Systems/elastic/floating-point/binary/elreal/logic"      "${LOGIC_SRCS}")
compile_all("true" "el_arith"      "Number Systems/elastic/floating-point/binary/elreal/arithmetic" "${ARITHMETIC_SRCS}")
compile_all("true" "el_sum"        "Number Systems/elastic/floating-point/binary/elreal/summation"  "${SUMMATION_SRCS}")
compile_all("true" "el_math"       "Number Systems/elastic/floating-point/binary/elreal/math"       "${MATH_SRCS}")
compile_all("true" "el_round"      "Number Systems/elastic/floating-point/binary/elreal/rounding"   "${ROUNDING_SRCS}")
compile_all("true" "el_oracle"     "Number Systems/elastic/floating-point/binary/elreal/oracle"     "${ORACLE_SRCS}")
