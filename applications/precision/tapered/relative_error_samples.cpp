#include <iostream>
#include <iomanip>

#include <cmath>
#include <universal/number/qd/qd.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

#include <universal/math/functions/x_over_one_minus_x.hpp>
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
	
    // The approach is to leverage the round trip identity of f^-1( f(x) ) => x. 
    // In regions where the function values are accurately represented
    // we expect the identity to hold.
    // In regions where the values are heavily approximated, the relative
    // error is expected to be significant.

	using Posit = posit<32, 2>;
	using Cfloat = cfloat<32, 8, std::uint32_t, true, false, false>; // an IEEE 754 32-bit float
	using Lns = lns<32, 24>;

	{
		constexpr int WIDTH = 25;
		// sample relative error of the ULP
		float base = 1.0e8;
		float ulp = std::nextafter(base, 2*base);
		std::cout << "starting base: " << to_binary(base) << " : " << base << '\n';
		std::cout << "ulp of base  : " << to_binary(ulp) << " : " << ulp << '\n';
		std::cout << "Relative error as a function of scale\n";
		std::cout << std::setw(36) << "base"
			<< std::setw(36) << "ulp"
			<< std::setw(WIDTH) << "base value"
			<< std::setw(WIDTH) << "float"
			<< std::setw(WIDTH) << "posit"
			<< std::setw(WIDTH) << "cfloat"
			<< std::setw(WIDTH) << "lns"
			<< '\n';
		for (int i = 0; i < 24; ++i) {
			float fa = RelativeError(ulp, base);
			Posit pa = RelativeError<Posit, Posit>(Posit(ulp), Posit(base));
//			std::cout << to_binary(Posit(base)) << " Posit(base)\n" << to_binary(Posit(ulp)) << " Posit(ulp)\n";

			Cfloat ca = RelativeError<Cfloat, Cfloat>(Cfloat(ulp), Cfloat(base));
			Lns la = RelativeError<Lns, Lns>(Lns(ulp), Lns(base));
			std::cout << to_binary(base) << " " << to_binary(ulp)
				<< std::setw(WIDTH) << base
				<< std::setw(WIDTH) << fa
				<< std::setw(WIDTH) << pa
				<< std::setw(WIDTH) << ca
				<< std::setw(WIDTH) << la
				<< '\n';
			base *= 2.0;
			ulp = std::nextafter(base, 2 * base);
		}
	}

	{
		std::cout << "Doing the RelativeError computation in double precision\n";
		constexpr int WIDTH = 25;
		// sample relative error of the ULP
		float base = 1.0e8;
		float ulp = std::nextafter(base, 2 * base);
		std::cout << "starting base: " << to_binary(base) << " : " << base << '\n';
		std::cout << "ulp of base  : " << to_binary(ulp) << " : " << ulp << '\n';
		std::cout << "Relative error as a function of scale\n";
		std::cout << std::setw(36) << "base"
			<< std::setw(36) << "ulp"
			<< std::setw(WIDTH) << "base value"
			<< std::setw(WIDTH) << "float"
			<< std::setw(WIDTH) << "posit"
			<< std::setw(WIDTH) << "cfloat"
			<< std::setw(WIDTH) << "lns"
			<< '\n';
		for (int i = 0; i < 24; ++i) {
			double fa = RelativeError(double(ulp), double(base));
			double pa = RelativeError(double(ulp), double(base));
			double ca = RelativeError(double(ulp), double(base));
			double la = RelativeError(double(ulp), double(base));
			std::cout << to_binary(base) << " " << to_binary(ulp)
				<< std::setw(WIDTH) << base
				<< std::setw(WIDTH) << fa
				<< std::setw(WIDTH) << pa
				<< std::setw(WIDTH) << ca
				<< std::setw(WIDTH) << la
				<< '\n';
			base *= 2.0;
			ulp = std::nextafter(base, 2 * base);
		}
	}

	{
		std::cout << "RelativeError of the ULP around 1.0\n";
		float base = 1.0f;
		float ulp = std::nextafter(base, 2 * base);
		constexpr int WIDTH = 25;
		std::cout << std::setw(36) << "base"
			<< std::setw(36) << "ulp"
			<< std::setw(WIDTH) << "base value"
			<< std::setw(WIDTH) << "float"
			<< std::setw(WIDTH) << "posit"
			<< std::setw(WIDTH) << "cfloat"
			<< std::setw(WIDTH) << "lns"
			<< '\n';
		for (int i = 0; i < 24; ++i) {
			double fa = RelativeError(double(ulp), double(base));
			double pa = RelativeError(double(ulp), double(base));
			double ca = RelativeError(double(ulp), double(base));
			double la = RelativeError(double(ulp), double(base));
			std::cout << to_binary(base) << " " << to_binary(ulp)
				<< std::setw(WIDTH) << base
				<< std::setw(WIDTH) << fa
				<< std::setw(WIDTH) << pa
				<< std::setw(WIDTH) << ca
				<< std::setw(WIDTH) << la
				<< '\n';
			base *= 10.0;
			ulp = std::nextafter(base, 2 * base);
		}
	}

    return EXIT_SUCCESS;
}
catch (...) {
    std::cerr << "Caught unexpected exception" << std::endl;
    return EXIT_FAILURE;
}
