// experiments.cpp: experiments with the quad-double (qd) floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <numbers>
// minimum set of include files to reflect source code dependencies
// Configure the qd template environment
// enable/disable arithmetic exceptions
#define DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/qd/qd.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
//#include <universal/native/error_free_ops.hpp>  // integral part of double-double and quad-double but can be used standalone
#include <universal/common/string_utils.hpp>

namespace sw {
	namespace universal {


		/*
		    enum _Fmtflags { // constants for formatting options
				_Fmtmask = 0xffff,
				_Fmtzero = 0
			};

			static constexpr int skipws     = 0x0001;
			static constexpr int unitbuf    = 0x0002;
			static constexpr int uppercase  = 0x0004;
			static constexpr int showbase   = 0x0008;
			static constexpr int showpoint  = 0x0010;
			static constexpr int showpos    = 0x0020;
			static constexpr int left       = 0x0040;
			static constexpr int right      = 0x0080;
			static constexpr int internal   = 0x0100;
			static constexpr int dec        = 0x0200;
			static constexpr int oct        = 0x0400;
			static constexpr int hex        = 0x0800;
			static constexpr int scientific = 0x1000;
			static constexpr int fixed      = 0x2000;

			static constexpr int boolalpha   = 0x4000;
			static constexpr int adjustfield = left | right | internal;
			static constexpr int basefield   = dec | oct | hex;
			static constexpr int floatfield  = scientific | fixed;
		 */
		struct fmtCapture {
			double v;
		};

		std::ostream& operator<<(std::ostream& ostr, const fmtCapture& v) {
			std::ios_base::fmtflags fmt = ostr.flags();
			std::streamsize precision = ostr.precision();
			std::streamsize width = ostr.width();
//			char fillChar = ostr.fill();
//			bool showpos = fmt & std::ios_base::showpos;
//			bool uppercase = fmt & std::ios_base::uppercase;
			bool fixed = fmt & std::ios_base::fixed;
			bool scientific = fmt & std::ios_base::scientific;

			bool left = fmt & std::ios_base::left;
			bool right = fmt & std::ios_base::right;
			bool internal = fmt & std::ios_base::internal;

			ostr << "width     = " << width << '\n';
			ostr << "precision = " << precision << '\n';
			ostr << (fixed ? "fixed\n" : "not fixed\n");
			ostr << (scientific ? "scientific\n" : "not scientific\n");
			ostr << (left ? "left\n" : "not left\n");
			ostr << (internal ? "internal\n" : "not internal\n");
			ostr << (right ? "right\n" : "not right\n");

			return ostr << v.v;
		}

	}
}



