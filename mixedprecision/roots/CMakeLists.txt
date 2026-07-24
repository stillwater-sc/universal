#file (GLOB SOURCES "./*.cpp")

set(SECANT_SRC "./secant.cpp")

compile_all("true" "mp" "Mixed-Precision/Root Finding" "${SECANT_SRC}")

# rpoly (Jenkins-Traub) depended on the extracted <blas/...> stack; removed here
# and re-homed to mp-numerics (universal#1204, #1209; follow-up #1211).

