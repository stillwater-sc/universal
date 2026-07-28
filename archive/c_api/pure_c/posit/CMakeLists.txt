file (GLOB SOURCES "./*.c")

add_library(posit_c_api_pure STATIC ${SOURCES})
set_target_properties(posit_c_api_pure PROPERTIES FOLDER "Libraries")

if(C_API_LIB_PIC)
  if(CMAKE_COMPILER_IS_GNUCXX OR MINGW OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  endif()
endif()

install(TARGETS posit_c_api_pure DESTINATION lib)
#install(FILES posit_c_api.h      DESTINATION include)