int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "quad-double (qd) experiments";
	int nrOfFailedTestCases = 0;

	auto defaultPrecision = std::cout.precision();


	std::cout << "+ ---------- - unevaluated pairs------------ +\n";
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
		double x0{ 1.0 };
		double x1{ 0.0 };
		double x2{ 0.0 };
		double x3{ 0.0 };
		int precisionForRange = 16;
		std::cout << std::setprecision(precisionForRange);
		x0 = 1.0;
		qd a(x0, x1, x2, x3);
		std::cout << centered("quad-double", precisionForRange + 6) << " : ";
		std::cout << centered("binary form of x0", 68) << " : ";
		std::cout << centered("real value of x0", 15) << '\n';
		std::cout << a << " : " << to_binary(x0) << " : " << x0 << '\n';
		for (int i = 1; i < 53; ++i) {
			x0 = 1.0 + (std::pow(2.0, - double(i)));
			qd a(x0, x1, x2, x3);
			std::cout << a << " : " << to_binary(x0) << " : " << std::setprecision(7) << x0 << std::setprecision(precisionForRange) << '\n';
		}
		// x0 is 1.0 + eps() at this point
		// std::cout << to_binary(x0) << '\n';
		std::cout << to_binary(qd(x0, x1, x2, x3)) << '\n';
		x0 = 1.0;
		precisionForRange = 32;
		std::cout << std::setprecision(precisionForRange);
		std::cout << centered("quad-double", precisionForRange + 6) << " : ";
		std::cout << centered("binary form of x1", 68) << " : ";
		std::cout << centered("real value of x1", 15) << '\n';
		for (int i = 0; i < 54; ++i) {
			x1 = (std::pow(2.0, -53.0 - double(i)));
			qd a(x0, x1, x2, x3);
			std::cout << a << " : " << to_binary(x1) << " : " << std::setprecision(7) << x1 << std::setprecision(precisionForRange) << '\n';
		}
		std::cout << to_binary(qd(x0, x1, x2, x3)) << '\n';
		x1 = 0.0;
		precisionForRange = 48;
		std::cout << std::setprecision(precisionForRange);
		std::cout << centered("quad-double", precisionForRange + 6) << " : ";
		std::cout << centered("binary form of x2", 68) << " : ";
		std::cout << centered("real value of x2", 15) << '\n';
		for (int i = 0; i < 54; ++i) {
			x2 = (std::pow(2.0, -106.0 - double(i)));
			qd a(x0, x1, x2, x3);
			std::cout << a << " : " << to_binary(x2) << " : " << std::setprecision(7) << x2 << std::setprecision(precisionForRange) << '\n';
		}
		std::cout << to_binary(qd(x0, x1, x2, x3)) << '\n';
		x2 = 0.0;
		precisionForRange = 64;
		std::cout << std::setprecision(precisionForRange);
		std::cout << centered("quad-double", precisionForRange + 6) << " : ";
		std::cout << centered("binary form of x3", 68) << " : ";
		std::cout << centered("real value of x3", 15) << '\n';
		for (int i = 0; i < 54; ++i) {
			x3 = (std::pow(2.0, -159.0 - double(i)));
			qd a(x0, x1, x2, x3);
			std::cout << a << " : " << to_binary(x3) << " : " << std::setprecision(7) << x3 << std::setprecision(precisionForRange) << '\n';
		}
		std::cout << to_binary(qd(x0, x1, x2, x3)) << '\n';
		std::cout << std::setprecision(defaultPrecision);
	}

	return 0;
	{
		// what is the difference between ostream fmt scientific/fixed

		fmtCapture v;
		v.v = 1.0e10;
		std::cout << " 1 " << v << '\n';
		std::cout << " 2 " << std::fixed << v << '\n';
		std::cout << " 3 " << std::scientific << v << '\n';
		std::cout << " 4 " << std::defaultfloat << v << '\n';
		std::cout << " 5 " << std::setw(10) << v << '\n';

		std::cout << " 6 " << std::fixed << std::scientific << v << '\n';
		std::cout << " 7 " << v << '\n';
		std::cout << " 8 " << std::scientific << std::fixed << v << '\n';
		std::cout << " 9 " << v << '\n';
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


/*
void ThreeSumExperiments() {
	using namespace sw::universal;
	std::cout << "three_sum experiments\n";

	double a{ 1.0 };
	double b{ ulp(a) / 2.0 };
	double c{ ulp(b) / 2.0 };
	ReportValue(a, "a = 1.0");
	ReportValue(b, "b = ulp(1.0)/2");
	ReportValue(c, "c = ulp(b)/2");

	std::cout << "two_sum\n";
	double r{ 0 };
	double s = two_sum(a, b, r);
	ReportValue(s, "sum");
	ReportValue(r, "residual");

	std::cout << "three_sum\n";
	double aa{ a }, bb{ b }, cc{ c };
	ReportValue(a, "a");
	ReportValue(b, "b");
	ReportValue(c, "c");
	three_sum(a, b, c);
	ReportValue(a, "a");
	ReportValue(b, "b");
	ReportValue(c, "c");

	std::cout << "three_sum2\n";
	a = aa, b = bb, c = cc; // reload
	ReportValue(a, "a");
	ReportValue(b, "b");
	ReportValue(c, "c");
	three_sum2(a, b, c);
	ReportValue(a, "a");
	ReportValue(b, "b");
	// c is unchanged

	std::cout << "three_sum3\n";
	a = aa, b = bb, c = cc; // reload
	ReportValue(a, "a");
	ReportValue(b, "b");
	ReportValue(c, "c");
	double sum = three_sum3(a, b, c);
	ReportValue(sum, "three_sum3");

}

three_sum experiments
             a = 1.0 : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
      b = ulp(1.0)/2 : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
        c = ulp(b)/2 : 0b0.011'1001'0101.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.232595e-32
two_sum
                 sum : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
            residual : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
three_sum
 in                a : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
                   b : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
                   c : 0b0.011'1001'0101.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.232595e-32
 out               a : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
                   b : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
                   c : 0b0.011'1001'0101.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.232595e-32
three_sum2
 in                a : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
                   b : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
                   c : 0b0.011'1001'0101.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.232595e-32
 out               a : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
                   b : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
three_sum3
 in                a : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
                   b : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
                   c : 0b0.011'1001'0101.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.232595e-32
 out      three_sum3 : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1

*/