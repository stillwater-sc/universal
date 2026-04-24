// api.cpp: application programming interface tests for bfloat16 number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the bfloat template environment
// enable/disable arithmetic exceptions
#define BFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "bfloat16 API tests";
	int nrOfFailedTestCases = 0;

	std::cout << "+-----------------   constexpr support (issue #725 / Epic #723)\n";
#if BIT_CAST_IS_CONSTEXPR
	{
		// bfloat16 is the upper 16 bits of an IEEE-754 float; conversion is
		// pure bit-shuffle via sw::bit_cast. After issue #725, all
		// constructors, operator=, conversion-out, increment/decrement, and
		// arithmetic operators are BIT_CAST_CONSTEXPR.
		// These checks fail to compile if the constexpr decoration is wrong.
		constexpr bfloat16 cx_pi   (3.14f);
		constexpr bfloat16 cx_npi  (-3.14f);
		constexpr bfloat16 cx_zero (0.0f);
		constexpr bfloat16 cx_one  (1.0f);
		constexpr bfloat16 cx_two  (2.0f);
		constexpr bfloat16 cx_d_pi (3.14);   // double source
		constexpr bfloat16 cx_int42(42);     // int source
		constexpr bfloat16 cx_neg42(-42);    // negative int

		// Constexpr arithmetic via lambda
		constexpr bfloat16 cx_sum  = []() { bfloat16 t(2.0f); t += bfloat16(3.0f); return t; }();
		constexpr bfloat16 cx_prod = []() { bfloat16 t(2.0f); t *= bfloat16(3.0f); return t; }();
		constexpr bfloat16 cx_diff = []() { bfloat16 t(5.0f); t -= bfloat16(2.0f); return t; }();
		constexpr bfloat16 cx_quot = []() { bfloat16 t(6.0f); t /= bfloat16(2.0f); return t; }();
		constexpr bfloat16 cx_neg  = -cx_pi;
		constexpr bfloat16 cx_inc  = []() { bfloat16 t(0.0f); ++t; return t; }();
		// suppress unused-variable warnings -- the constexpr evaluation IS the test
		(void)cx_npi; (void)cx_one; (void)cx_zero; (void)cx_neg; (void)cx_inc;

		// Constexpr conversion-out
		constexpr float two_back  = float(cx_two);
		constexpr int   int_back  = int(cx_neg42);

		static_assert(float(cx_sum)  == 5.0f, "constexpr 2+3 == 5");
		static_assert(float(cx_prod) == 6.0f, "constexpr 2*3 == 6");
		static_assert(float(cx_diff) == 3.0f, "constexpr 5-2 == 3");
		static_assert(float(cx_quot) == 3.0f, "constexpr 6/2 == 3");
		static_assert(two_back == 2.0f,        "constexpr conversion-out");
		static_assert(int_back == -42,         "constexpr int conversion");
		static_assert(cx_zero != cx_one,       "constexpr comparison");

		// Cross-check constexpr value matches runtime value bit-for-bit
		bfloat16 rt_pi  (3.14f);
		bfloat16 rt_int42(42);
		bfloat16 rt_d_pi (3.14);
		if (float(cx_pi)    != float(rt_pi))    { ++nrOfFailedTestCases; std::cout << "FAIL constexpr bfloat16(3.14f)\n"; }
		if (float(cx_int42) != float(rt_int42)) { ++nrOfFailedTestCases; std::cout << "FAIL constexpr bfloat16(42)\n"; }
		if (float(cx_d_pi)  != float(rt_d_pi))  { ++nrOfFailedTestCases; std::cout << "FAIL constexpr bfloat16(3.14)\n"; }
		std::cout << "PASS constexpr support\n";
	}
#else
	{
		std::cout << "SKIP constexpr support (compiler lacks constexpr bit_cast support)\n";
	}
