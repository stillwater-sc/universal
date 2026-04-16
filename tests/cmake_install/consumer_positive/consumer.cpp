#include <iostream>
#include <universal/number/posit/posit.hpp>

// A C++20 concept: this translation unit cannot compile unless the
// compiler is on C++20. The install-smoke test relies on Universal's
// imported target propagating cxx_std_20 to the consumer; if the
// propagation breaks, the build fails here at the concept definition.
template <typename T>
concept IsArithmetic = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

template <IsArithmetic T>
T add(const T& a, const T& b) { return a + b; }

int main() {
    using posit32 = sw::universal::posit<32, 2>;
    posit32 a{1.5}, b{2.25};
    std::cout << "posit32 sum = " << add(a, b) << '\n';
    return 0;
}
