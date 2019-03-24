add_library(posit_c_api STATIC posit_c_api.cpp)
install(TARGETS posit_c_api DESTINATION lib)
install(FILES posit_c_api.h DESTINATION include)
