file (GLOB KRYLOV_SRCS "./cg/*.cpp")
file (GLOB DNN_SRCS "./dnn/*.cpp")

compile_all("true" "mp" "Mixed-Precision/Linear Algebra/Krylov/Conjugate Gradient" "${KRYLOV_SRCS}")
compile_all("true" "mp" "Mixed-Precision/Linear Algebra/DNN" "${DNN_SRCS}")
