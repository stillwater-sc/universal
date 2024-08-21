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
#include <universal/native/error_free_ops.hpp>

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

		void dd_binary(dd const& v) {
			std::cout << to_pair(v) << '\n';
		}

		// specialize ReportValue for double-double (dd)
		void ReportValue(const dd& a, const std::string& label = "", unsigned labelWidth = 20, unsigned precision = 32) {
			auto defaultPrecision = std::cout.precision();
			std::cout << std::setprecision(precision);
			std::cout << std::setw(labelWidth) << label << " : " << a << '\n';
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
		ReportValue(next, "nextafter 0.0");
		double one{ 1.0 };
		next = std::nextafter(one, +INFINITY);
		ReportValue(next, "nextafter 1.0");


		// ULP at 1.0 is 2^-106
		double ulpAtOne = std::pow(2.0, -106);

		dd a{ 1.0 };
		a += ulpAtOne;
		ReportValue(a, "1.0 + eps");

		a = 1.0;
		dd ddUlpAtOne = ulp(a);
		ReportValue(ddUlpAtOne, "ulp(1.0)");
		a += ulp(a);
		ReportValue(a, "1.0 + ulp(1.0)");

		dd eps = std::numeric_limits<dd>::epsilon();
		ReportValue(eps, "epsilon");

	}

	std::cout << "+----------     unevaluated pairs    ------------ +\n";
	{
		// what is the value that adds a delta one below the least significant fraction bit of the high double?
		// dd = high + lo
		//    = 1*2^0 + 1*2^-53
		//    = 1.0e00 + 1.0elog10(2^-53)
		double high{ std::pow(2.0, 0.0) };
		ReportValue(high, "2^0");
		double low{ std::pow(2.0, -53.0) };
		ReportValue(low, "2^-53");
		std::cout << std::log10(low) << '\n';
		double exponent = -std::ceil(std::abs(std::log10(low)));
		std::cout << "exponent : " << exponent << '\n';

		// now let's walk that bit down to the ULP
		std::cout << std::setprecision(32);
		for (int i = 0; i < 54; ++i) {
			low = (std::pow(2.0, -53.0 - double(i)));
			dd a(high, low);
			std::cout << a  << '\n';
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
