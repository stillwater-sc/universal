# elastic/elreal/CMakeLists.txt
#
# Phase 1 (#925): block<FpType> representation only. Co-list, arithmetic,
# math suite and conversion are added in subsequent phases (#926-#933).

file (GLOB BLOCK_SRCS "./block/*.cpp")

compile_all("true" "el_block" "Number Systems/elastic/floating-point/binary/elreal/block" "${BLOCK_SRCS}")
