// experiments.cpp: experiments with the double-double floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <numbers>
// minimum set of include files to reflect source code dependencies
// Configure the dd template environment
// enable/disable arithmetic exceptions
#define DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/dd/dd.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
//#include <universal/numerics/error_free_ops.hpp>  // integral part of double-double and quad-double but can be used standalone
#include <universal/common/string_utils.hpp>

namespace sw {
	namespace universal {

		void Progression(double v) {
			using namespace sw::universal;

			auto defaultPrecision = std::cout.precision();
			float f{ float(v) };
			std::cout << std::setprecision(7);
			std::cout << to_binary(f, true) << " : " << f << '\n';

			double d{ v };
			std::cout << std::setprecision(17);
			std::cout << to_binary(d, true) << " : " << d << '\n';

			dd a{ v };
			std::cout << std::setprecision(35);
			std::cout << to_binary(a, true) << " : " << a << '\n';
			std::cout << std::setprecision(defaultPrecision);
		}


		void SettingBits() {
			std::cout << "+----------     Setting float bits    ---------+\n";
			{
				float v{ 0.0f };
				setbit(v, 31);
				ReportValue(v);
				setbit(v, 23); // set min normal
				ReportValue(v);
				setbit(v, 23, false); setbit(v, 0); // set smallest denorm
				ReportValue(v);
			}
			std::cout << "+----------     Setting double bits    ---------+\n";
			{
				double v{ 0.0 };
				setbit(v, 63);
				ReportValue(v);
				setbit(v, 52); // set min normal
				ReportValue(v);
				setbit(v, 52, false); setbit(v, 0); // set smallest denorm
				ReportValue(v);
			}
			std::cout << "+----------     Setting double-double bits    ---------+\n";
			{
				dd v{ 0.0 };
				v.setbit(127);
				ReportValue(v);
				v.setbit(116); // set min normal
				ReportValue(v);
				v.setbit(116, false); v.setbit(64); // set smallest denorm
				ReportValue(v);
			}
		}

		void dd_binary(dd const& v) {
			std::cout << to_pair(v) << '\n';
		}

		void adjust(dd const& a) {
			dd r = abs(a);
			dd ten(10.0);
			int e{ 0 };
			dd_binary(r);
			frexp(r, &e);
			std::cout << "exponent : " << e << '\n';

			if (e < 0) {
				if (e > 300) {
					r = ldexp(r, 53);		dd_binary(r);
					r *= pown(ten, -e);	dd_binary(r);
					r = ldexp(r, -53);	dd_binary(r);
				}
				else {
					r *= pown(ten, -e);	dd_binary(r);
				}
			}
			else {
				if (e > 0) {
					if (e > 300) {
						r = ldexp(r, -53);	dd_binary(r);
						r /= pown(ten, e);		dd_binary(r);
						r = ldexp(r, 53);		dd_binary(r);
					}
					else {
						r /= pown(ten, -e);	dd_binary(r);
					}
				}
			}
		}

	}
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "double-double (dd) experiments";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	auto defaultPrecision = std::cout.precision();

