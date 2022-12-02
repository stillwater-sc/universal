// api.cpp: application programming interface tests for bfloat16 number system
//
// Copyright (C) 2022-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the cfloat template environment
// enable/disable arithmetic exceptions
#define BFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/bfloat/bfloat.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "bfloat16 Application Programming Interface tests";
	int nrOfFailedTestCases = 0;

	{
		bfloat16 a;
		a = 1.0f;
		std::cout << to_binary(a) << " : " << a << '\n';
	}


	// important behavioral traits
	{
		using TestType = bfloat16;
		ReportTrivialityOfType<TestType>();
	}

	// default behavior
	std::cout << "+---------    Default bfloat has subnormals, but no supernormals\n";
	{
		using Real = bfloat16;

		Real a(1.0f), b(0.5f);
		ArithmeticOperators(a, b);
	}
	return 0;

	// report on the dynamic range of some standard configurations
	std::cout << "+---------    Dynamic ranges of standard bfloat16 configurations   --------+\n";
	{
		// quarter, half, single, duble, quad, and octo precision IEEE-754 style floating-point
		std::cout << "quarter  precision: " << cfloat_range<quarter>() << '\n';
		std::cout << "half     precision: " << cfloat_range<half>() << '\n';
		std::cout << "single   precision: " << cfloat_range<single>() << '\n';
		std::cout << "double   precision: " << cfloat_range<duble>() << '\n';
		std::cout << "---\n";

		bfloat16 bf; // uninitialized
		bf.setbits(0x01);  // smallest subnormal
		std::cout << "minpos  bfloat16 : " << to_binary(bf) << " : " << bf << '\n';
		bf.setbits(0x5f);  // max normal
		std::cout << "maxnorm bfloat16 : " << to_binary(bf) << " : " << bf << '\n';
		bf.setbits(0x7d);  // max supernormal
		std::cout << "maxpos  bfloat16 : " << to_binary(bf) << " : " << bf << '\n';

		bf.minpos();
		std::cout << "minpos bfloat16  : " << to_binary(bf) << " : " << bf << '\n';
		bf.maxpos();
		std::cout << "maxpos bfloat16  : " << to_binary(bf) << " : " << bf << '\n';

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

		a.setbits(0xAAAA);
		std::cout << to_binary(a) << " : " << a << '\n';

		a.assign(std::string("0b1.01010.1010'1010'10"));
		std::cout << to_binary(a) << " : " << a << '\n';

		a.assign(std::string("0b1.01010.10'1010'1010"));
		std::cout << to_binary(a) << " : " << a << '\n';
	}

	std::cout << "+---------    set specific values of interest   --------+\n";
	{
		bfloat16 a; // uninitialized
		std::cout << "maxpos : " << a.maxpos() << " : " << scale(a) << '\n';
		std::cout << "minpos : " << a.minpos() << " : " << scale(a) << '\n';
		std::cout << "zero   : " << a.zero() << " : " << scale(a) << '\n';
		std::cout << "minneg : " << a.minneg() << " : " << scale(a) << '\n';
		std::cout << "maxneg : " << a.maxneg() << " : " << scale(a) << '\n';
		std::cout << dynamic_range(a) << std::endl;
	}

	std::cout << "+---------    cfloat<16, 5, uint32_t, hasSubnormals, noSupernormals, notSaturating>         half-precision subnormals   --------+\n";
	{
		constexpr size_t nbits = 16;
		constexpr size_t es = 5;
		using BlockType = uint32_t;
		using Cfloat = cfloat<nbits, es, BlockType, true>;
		constexpr size_t fbits = Cfloat::fbits;
		Cfloat a, b; // uninitialized

		// enumerate the subnormals
		uint32_t pattern = 1ul;
		std::streamsize precision = std::cout.precision();
		std::cout << std::setw(nbits) << "binary" << " : " << std::setw(nbits) << "native" << " : " << std::setw(nbits) << "conversion\n";
//		std::cout << std::setprecision(3);
		std::cout << std::fixed;
		for (unsigned i = 0; i < fbits; ++i) {
			a.setbits(pattern);
			std::cout << color_print(a) << " : " << std::setw(nbits) << a << " : " << std::setw(nbits) << float(a) << '\n';
			pattern <<= 1;
		}
		// enumerate the normals
		a.setbits(0x0400);
		for (size_t i = 0; i < 30; ++i) {
			std::cout << color_print(a) << " : " << std::setw(nbits) << a << " : " << std::setw(nbits) << float(a) << " + 1ULP ";
			b = a; ++b;
			std::cout << color_print(b) << " : " << std::setw(nbits) << b << " : " << std::setw(nbits) << float(b) << '\n';
			a *= 2;
		}
		std::cout << std::setprecision(precision);
		std::cout << std::scientific;
	}
	std::cout << "+---------    bfloat16   --------+\n";
	{
		float subnormal = std::nextafter(0.0f, 1.0f);
		using Bfloat = bfloat16;
		Bfloat a; // uninitialized

		uint16_t pattern = 0x0001ul;
		for (unsigned i = 0; i < 8; ++i) {
			a.setbits(pattern);
			std::cout << to_binary(a) << " " << a << ": ";
			pattern <<= 1;
			std::cout << color_print(subnormal) << " : " << subnormal << std::endl;
			subnormal *= 2.0f;

			if (i < 8) { // the last iteration is a normal encoding
				constexpr bool isNormal = false;
//				int scale_offset = static_cast<int>(a.significant(significant, isNormal)); // significant will be in leading 1 format, so not interesting unless you are doing arithmetic
//				int check = a.MIN_EXP_NORMAL - scale_offset;
//				std::cout << a.MIN_EXP_NORMAL << " - " << scale_offset << " = (" << check << ") should be equal to " << a.scale() << std::endl;
			}
		}
	}

	std::cout << "+---------    Subnormal exponent values   --------+\n";
	{
		// we are not using element [0] as es = 0 is not supported in the cfloat spec
		int exponents[] = {
			0, 1, 0, -2, -6, -14, -30, -62, -126, -254, -510, -1022
		};
		for (int i = 1; i < 12; ++i) {
			std::cout << "es = " << i << " = " << exponents[i] << " " << std::setprecision(17) << subnormal_exponent[i] << std::endl;
		}
	}

	std::cout << "+---------    special value properties bfloat16 vs IEEE754   --------+\n";
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

		bfloat16 a(fa);
		if ((a < 0.0f && a > 0.0f && a != 0.0f) || a.isneg()) {
			std::cout << "bfloat16 is incorrectly implemented\n";
			++nrOfFailedTestCases;
		}
		else {
			std::cout << "bfloat16 NAN has no sign\n";
		}
	}

	{
		using cfloat = sw::universal::cfloat<32, 8, uint32_t, true, false, false>;

		std::cout << "cfloat(INFINITY): " << cfloat(INFINITY) << "\n";
		std::cout << "cfloat(-INFINITY): " << cfloat(-INFINITY) << "\n";

		std::cout << "cfloat(std::numeric_limits<float>::infinity())  : " << cfloat(std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "cfloat(-std::numeric_limits<float>::infinity()) : " << cfloat(-std::numeric_limits<float>::infinity()) << "\n";

		std::cout << " 2 * std::numeric_limits<float>::infinity()  : " << 2 * std::numeric_limits<float>::infinity() << "\n";
		std::cout << " 2 * std::numeric_limits<cfloat>::infinity() : " << 2 * std::numeric_limits<cfloat>::infinity() << "\n";
		std::cout << "-2 * std::numeric_limits<cfloat>::infinity() : " << -2 * std::numeric_limits<cfloat>::infinity() << "\n";

		std::cout << "sw::universal::nextafter(cfloat(0), std::numeric_limits<cfloat>::infinity())  : " << sw::universal::nextafter(cfloat(-0), std::numeric_limits<cfloat>::infinity()) << "\n";
		std::cout << "std::nextafter(float(0), std::numeric_limits<float>::infinity())              : " << std::nextafter(float(-0), std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "sw::universal::nextafter(cfloat(0), -std::numeric_limits<cfloat>::infinity()) : " << sw::universal::nextafter(cfloat(0), -std::numeric_limits<cfloat>::infinity()) << "\n";
		std::cout << "std::nextafter(float(0), -std::numeric_limits<float>::infinity())             : " << std::nextafter(float(0), -std::numeric_limits<float>::infinity()) << "\n";

		std::cout << "cfloat(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_QUIET)      : " << cfloat(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_QUIET) << "\n";
		std::cout << "cfloat(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) : " << cfloat(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) << "\n";
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
