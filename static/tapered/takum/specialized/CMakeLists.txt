file (GLOB SOURCES "./*.cpp")
find_package(Threads REQUIRED)

compile_all("true" "tkm" "Number Systems/static/floating-point/tapered/takum/specialized" "${SOURCES}")
target_link_libraries(tkm_takum_16_3 Threads::Threads)

# opt-in exhaustive verification (all 2^32 operand pairs per operator): make tkm_takum_16_3_exhaustive
add_executable(tkm_takum_16_3_exhaustive EXCLUDE_FROM_ALL takum_16_3.cpp)
target_compile_definitions(tkm_takum_16_3_exhaustive PRIVATE TAKUM16_EXHAUSTIVE)
target_link_libraries(tkm_takum_16_3_exhaustive Threads::Threads)
set_target_properties(tkm_takum_16_3_exhaustive PROPERTIES FOLDER "Number Systems/static/floating-point/tapered/takum/specialized")