#endif

	{
		bfloat16 a;
		a = 1.0f;
		std::cout << to_binary(a) << " : " << a << '\n';
		if (a.isone()) {
			std::cout << "bfloat16 isone() test passed\n";
		} else {
			std::cout << "bfloat16 isone() test failed\n";
			++nrOfFailedTestCases;
		}
	}

	// important behavioral traits
	{
		using TestType = bfloat16;
		ReportTrivialityOfType<TestType>();
	}

	// default behavior
	std::cout << "+---------    Default bfloat16 has subnormals, but no max-exponent values\n";
	{
		using Real = bfloat16;

		Real a(1.0f), b(0.5f);
		ArithmeticOperators(a, b);
	}

	// logical operators
	std::cout << "+---------    Logical operators\n";
	{
		using Real = bfloat16;
		Real a(1.0f), b(0.5f);
		LogicalOperators(a, b);
	}

	// explicit conversions
	std::cout << "+---------    Explicit conversions\n";
	{
		using Real = bfloat16;
		Real a(1.0f);
		ExplicitConversions(a);
	}

	// report on the dynamic range of some standard configurations
	std::cout << "+---------    Dynamic ranges of standard bfloat16 configurations   --------+\n";
	{
		bfloat16 bf; // uninitialized

		bf.maxpos();
		std::cout << "maxpos  bfloat16 : " << to_binary(bf) << " : " << bf << '\n';
		bf.setbits(0x0080);  // positive min normal
		std::cout << "minnorm bfloat16 : " << to_binary(bf) << " : " << bf << '\n';
		bf.minpos();
		std::cout << "minpos  bfloat16 : " << to_binary(bf) << " : " << bf << '\n';
		bf.zero();
		std::cout << "zero             : " << to_binary(bf) << " : " << bf << '\n';
		bf.minneg();
		std::cout << "minneg  bfloat16 : " << to_binary(bf) << " : " << bf << '\n';
		bf.setbits(0x8080);  // negative min normal
		std::cout << "minnegnorm       : " << to_binary(bf) << " : " << bf << '\n';
		bf.maxneg();
		std::cout << "maxneg  bfloat16 : " << to_binary(bf) << " : " << bf << '\n';

		std::cout << "---\n";
	}

	// use type aliases of standard configurations
	std::cout << "+---------    Type aliases for some industry standard float configurations   --------+\n";
	{
		float f1, f2, f3;
		f1 = 1.0f;
		f2 = 1.0e-3f;
		f3 = f1 / f2;
		std::cout << "float32  : " << type_tag(f3) << '\n';
		std::cout << f1 << " / " << f2 << " = " << f3 << " : " << to_binary(f3) << '\n';

		sw::universal::bfloat16 b1(f1), b2(f2), b3;
		b3 = b1 / b2;
		std::cout << "bfloat16 : " << type_tag(b3) << '\n';
		std::cout << b1 << " / " << b2 << " = " << b3 << " : " << to_binary(b3) << '\n';

	}

	// constexpr and specific values
	std::cout << "+---------    constexpr and specific values   --------+\n";
	{
		using Real = bfloat16;

		CONSTEXPRESSION Real a{}; // zero constexpr
		std::cout << type_tag(a) << '\n';

		 Real b(1.0f);  // constexpr of a native type conversion
		std::cout << to_binary(b) << " : " << b << '\n';

		CONSTEXPRESSION Real c(SpecificValue::minpos);  // constexpr of a special value in the encoding
		std::cout << to_binary(c) << " : " << c << " == minpos" << '\n';

		CONSTEXPRESSION Real d(SpecificValue::maxpos);  // constexpr of a special value in the encoding
		std::cout << to_binary(d) << " : " << d << " == maxpos" << '\n';
	}

	// set bit patterns
	std::cout << "+---------    set bit patterns API   --------+\n";
	{
		using Real = bfloat16;

		Real a; // uninitialized
		std::cout << type_tag(a) << '\n';

		a.setbits(0x0000);
		std::cout << to_binary(a) << " : " << a << '\n';

		a.setbit(8);
		std::cout << to_binary(a) << " : " << a << " : set bit 8 assuming 0-based" << '\n';
		a.setbits(0xffff);
		a.setbit(8, false);
		std::cout << to_binary(a) << " : " << a << " : reset bit 8" << '\n';

		a.setbits(0xAAAA);
		std::cout << to_binary(a) << " : " << a << '\n';

		a.assign(std::string("0b1.0101'0101.0101'010"));
		std::cout << to_binary(a) << " : " << a << '\n';

		a.assign(std::string("0b0.1010'1010.1010'101"));
		std::cout << to_binary(a) << " : " << a << '\n';
	}

	std::cout << "+---------    set specific values of interest   --------+\n";
	{
		bfloat16 a{ 0 }; // initialized
		std::cout << "maxpos : " << a.maxpos() << " : " << scale(a) << '\n';
		std::cout << "minpos : " << a.minpos() << " : " << scale(a) << '\n';
		std::cout << "zero   : " << a.zero()   << " : " << scale(a) << '\n';
		std::cout << "minneg : " << a.minneg() << " : " << scale(a) << '\n';
		std::cout << "maxneg : " << a.maxneg() << " : " << scale(a) << '\n';
		std::cout << dynamic_range<bfloat16>() << std::endl;
	}

	/* reference
	std::cout << "+---------    cfloat<16, 8, uint16_t, hasSubnormals, noSupernormals, notSaturating>   --------+\n";
	{
		constexpr size_t nbits = 16;
		constexpr size_t es = 8;
		using BlockType = uint16_t;
		using Cfloat = cfloat<nbits, es, BlockType, true>;
		constexpr size_t fbits = Cfloat::fbits;
		Cfloat a, b; // uninitialized

		std::streamsize precision = std::cout.precision();
		//std::cout << std::setprecision(3);
		//std::cout << std::fixed;
		std::cout << std::setw(nbits) << "binary" << " : " << std::setw(nbits) << "native" << " : " << std::setw(nbits) << "conversion\n";

		// enumerate the subnormals
		uint16_t pattern = 0x1ul;

		for (unsigned i = 0; i < fbits; ++i) {
			a.setbits(pattern);
			std::cout << color_print(a) << " : " << std::setw(nbits) << a << " : " << std::setw(nbits) << float(a) << '\n';
			pattern <<= 1;
		}
		// enumerate the normals
		a.setbits(0x0080u);
		for (size_t i = 0; i < 254; ++i) {
			std::cout << color_print(a) << " : " << std::setw(nbits) << a << " : " << std::setw(nbits) << float(a) << " + 1ULP ";
			b = a; ++b;
			std::cout << color_print(b) << " : " << std::setw(nbits) << b << " : " << std::setw(nbits) << float(b) << '\n';
			a *= 2;
		}
		std::cout << std::setprecision(precision);
		std::cout << std::scientific;
	}
	*/
	std::cout << "+---------    bfloat16   --------+\n";
	{
		using Bfloat = bfloat16;
		constexpr unsigned nbits = 16;
		//constexpr unsigned es = 8;
		constexpr unsigned fbits = 7;
		Bfloat a, b; // uninitialized

		std::streamsize precision = std::cout.precision();
		//std::cout << std::setprecision(3);
		//std::cout << std::fixed;
		std::cout << std::setw(nbits) << "binary" << " : " << std::setw(nbits) << "native" << " : " << std::setw(nbits) << "conversion\n";

		// enumerate the subnormals
		uint16_t pattern = 0x1ul;
		for (unsigned i = 0; i < fbits; ++i) {
			a.setbits(pattern);
			std::cout << color_print(a) << " : " << std::setw(nbits) << a << " : " << std::setw(nbits) << float(a) << '\n';
			pattern <<= 1;
		}
		// enumerate the normals
		a.setbits(0x0080u);
		for (size_t i = 0; i < 254; ++i) {
			std::cout << color_print(a) << " : " << std::setw(nbits) << a << " : " << std::setw(nbits) << float(a) << " + 1ULP ";
			b = a; ++b;
			std::cout << color_print(b) << " : " << std::setw(nbits) << b << " : " << std::setw(nbits) << float(b) << '\n';
			a *= 2;
		}
		std::cout << std::setprecision(precision);
		std::cout << std::scientific;
	}

	std::cout << "+---------    special value properties bfloat16 vs IEEE-754   --------+\n";
	{
		float fa;
		fa = NAN;
		std::cout << "qNAN   : " << to_binary(NAN) << '\n';
		std::cout << "sNAN   : " << to_binary(-NAN) << '\n';
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-overlap-compare"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlogical-op"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4127) // conditional expression is constant
#endif
		if (fa < 0.0f && fa > 0.0f && fa != 0.0f) {
			std::cout << "IEEE-754 is incorrectly implemented\n";
		}
		else {
			std::cout << "IEEE-754 NAN has no sign\n";
		}
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

		bfloat16 a(fa);
		if ((a < 0.0f && a > 0.0f && a != 0.0f)) {
			std::cout << "bfloat16 is incorrectly implemented\n";
			//++nrOfFailedTestCases;
		}
		else {
			std::cout << "bfloat16 NAN has no sign\n";
		}
	}

	{
		std::cout << "bfloat16(INFINITY): " << bfloat16(INFINITY) << "\n";
		std::cout << "bfloat16(-INFINITY): " << bfloat16(-INFINITY) << "\n";

		std::cout << "bfloat16(std::numeric_limits<float>::infinity())  : " << bfloat16(std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "bfloat16(-std::numeric_limits<float>::infinity()) : " << bfloat16(-std::numeric_limits<float>::infinity()) << "\n";

		std::cout << " 2 * std::numeric_limits<float>::infinity()  : " << 2 * std::numeric_limits<float>::infinity() << "\n";
		std::cout << " 2 * std::numeric_limits<bfloat16>::infinity() : " << 2 * std::numeric_limits<bfloat16>::infinity() << "\n";
		std::cout << "-2 * std::numeric_limits<bfloat16>::infinity() : " << -2 * std::numeric_limits<bfloat16>::infinity() << "\n";

		std::cout << "sw::universal::nextafter(bfloat16(0), std::numeric_limits<bfloat16>::infinity())  : " << sw::universal::nextafter(bfloat16(-0), std::numeric_limits<bfloat16>::infinity()) << "\n";
		std::cout << "std::nextafter(float(0), std::numeric_limits<float>::infinity())                  : " << std::nextafter(float(-0), std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "sw::universal::nextafter(bfloat16(0), -std::numeric_limits<bfloat16>::infinity()) : " << sw::universal::nextafter(bfloat16(0), -std::numeric_limits<bfloat16>::infinity()) << "\n";
		std::cout << "std::nextafter(float(0), -std::numeric_limits<float>::infinity())                 : " << std::nextafter(float(0), -std::numeric_limits<float>::infinity()) << "\n";

		std::cout << "bfloat16(std::numeric_limits<bfloat16>::quiet_NaN()).isnan(sw::universal::NAN_TYPE_QUIET)          : " << bfloat16(std::numeric_limits<bfloat16>::quiet_NaN()).isnan(sw::universal::NAN_TYPE_QUIET) << "\n";
		std::cout << "bfloat16(std::numeric_limits<bfloat16>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) : " << bfloat16(std::numeric_limits<bfloat16>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) << "\n";
		std::cout << "bfloat16(std::numeric_limits<float>::quiet_NaN()).isnan(sw::universal::NAN_TYPE_QUIET)             : " << bfloat16(std::numeric_limits<float>::quiet_NaN()).isnan(sw::universal::NAN_TYPE_QUIET) << "\n";
		std::cout << "bfloat16(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING)    : " << bfloat16(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) << "\n";

		float float_sNaN{ std::numeric_limits<float>::signaling_NaN() };
		ReportValue(float_sNaN, "float_sNaN");
		bfloat16 bfloat_sNaN{ float_sNaN };
		ReportValue(bfloat_sNaN, "bfloat_sNaN");
		to_binary(bfloat_sNaN);

		float float_qNaN{ std::numeric_limits<float>::quiet_NaN() };
		ReportValue(float_qNaN, "float_qNaN");
		bfloat16 bfloat_qNaN{ float_qNaN };
		ReportValue(bfloat_qNaN, "bfloat_qNaN");
		to_binary(bfloat_qNaN);

	}

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