	std::cout << "+----------                ULP assessments             ---------+\n";
	{
		double zero{ 0.0 };
		double next = std::nextafter(zero, +INFINITY);
		ReportValue(next, "nextafter 0.0", 40);
		double one{ 1.0 };
		next = std::nextafter(one, +INFINITY);
		ReportValue(next, "nextafter 1.0", 40);
		std::cout << '\n';

		{
			// ULP at 1.0 is 2^-106
			double ulpAtOne = std::pow(2.0, -106);

			dd a{ 1.0 };
			a += ulpAtOne;
			ReportValue(a, "reference of 1.0 + ulp(1.0)", 40);

			a = 1.0;
			dd ddUlpAtOne = ulp(a);
			ReportValue(ddUlpAtOne, "ulp(1.0)", 40);
			a += ulp(a);
			ReportValue(a, "ulp function of 1.0 + ulp(1.0)", 40);

			double d_ulpAtOne = ulp(1.0);
			ReportValue(d_ulpAtOne, "ulp<double>(1.0)", 40);
			double d_epsilon = std::numeric_limits<double>::epsilon();
			ReportValue(d_epsilon, "epsilon<double>", 40);
			ReportValue(1.0 + d_epsilon, "1.0 + eps", 40);
			dd dd_epsilon = std::numeric_limits<dd>::epsilon();
			ReportValue(dd_epsilon, "epsilon<double-double>", 40);
			a = 1.0;
			a += dd_epsilon;
			ReportValue(a, "1.0 + eps", 40);
		}

		{
			dd a; // uninitialized
			double hi{ 1.0 }, lo{ 0.0 };
			a.set(hi, lo); // set the value of the double-double: set does not check the arguments for alignment
			double nlo;
			if (lo == 0.0) {
				nlo = std::numeric_limits<double>::epsilon() / 2.0;
				int binaryExponent = scale(hi) - 53;
				nlo /= std::pow(2.0, -binaryExponent);
			}
			else {
				nlo = (hi < 0.0 ? std::nextafter(lo, -INFINITY) : std::nextafter(lo, +INFINITY));
			}
			dd n(hi, nlo);
			ReportValue(a, "a = 1.0");
			ReportValue(nlo, "new low");
			ReportValue(n, "n");
			ReportValue(n - a, "n - a");
		}

		std::cout << '\n';
		for (int i = 0; i < 10; ++i) {
			double a(double(1ull << i));
			double ulpAtI = ulp(a);
			std::string label = "ulpAt<double>(2^" + std::to_string(i) + ")";
			ReportValue(ulpAtI, label);
		}
		std::cout << '\n';
		for (int i = 0; i < 5; ++i) {
			dd a(1ull << i);
			dd ulpAtI = ulp(a);
			std::string label = "ulpAt<dd>(2^" + std::to_string(i) + ")";
			ReportValue(ulpAtI, label, 20, 32);
		}
		std::cout << std::right << std::setw(20) << "......." << " :\n" << std::internal;
		for (int i = 53; i < 64; ++i) {
			dd a(1ull << i);
			dd ulpAtI = ulp(a);
			std::string label = "ulpAt<dd>(2^" + std::to_string(i) + ")";
			ReportValue(ulpAtI, label, 20, 32);
		}
		std::cout << '\n';
		std::cout << "   with a non-zero low segment\n";
		for (int i = 0; i < 5; ++i) {
			dd a(1ull << i);
			dd ulpAtI = ulp(a);
			ulpAtI += a;
			std::string label = "ulpAt<dd>(2^" + std::to_string(i) + "+ulp)";
			ReportValue(ulpAtI, label, 20, 32);
//			std::cout << to_components(ulpAtI) << std::setprecision(32) << ulpAtI << std::setprecision(defaultPrecision) << '\n';
//			std::cout << to_binary(ulpAtI, true) << '\n';
		}
	}

	std::cout << "+----------     unevaluated pairs    ------------ +\n";
	{
		// what is the value that adds a delta one below the least significant fraction bit of the high double?
		// dd = high + lo
		//    = 1*2^0 + 1*2^-53
		//    = 1.0e00 + 1.0elog10(2^-53)
		double x0{ std::pow(2.0, 0.0) };
		ReportValue(x0, "2^0");
		double x1{ std::pow(2.0, -53.0) };
		ReportValue(x1, "2^-53");
		std::cout << std::log10(x1) << '\n';
		double exponent = -std::ceil(std::abs(std::log10(x1)));
		std::cout << "exponent : " << exponent << '\n';

		// now let's walk that bit down to the ULP
		unsigned precisionForRange = 16;
		std::cout << std::setprecision(precisionForRange);
		x0 = 1.0;
		dd a(x0, x1);
		std::cout << centered("double-double", precisionForRange + 6u) << " : ";
		std::cout << centered("binary form of x0", 68) << " : ";
		std::cout << centered("real value of x0", 15) << '\n';
		std::cout << a << " : " << to_binary(x0) << " : " << x0 << '\n';
		for (int i = 1; i < 53; ++i) {
			x0 = 1.0 + (std::pow(2.0, -double(i)));
			a.set(x0, x1);
			std::cout << a << " : " << to_binary(x0) << " : " << std::setprecision(7) << x0 << std::setprecision(precisionForRange) << '\n';
		}
		// x0 is 1.0 + eps() at this point
		std::cout << to_binary(dd(x0, x1)) << '\n';
		x0 = 1.0;
		precisionForRange = 32;
		std::cout << std::setprecision(precisionForRange);
		std::cout << centered("double-double", precisionForRange + 6u) << " : ";
		std::cout << centered("binary form of x1", 68) << " : ";
		std::cout << centered("real value of x1", 15) << '\n';
		for (int i = 0; i < 54; ++i) {
			x1 = (std::pow(2.0, -53.0 - double(i)));
			a.set(x0, x1);
			std::cout << a << " : " << to_binary(x1) << " : " << std::setprecision(7) << x1 << std::setprecision(precisionForRange) << '\n';
		}
		std::cout << std::setprecision(defaultPrecision);
	}

