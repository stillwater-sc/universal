#include <iostream>
#include <iomanip>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

#include <math/functions/x_over_one_minus_x.hpp>
#include <universal/utility/error.hpp>


int main()
try {

	// measure the error propagation of the function x / (1 - x)
	using namespace sw::universal;
    using namespace sw::function;

	// the function x / (1 - x) is going to infinity as x approaches 1
    // When we use tapered number systems, such as posits or lns, the
	// error increased dramatically when we approach the limit.
	// We would like to see how the error propagates through the function.

	// evaluate the function across the domain (0, 2]
	const int nrSamples = 27;
	constexpr double x_min = 1.0e-4;
	constexpr double x_max = 2.0;
	constexpr double x_step = (x_max - x_min) / nrSamples;
	double x = x_min;
	posit<32, 2> pa(x_step), p_step(x_step), pb;
	cfloat<32, 8> ca(x_step), c_step(x_step), cb;
	lns<32, 24> la(x_step), l_step(x_step), lb;
	constexpr int WIDTH = 25;
	std::cout << "Relative error of x / (1 - x) for different number systems" << std::endl;
	std::cout << std::setw(WIDTH) << x
		<< std::setw(WIDTH) << "double"
		<< std::setw(WIDTH) << "posit<32,2>"
		<< std::setw(WIDTH) << "cfloat<32,8>"
		<< std::setw(WIDTH) << "lns<32,24>"
		<< std::endl;
	for (int i = 0; i < nrSamples; ++i) {
		double y = x_over_one_minus_x(x); x += x_step;
		pb = x_over_one_minus_x(pa); pa += p_step;
		cb = x_over_one_minus_x(ca); ca += c_step;
		lb = x_over_one_minus_x(la); la += l_step;
		std::cout << std::setprecision(8)
			<< std::setw(WIDTH) << x
			<< std::setw(WIDTH) << y
			<< std::setw(WIDTH) << RelativeError(double(pb), y)
			<< std::setw(WIDTH) << RelativeError(double(cb), y)
			<< std::setw(WIDTH) << RelativeError(double(lb), y)
			<< std::endl;
	}

    return EXIT_SUCCESS;
}
catch (...) {
    std::cerr << "Caught unexpected exception" << std::endl;
    return EXIT_FAILURE;
}
