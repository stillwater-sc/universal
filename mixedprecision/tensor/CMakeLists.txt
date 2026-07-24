# CG mixed-precision study (./cg/*.cpp): REMOVED FROM THE BUILD (universal#1204,
# Phase 4). The pure variants (mv-dot x cmp-dot, mv-fdp x cmp-fdp) are migrated to
# mp-iterative (applications/krylov/cg_precision). The mixed variants
# (mv-dot x cmp-fdp, mv-fdp x cmp-dot) will be redesigned in mp-iterative with a
# separate matvec/dot accumulator. The ./cg/ sources are intentionally LEFT IN
# PLACE for reference during that redesign, but are no longer compiled here.
#
# file (GLOB KRYLOV_SRCS "./cg/*.cpp")
# compile_all("true" "mp" "Mixed-Precision/Tensor Algebra/Krylov/Conjugate Gradient" "${KRYLOV_SRCS}")

file (GLOB DNN_SRCS "./dnn/*.cpp")
compile_all("true" "mp" "Mixed-Precision/Tensor Algebra/DNN" "${DNN_SRCS}")