	std::cout << "+----------     Smallest normal number progressions    ---------+\n";
	{
		constexpr double smallestNormal = std::numeric_limits<double>::min();
		dd a(smallestNormal);
		for (int i = 0; i < 10; ++i) {
			ReportValue(a);
			a *= 2.0;
		}
	}

	std::cout << "+----------     subnormal exponent adjustment    ---------+\n";
	{
		constexpr double smallestNormal = std::numeric_limits<double>::min();
		dd a{ smallestNormal };
		for (int i = 0; i < 5; ++i) {
			adjust(a);
			a /= 2.0;
		}
		a = smallestNormal;
		for (int i = 0; i < 5; ++i) {
			adjust(a);
			a *= 2.0;
		}

	}

	std::cout << "+---------    double-double subnormal behavior   --------+\n";
	{
		constexpr double smallestNormal = std::numeric_limits<double>::min();
		ReportValue(smallestNormal, "smallest normal");
		double ulpAtSmallestNormal = ulp(smallestNormal);
		ReportValue(ulpAtSmallestNormal, "ulpAtSmallestNormal");
		double subnormal = smallestNormal / 2.0;
		std::cout << to_binary(subnormal) << " : " << subnormal << '\n';
		dd a(smallestNormal + ulpAtSmallestNormal);
		for (int i = 0; i < 10/*106*/; ++i) {
			std::string tag = "pow(a, -" + std::to_string(i) + ")";
			ReportValue(a, tag);
			a /= 2.0;
		}
	}

	std::cout << "---------  decimal string rounding   -------------\n";
	{
		dd a{};
		a.assign("1.5555555");
		std::cout << "default to_string()    format : " << a.to_string() << '\n';
		a.assign("1.5555554");
		std::cout << "default to_string()    format : " << a.to_string() << '\n';
		a.assign("1.5555556");
		std::cout << "default to_string()    format : " << a.to_string() << '\n';
		a.assign("1.55555555");
		std::cout << "default to_string()    format : " << a.to_string() << '\n';
		a.assign("1.55555554");
		std::cout << "default to_string()    format : " << a.to_string() << '\n';
		a.assign("1.55555556");
		std::cout << "default to_string()    format : " << a.to_string() << '\n';
		a.assign("1.55555555");
		std::cout << "to_string(precision=4) format : " << a.to_string(4) << '\n';
		a.assign("1.55555554");
		std::cout << "to_string(precision=4) format : " << a.to_string(4) << '\n';
		a.assign("1.55555556");
		std::cout << "to_string(precision=4) format : " << a.to_string(4) << '\n';
	}

	std::cout << "+-----------    splitting a double value   --------------+\n";
	{
		const int BITS = ( std::numeric_limits< double >::digits + 1 ) / 2;   // == 27
		const double SPLITTER = std::ldexp(1.0, BITS) + 1.0; // ==  134217729.0
		const double SPLIT_THRESHOLD = std::ldexp((std::numeric_limits< double >::max)(), -BITS - 1);  // == 6.6969287949141700e+299
		ReportValue(SPLITTER, "SPLITTER");
		ReportValue(SPLIT_THRESHOLD, "SPLIT_THRESHOLD", 20, 17);

		double a, increment;

		std::cout << std::setprecision(17);

		increment = SPLIT_THRESHOLD / 2.0;
		ReportValue(increment);
		a = increment;
		for (int i = 0; i < 3; ++i) {
			double hi, lo;
			split(a, hi, lo);
			ReportValue(a, "a");
//			ReportValue(hi, "hi");
//			ReportValue(lo, "lo");
			a += increment;
		}

		std::cout << std::setprecision(defaultPrecision);
	}

	std::cout << "+------------   Horner's Rule ----------+\n";
	{
		std::vector<dd> polynomial = {
			1.0, 1.0, 1.0, 1.0, 1.0, 1.0
		};

		std::cout << "polyeval(1.0)  : " << polyeval(polynomial, 5, dd(1.0)) << '\n';
	}

	std::cout << "+------------   gamma function ----------+\n";
	{
		double param, result;
		param = 0.5;
		result = tgamma(param);
		std::cout << "tgamme(0.5) : " << result << '\n';
	}


	std::cout << std::setprecision(defaultPrecision);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
