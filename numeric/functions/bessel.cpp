#include <iostream>
#include <iomanip>
#include <universal/verification/test_suite.hpp>
#include <universal/number/posit/posit.hpp>
#include <math/functions/bessel.hpp>
#include <math/functions.hpp>

template<typename Scalar>
void TestBesselFunctions(const std::string& tag, Scalar x, unsigned int max_order = 5) {
    using namespace sw::universal;
	using namespace sw::math::function;

    std::cout << "\nTesting Bessel functions with " << tag << " at x = " << x << "\n";

	// Bessel functions of the first kind J_n(x)
    Scalar j0 = bessel_j0<Scalar>(x);
    std::cout << "  J_" << 0 << "(x) = " << j0 << "\n";
    for (unsigned int n = 0; n <= max_order; ++n) {
        Scalar jn = bessel_j<Scalar>(n, x);
        std::cout << "  J_" << n << "(x) = " << jn << "\n";
	}

	// Bessel functions of the second kind Y_n(x)
	// and Modified Bessel functions of the first kind I_n(x)
	// and Modified Bessel functions of the second kind K_n(x)
    // 
    // Base functions
    Scalar i0 = bessel_i0<Scalar>(x);
    Scalar i1 = bessel_i1<Scalar>(x);
    Scalar k0 = bessel_k0<Scalar>(x);
    Scalar k1 = bessel_k1<Scalar>(x);
    Scalar y0 = bessel_y0<Scalar>(x);
    Scalar y1 = bessel_y1<Scalar>(x);

    std::cout << "  I_0(x) = " << i0 << "\n";
    std::cout << "  I_1(x) = " << i1 << "\n";
    std::cout << "  K_0(x) = " << k0 << "\n";
    std::cout << "  K_1(x) = " << k1 << "\n";
    std::cout << "  Y_0(x) = " << y0 << "\n";
    std::cout << "  Y_1(x) = " << y1 << "\n";

    // Recurrence-based values
    for (unsigned int n = 2; n <= max_order; ++n) {
        Scalar in = bessel_i<Scalar>(n, x);
        Scalar kn = bessel_k<Scalar>(n, x);
        Scalar yn = bessel_y<Scalar>(n, x);

        std::cout << "  I_" << n << "(x) = " << in << "\n";
        std::cout << "  K_" << n << "(x) = " << kn << "\n";
        std::cout << "  Y_" << n << "(x) = " << yn << "\n";
    }
}

int main() {
    using namespace sw::universal;

    try {
        constexpr double x = 3.0;

        // Test with float
        TestBesselFunctions<float>("float", x);

        // Test with double
        TestBesselFunctions<double>("double", x);

        // Test with posit<32,2>
        using Posit = posit<32, 2>;
        TestBesselFunctions<Posit>("posit<32,2>", Posit(x));

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}