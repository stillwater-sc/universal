file (GLOB SOURCES "./*.c")

add_library(posit_c_api STATIC ${SOURCES})

if(C_API_LIB_PIC)
  if(CMAKE_COMPILER_IS_GNUCXX OR MINGW OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  endif()
endif()

install(TARGETS posit_c_api DESTINATION lib)
install(FILES posit_c_api.h DESTINATION include)
