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
#define QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/qd/qd.hpp>
// types to compare to
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/verification/test_suite.hpp>
//#include <universal/numerics/error_free_ops.hpp>  // integral part of double-double and quad-double but can be used standalone
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
		std::cout << std::defaultfloat;
	}

	std::cout << "+----------  basic arithmetic -----------+\n";
	{
		qd a, b, c;

		a = double(1ull << 53);
		b = 1.0;

		c = a + 1.0;
		ReportValue(c, "c = a + b", 20, 32);
	}

	std::cout << "+----------  to_binary and to_components -----+\n";
	{
		std::cout << std::setprecision(64);
		qd a("0.1"), b = 1.0 / qd(3.0);

		std::cout << a << '\n';
		std::cout << to_components(a) << '\n';
		std::cout << b << '\n';
		std::cout << to_components(b) << '\n';

		std::cout << std::setprecision(defaultPrecision);
	}

	{
		std::cout << std::setprecision(32);
		dd a("0.1"), b = 1.0 / dd(3.0);

		std::cout << a << '\n';
		std::cout << to_components(a) << '\n';
		std::cout << b << '\n';
		std::cout << to_components(b) << '\n';

		std::cout << std::setprecision(defaultPrecision);
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