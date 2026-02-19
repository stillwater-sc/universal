#include <iostream>
#include <iomanip>

#include <universal/number/qd/qd.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

#include <math/functions/x_over_one_minus_x.hpp>
#include <universal/utility/error.hpp>


template<typename Real>
Real RoundTrip(Real x) {
    Real f_of_x = sw::function::x_over_one_minus_x(x);
    Real back_to_x = sw::function::x_over_one_plus_x(f_of_x);
    return back_to_x;
}

template<typename Real>
void RelativeErrorAt(Real x) {
	using namespace sw::universal;
	std::cout << type_tag(x) << " x = " << x << '\n';
	Real yinv = sw::function::x_over_one_plus_x(x);
	Real y = sw::function::x_over_one_minus_x(yinv);
	std::cout << "x    : " << color_print(x) << " : " << x << '\n';
	std::cout << "yinv : " << color_print(yinv) << " : " << yinv << '\n';
	std::cout << "y    : " << color_print(y) << " : " << y << '\n';

	std::cout << "RelativeError : " << RelativeError(double(y), double(x)) << '\n';
}


namespace sw {
	namespace universal {

		template<typename Posit, typename Cfloat, typename Lns>
		void CompareRelativeError(double da) {
			qd qa{ da };
			Posit pa{ da };
			Cfloat ca{ da };
			Lns la{ da };
			RelativeErrorAt(qa);
			RelativeErrorAt(pa);
			RelativeErrorAt(ca);
			RelativeErrorAt(la);
		}
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
	
    // The approach is to leverage the round trip identity of f(x) * f^-1(x). 
    // In regions where the function values are accurately represented
    // we expect the identity to hold.
    // In regions where the values are heavily approximated, the relative
    // error is expected to be significant.

	using Posit = posit<32, 2>;
	using Cfloat = cfloat<32, 8, std::uint32_t, true, false, false>; // an IEEE 754 32-bit float
	using Lns = lns<32, 24>;

	// evaluate the function across the domain (0, 2]
	const int nrSamples = 25;
	constexpr double x_min = 0.99;
	constexpr double x_max = 1.01;
	constexpr double x_step = (x_max - x_min) / nrSamples;
	double x = x_min;
	qd qa(x_step), qb;
	Posit pa(x_step), p_step(x_step), pb;
	Cfloat ca(x_step), c_step(x_step), cb;
	Lns la(x_step), l_step(x_step), lb;
	constexpr int WIDTH = 25;
	std::cout << "Relative error of F(F^-1(x)) = x for different number systems" << std::endl;
	std::cout << std::setw(WIDTH) << x
		<< std::setw(WIDTH) << "F(F^-1(x))"
		<< std::setw(WIDTH) << "quad"
		<< std::setw(WIDTH) << "posit<32,2>"
		<< std::setw(WIDTH) << "cfloat<32,8>"
		<< std::setw(WIDTH) << "lns<32,24>"
		<< std::endl;
	for (int i = 0; i < nrSamples; ++i) {
		double y = RoundTrip(x); 
		qb = RoundTrip(qa);
		pb = RoundTrip(pa); 
		cb = RoundTrip(ca);
		lb = RoundTrip(la);
		std::cout << std::setprecision(8)
			<< std::setw(WIDTH) << x
			<< std::setw(WIDTH) << y
			<< std::setw(WIDTH) << RelativeError(qa, qb)
			<< std::setw(WIDTH) << RelativeError(double(pb), x)
			<< std::setw(WIDTH) << RelativeError(double(cb), x)
			<< std::setw(WIDTH) << RelativeError(double(lb), x)
			<< std::endl;

		x += x_step;
		pa += p_step;
		ca += c_step;
		la += l_step;
	}

	{
		// pick an interesting value to manually trace
		std::cout << "Manually trace a value\n";
		double x = 1.0 - 1.0e-6;
		qd qa(x);
		Posit pa(x);
		Cfloat ca(x);
		Lns la(x);
		double y = RoundTrip(x);
		qd qb = RoundTrip(qa);
		Posit pb = RoundTrip(pa);
		Cfloat cb = RoundTrip(ca);
		Lns lb = RoundTrip(la);
		std::cout << std::setprecision(25)
			<< to_binary(x) << " : " << x << '\n'
			<< to_binary(y) << " : " << y << '\n';
		std::cout << std::setprecision(15)
			<< std::setw(WIDTH) << x
			<< std::setw(WIDTH) << y
			<< std::setw(WIDTH) << RelativeError(qa, qb)
			<< std::setw(WIDTH) << RelativeError(double(pb), x)
			<< std::setw(WIDTH) << RelativeError(double(cb), x)
			<< std::setw(WIDTH) << RelativeError(double(lb), x)
			<< '\n';
		std::cout << std::setprecision(15)
			<< std::setw(WIDTH) << x
			<< std::setw(WIDTH) << y
			<< std::setw(WIDTH) << RelativeError(qa, qb)
			<< std::setw(WIDTH) << RelativeError(qd(double(pb)), qb)
			<< std::setw(WIDTH) << RelativeError(qd(double(cb)), qb)
			<< std::setw(WIDTH) << RelativeError(qd(double(lb)), qb)
			<< std::endl;
	}

    // we can generate the value of x that causes the range values to cycle through the tapered regions
    // by simply taking the inverse of the function at that value.
    // For example, if we take a 32bit posit with just 5 mantissa bits we have a low precision real

	{
		// I want the function y1 = x / (1 - x) to yield a value that is in the tapered region
		// of the posits. I can generate the required x value by simply picking a value in the 
		// tapered region and taking the inverse function y2 = x / (1 + x) at that value.
		double y1 = 2.0e13;
		double y2 = sw::function::x_over_one_plus_x(y1);
		std::cout << "y1 = " << y1 << '\n';
		std::cout << "y2 = " << y2 << '\n';  // this should be close to 1.0
	}

	{
		// create a value that resides in a low precision region of the posit
		//pa.setbits(0b0111'1111'1111'1111'1111'1111'1000'1111);
		//pa.setbits(0b0111'1111'1111'1111'1111'1000'0000'1111);
		//pa.setbits(0b0111'1111'1111'1111'1000'0000'0000'1111);
		//pa.setbits(0b0111'1111'1111'1000'0000'0000'0000'1111);
		//pa.setbits(0b0111'1111'1000'0000'0000'0000'0000'1111);
		//pa.setbits(0b0111'1000'0000'0000'0000'0000'0000'1111);
		// take the inverse of the function at that value
		// to generate an input that will cycle the identity equation y * yinv
		// through the low precision region

		double da;

		pa.setbits(0b0111'1110'0000'0000'0000'0000'0000'1111);
		da = double(pa);
		CompareRelativeError<Posit, Cfloat, Lns>(da);

		pa.setbits(0b0111'1111'1000'0000'0000'0000'0000'1111);
		da = double(pa);
		CompareRelativeError<Posit, Cfloat, Lns>(da);
	}

    return EXIT_SUCCESS;
}
catch (...) {
    std::cerr << "Caught unexpected exception" << std::endl;
    return EXIT_FAILURE;
}
