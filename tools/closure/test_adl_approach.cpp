// test_adl_approach.cpp: test that ADL works for integer types
#include <iostream>
#include <universal/number/integer/integer.hpp>
#include <universal/utility/closure_plot_png.hpp>

int main() {
    using namespace sw::universal;

    std::cout << "Testing ADL approach for integer types..." << std::endl;

    // Test ADL functions work
    integer<8> i(42);

    // These should all compile and work through ADL
    bool nan_result = sw::universal::isnan(i);
    bool inf_result = sw::universal::isinf(i);
    bool normal_result = sw::universal::isnormal(i);

    std::cout << "isnan(42): " << nan_result << std::endl;
    std::cout << "isinf(42): " << inf_result << std::endl;
    std::cout << "isnormal(42): " << normal_result << std::endl;

    // Test that the closure plot template instantiates
    ClosurePlotPNG<integer<8>> generator;
    std::cout << "Template instantiation successful!" << std::endl;

    return 0;
}