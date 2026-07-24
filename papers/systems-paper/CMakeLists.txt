# The systems-paper demos (iterative_refinement, conjugate_gradient, idrs) all
# depend on the extracted <blas/...> + <numeric/containers/...> stack, so the
# whole directory is dropped from the build (universal#1204, #1209). Sources are
# left in place for reference; rebuild them against MTL5 in the mp-* repos.
#
# file (GLOB SOURCES "./*.cpp")
# compile_all("true" "paper" "Papers/systems-paper" "${SOURCES}")
