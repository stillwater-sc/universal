add_library(posit_c_api_shim STATIC posit_c_api.cpp)
set_target_properties(posit_c_api_shim PROPERTIES FOLDER "Libraries")

if(C_API_LIB_PIC)
  if(CMAKE_COMPILER_IS_GNUCXX OR MINGW OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  endif()
endif()

install(TARGETS posit_c_api_shim DESTINATION lib)
#install(FILES posit_c_api.h      DESTINATION include)

# TODO: we might not have the same API for the shim as for the pure C api as the C++ shim can support
# arbitrary precision posits and the pure C api will only support standard posits (8/16/32/64/128/256)
# There has already been a demand for intermediate posits that fit in byte-boundaries, such as 24, 40, 48, 56,
# and they are likely going to be needed in specialized, high-performance configurations for G+SMO, DPLASMA,
# MTL4, MTL5, etc.
# We may need to revisit this design when these specializations get productized.
