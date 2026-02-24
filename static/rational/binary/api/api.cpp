// api.cpp: application programming interface tests for fixed-size arbitrary configuration binary rational number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the rational template environment
// enable/disable arithmetic exceptions
#define RATIONAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/rational/rational.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw {
	namespace universal {
		
		template<typename Rational, typename Real>
		int Conversion(Real v) {
			int nrOfFailedTestCases{ 0 };

			Rational r{ v };
			ReportValue(r, type_tag(r));
			return nrOfFailedTestCases;
		}
	}
}
int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "binary rational API";
	std::string test_tag    = "rational";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// important behavioral traits
	{
		using TestType = rational<16, base2, uint16_t>;
		ReportTrivialityOfType<TestType>();
	}

	// conversions
	std::cout << "+---------    Conversions\n";
	{
		Conversion< rb8 >(-1.875f);
		Conversion< rb16 >(1.875e1);
		Conversion< rb32 >(-1.875e5f);
		Conversion< rb64 >(1.875e10);
		Conversion< rb128 >(1.875e20);

		Conversion< rb16 >(1.0e4f);
		Conversion< rb16 >(1.5e4f);
		Conversion< rb16 >(1.75e4f);
		Conversion< rb16 >(1.875e4f);
		Conversion< rb16 >(1.9375e4f);
		Conversion< rb16 >(3.2767e4f);
	}

	// default behavior
	std::cout << "+---------    Default rational<16, base2, uint16_t>\n";
	{
		using Real = rational<16, base2, uint16_t>;

		Real a(1.0f), b(0.5f);
		std::cout << a << '\n';
		std::cout << to_binary(a) << '\n';
		using SignedBlockBinary = blockbinary<16, uint16_t>;
		SignedBlockBinary sbb = 17;
		std::cout << double(sbb) << '\n';
		//ArithmeticOperators(a, b);
	}

	// report on the dynamic range of some standard configurations
	std::cout << "+---------    Dynamic ranges of standard rational<16, base2, uint16_t> configurations   --------+\n";
	{
		// default standard types: rb8, rb16, rb32, and rb64
		ExtremeValues< rational<8, base2, uint8_t> >();
		ExtremeValues< rational<16, base2, uint16_t> >();
		ExtremeValues< rational<32, base2, uint32_t> >();
		ExtremeValues< rational<64, base2, uint64_t> >();

		std::cout << "---\n";
	}


	// constexpr and specific values
	std::cout << "+---------    constexpr and specific values   --------+\n";
	{
		using Real = rational<16, base2, uint16_t>;

		CONSTEXPRESSION Real a{}; // zero constexpr
		std::cout << type_tag(a) << '\n';

		 Real b(1.0f);  // constexpr of a native type conversion
		std::cout << to_binary(b) << " : " << b << '\n';

		CONSTEXPRESSION Real c(SpecificValue::minpos);  // constexpr of a special value in the encoding
		std::cout << to_binary(c) << " : " << c << " == minpos" << '\n';

		CONSTEXPRESSION Real d(SpecificValue::maxpos);  // constexpr of a special value in the encoding
		std::cout << to_binary(d) << " : " << d << " == maxpos" << '\n';
	}


	std::cout << "+---------    set specific values of interest   --------+\n";
	{
		rational<16, base2, uint16_t> a{ 0 }; // initialized
		std::cout << "maxpos : " << a.maxpos() << " : " << scale(a) << '\n';
		std::cout << "minpos : " << a.minpos() << " : " << scale(a) << '\n';
		std::cout << "zero   : " << a.zero()   << " : " << scale(a) << '\n';
		std::cout << "minneg : " << a.minneg() << " : " << scale(a) << '\n';
		std::cout << "maxneg : " << a.maxneg() << " : " << scale(a) << '\n';
		std::cout << dynamic_range<rational<16, base2, uint16_t>>() << std::endl;
	}

	std::cout << "+---------    rational<16, base2, uint16_t>   --------+\n";
	{
		rb16 a{ 1 }, b{ 2 }, c;
		std::cout << "in-place operators";
		c = a * b;
		for (int i = 0; i < 4; ++i) {
			std::cout << to_binary(c) << " : " << c << '\n';
			c *= 10;
		}

		std::cout << "binary operators";
		c = 2;
		for (int i = 0; i < 4; ++i) {
			std::cout << to_binary(c) << " : " << c << '\n';
			c = c * 10;
		}

		c = a + b;
		std::cout << a << " + " << b << " = " << c << '\n';
		c = a - b;
		std::cout << a << " - " << b << " = " << c << '\n';
		c = a * b;
		std::cout << a << " * " << b << " = " << c << '\n';
		c = a / b;
		std::cout << a << " / " << b << " = " << c << '\n';

		int x{ -2 };
		c = a + x;
		std::cout << a << " + " << x << " = " << c << '\n';
		c = a - x;
		std::cout << a << " - " << x << " = " << c << '\n';
		c = a * x;
		std::cout << a << " * " << x << " = " << c << '\n';
		c = a / x;
		std::cout << a << " / " << x << " = " << c << '\n';

		x = -1;
		c = x + b;
		std::cout << x << " + " << b << " = " << c << '\n';
		c = x - b;
		std::cout << x << " - " << b << " = " << c << '\n';
		c = x * b;
		std::cout << x << " * " << b << " = " << c << '\n';
		c = x / b;
		std::cout << x << " / " << b << " = " << c << '\n';

		// ratios
		a.set(1, 2); b.set(3, 4);
		c = a + b;
		std::cout << a << " + " << b << " = " << c << '\n';
		c = a - b;
		std::cout << a << " - " << b << " = " << c << '\n';
		c = a * b;
		std::cout << a << " * " << b << " = " << c << '\n';
		c = a / b;
		std::cout << a << " / " << b << " = " << c << '\n';
	}

	std::cout << "+---------    rational<16, base2, uint16_t> arithmetic closure constraints   --------+\n";
	{
		/*
			rational<8, base2, uint8_t>   : [ -128 ... -0.00787402 0 0.00787402 ... 127 ]
			rational<16, base2, uint16_t> : [ -32768 ... -3.05185e-05 0 3.05185e-05 ... 32767 ]
			rational<32, base2, uint32_t> : [ -2.14748e+09 ... -4.65661e-10 0 4.65661e-10 ... 2.14748e+09 ]
			rational<64, base2, uint64_t> : [ -9.22337e+18 ... -1.0842e-19 0 1.0842e-19 ... 9.22337e+18 ]
		 */

		float f{ 32767 };
		f /= 10000;
		std::cout << to_binary(f) << " : " << f << '\n';
		rb16 r;
		r = f;
		for (int i = 0; i < 4; ++i) {
			std::cout << to_binary(r) << " : " << r << '\n';
			r *= 10;
		}


	}

	{
		rb16 r;
		r.maxpos();
		std::cout << std::setprecision(25);
		std::cout << to_binary(float(r)) << " : " << float(r) << '\n';
		std::cout << to_binary(double(r)) << " : " << double(r) << '\n';
		// 0b0.10111110.00000000000000000000000 : 9.2233720368547758e+18
		// 0b0.10000111110.0000000000000000000000000000000000000000000000000000 : 9.2233720368547758e+18
		float f{ 9.223372036854775808e+18 };
		std::cout << to_binary(f) << " : " << f << '\n';
		double d{ 9.223372036854775808e+18 };
		std::cout << to_binary(d) << " : " << d << '\n';

		int64_t i64{ 9223372036854775807 };
		std::cout << to_binary(i64) << " : " << i64 << '\n';

		r = f;
		std::cout << to_binary(r) << " : " << r << '\n';

	}

	/*
	std::cout << "+---------    special value properties rational<16, base2, uint16_t> vs IEEE-754   --------+\n";
	{
		float fa;
		fa = NAN;
		std::cout << "qNAN   : " << to_binary(NAN) << '\n';
		std::cout << "sNAN   : " << to_binary(-NAN) << '\n';
		if (fa < 0.0f && fa > 0.0f && fa != 0.0f) {
			std::cout << "IEEE-754 is incorrectly implemented\n";
		}
		else {
			std::cout << "IEEE-754 NAN has no sign\n";
		}

		rational<16, base2, uint16_t> a(fa);
		if ((a < 0.0f && a > 0.0f && a != 0.0f)) {
			std::cout << "rational<16, base2, uint16_t> is incorrectly implemented\n";
			++nrOfFailedTestCases;
		}
		else {
			std::cout << "rational<16, base2, uint16_t> NAN has no sign\n";
		}
	}

	{
		std::cout << "rational<16, base2, uint16_t>(INFINITY): " << rational<16, base2, uint16_t>(INFINITY) << "\n";
		std::cout << "rational<16, base2, uint16_t>(-INFINITY): " << rational<16, base2, uint16_t>(-INFINITY) << "\n";

		std::cout << "rational<16, base2, uint16_t>(std::numeric_limits<float>::infinity())  : " << rational<16, base2, uint16_t>(std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "rational<16, base2, uint16_t>(-std::numeric_limits<float>::infinity()) : " << rational<16, base2, uint16_t>(-std::numeric_limits<float>::infinity()) << "\n";

		std::cout << " 2 * std::numeric_limits<float>::infinity()  : " << 2 * std::numeric_limits<float>::infinity() << "\n";
		std::cout << " 2 * std::numeric_limits<rational<16, base2, uint16_t>>::infinity() : " << 2 * std::numeric_limits<rational<16, base2, uint16_t>>::infinity() << "\n";
		std::cout << "-2 * std::numeric_limits<rational<16, base2, uint16_t>>::infinity() : " << -2 * std::numeric_limits<rational<16, base2, uint16_t>>::infinity() << "\n";

		std::cout << "sw::universal::nextafter(rational<16, base2, uint16_t>(0), std::numeric_limits<rational<16, base2, uint16_t>>::infinity())  : " << sw::universal::nextafter(rational<16, base2, uint16_t>(-0), std::numeric_limits<rational<16, base2, uint16_t>>::infinity()) << "\n";
		std::cout << "std::nextafter(float(0), std::numeric_limits<float>::infinity())                  : " << std::nextafter(float(-0), std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "sw::universal::nextafter(rational<16, base2, uint16_t>(0), -std::numeric_limits<rational<16, base2, uint16_t>>::infinity()) : " << sw::universal::nextafter(rational<16, base2, uint16_t>(0), -std::numeric_limits<rational<16, base2, uint16_t>>::infinity()) << "\n";
		std::cout << "std::nextafter(float(0), -std::numeric_limits<float>::infinity())                 : " << std::nextafter(float(0), -std::numeric_limits<float>::infinity()) << "\n";

		std::cout << "rational<16, base2, uint16_t>(std::numeric_limits<rational<16, base2, uint16_t>>::quiet_NaN()).isnan(sw::universal::NAN_TYPE_QUIET)          : " << rational<16, base2, uint16_t>(std::numeric_limits<rational<16, base2, uint16_t>>::quiet_NaN()).isnan(sw::universal::NAN_TYPE_QUIET) << "\n";
		std::cout << "rational<16, base2, uint16_t>(std::numeric_limits<rational<16, base2, uint16_t>>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) : " << rational<16, base2, uint16_t>(std::numeric_limits<rational<16, base2, uint16_t>>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) << "\n";
		std::cout << "rational<16, base2, uint16_t>(std::numeric_limits<float>::quiet_NaN()).isnan(sw::universal::NAN_TYPE_QUIET)             : " << rational<16, base2, uint16_t>(std::numeric_limits<float>::quiet_NaN()).isnan(sw::universal::NAN_TYPE_QUIET) << "\n";
		std::cout << "rational<16, base2, uint16_t>(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING)    : " << rational<16, base2, uint16_t>(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) << "\n";

		float float_sNaN{ std::numeric_limits<float>::signaling_NaN() };
		ReportValue(float_sNaN, "float_sNaN");
		rational<16, base2, uint16_t> rational_sNaN{ float_sNaN };
		ReportValue(rational_sNaN, "rational_sNaN");
		to_binary(rational_sNaN);

		float float_qNaN{ std::numeric_limits<float>::quiet_NaN() };
		ReportValue(float_qNaN, "float_qNaN");
		rational<16, base2, uint16_t> rational_qNaN{ float_qNaN };
		ReportValue(rational_qNaN, "rational_qNaN");
		to_binary(rational_qNaN);

	}
	*/
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
