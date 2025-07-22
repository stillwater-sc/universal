#include <iostream>
#include <iomanip>

#include <cmath>
#include <universal/number/qd/qd.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

#include <math/functions/x_over_one_minus_x.hpp>
#include <universal/utility/error.hpp>

template<typename Posit, typename Cfloat, typename Lns>
void ScanRelativeError(double scale = 10.0) {
	using namespace sw::universal;

	std::cout << "RelativeError of the ULP across scales that taper\n";
	constexpr int WIDTH = 25;

	float base = 1.0f;
	float ulp = std::nextafter(base, 2 * base);  // take the ulp of single precision float
	Posit pb{ ulp };
	Cfloat cb{ ulp };
	Lns lb{ ulp };
	// print the header of the table
	std::cout << std::setw(36) << "base"
		<< std::setw(36) << "ulp"
		<< std::setw(WIDTH) << "ulp value"
		<< std::setw(WIDTH) << "float"
		<< std::setw(WIDTH) << "posit"
		<< std::setw(WIDTH) << "cfloat"
		<< std::setw(WIDTH) << "lns"
		<< '\n';
	for (int i = 0; i < 24; ++i) {
		// the base is the reference we want to measure the ulp against in terms of relative error
		double fa = RelativeError(double(ulp), double(base));  // should be 0 as we can represent the ulp exactly
		double pa = RelativeError(double(pb), double(base));
		double ca = RelativeError(double(cb), double(base));
		double la = RelativeError(double(lb), double(base));
		std::cout << to_binary(base) << " " << to_binary(ulp)
			<< std::setw(WIDTH) << ulp
			<< std::setw(WIDTH) << fa
			<< std::setw(WIDTH) << pa
			<< std::setw(WIDTH) << ca
			<< std::setw(WIDTH) << la
			<< '\n';
		base *= scale;
		ulp = std::nextafter(base, 2 * base);
		pb = ulp;
		cb = ulp;
		lb = ulp;
	}
}

int main()
try {

    // measure the error propagation of the function x / (1 - x)
    using namespace sw::universal;
    using namespace sw::function;

    // the function x / (1 - x) is going to infinity as x approaches 1
    // When we use tapered number systems, such as posits or lns, the
    // error increased dramatically when we approach the limit.
    // We would like to see how the error propagates through the function.
	
    // The approach is to leverage the round trip identity of f^-1( f(x) ) => x. 
    // In regions where the function values are accurately represented
    // we expect the identity to hold.
    // In regions where the values are heavily approximated, the relative
    // error is expected to be significant.

	using Posit = posit<32, 2>;
	using Cfloat = cfloat<32, 8, std::uint32_t, true, false, false>; // an IEEE 754 32-bit float
	using Lns = lns<32, 24>;

	ScanRelativeError<Posit, Cfloat, Lns>(2.0);
	ScanRelativeError<Posit, Cfloat, Lns>(10.0);

    return EXIT_SUCCESS;
}
catch (...) {
    std::cerr << "Caught unexpected exception" << std::endl;
    return EXIT_FAILURE;
}
