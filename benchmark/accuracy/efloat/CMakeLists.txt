# efloat against MPFR, bit for bit in all four rounding modes (#1782): MPFR is a dependency of this
# check only, so it is built when mpfr.h, libmpfr and libgmp are found on a native build, and
# skipped otherwise. The ctest is the quick run, every function up to 512 bits; run the program
# with no arguments for the full one, up to 8192 bits.
if(NOT CMAKE_CROSSCOMPILING)
  find_path(UNIVERSAL_MPFR_INCLUDE_DIR mpfr.h)
  find_library(UNIVERSAL_MPFR_LIBRARY mpfr)
  find_library(UNIVERSAL_GMP_LIBRARY gmp)
  if(UNIVERSAL_MPFR_INCLUDE_DIR AND UNIVERSAL_MPFR_LIBRARY AND UNIVERSAL_GMP_LIBRARY)
    add_executable(accuracy_efloat_mpfr_check ./mpfr_check.cpp)
    target_include_directories(accuracy_efloat_mpfr_check PRIVATE ${UNIVERSAL_MPFR_INCLUDE_DIR})
    target_link_libraries(accuracy_efloat_mpfr_check PRIVATE ${UNIVERSAL_MPFR_LIBRARY} ${UNIVERSAL_GMP_LIBRARY})
    set_target_properties(accuracy_efloat_mpfr_check PROPERTIES FOLDER "Benchmarks/Accuracy/efloat")
    add_test(NAME accuracy_efloat_mpfr_check COMMAND accuracy_efloat_mpfr_check 0.1 all 512)
  else()
    message(STATUS "MPFR not found: accuracy_efloat_mpfr_check is skipped (Debian/Ubuntu: libmpfr-dev)")
  endif()
endif()
