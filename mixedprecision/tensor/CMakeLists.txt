file (GLOB KRYLOV_SRCS "./cg/*.cpp")
file (GLOB DNN_SRCS "./dnn/*.cpp")

compile_all("true" "mp" "Mixed-Precision/Tensor Algebra/Krylov/Conjugate Gradient" "${KRYLOV_SRCS}")
compile_all("true" "mp" "Mixed-Precision/Tensor Algebra/DNN" "${DNN_SRCS}")
