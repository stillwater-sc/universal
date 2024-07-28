// api.cpp: application programming interface tests for doubledouble number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the dd template environment
// enable/disable arithmetic exceptions
#define DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/dd/dd.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "doubledouble (dd) API tests";
	int nrOfFailedTestCases = 0;

	{
		dd a;
		a = 1.0f;
		std::cout << to_binary(a) << " : " << a << '\n';
	}

	// important behavioral traits
	{
		using TestType = dd;
		ReportTrivialityOfType<TestType>();
	}

	// default behavior
	std::cout << "+---------    Default dd has subnormals, but no supernormals\n";
	{
		using Real = dd;

		Real a(1.0f), b(0.5f);
		ArithmeticOperators(a, b);
	}

	// report on the dynamic range of some standard configurations
	std::cout << "+---------    Dynamic ranges of standard bfloat16 configurations   --------+\n";
	{
		dd a; // uninitialized

		a.maxpos();
		std::cout << "maxpos  aloat16 : " << to_binary(a) << " : " << a << '\n';
		a.setbits(0x0080);  // positive min normal
		std::cout << "minnorm aloat16 : " << to_binary(a) << " : " << a << '\n';
		a.minpos();
		std::cout << "minpos  aloat16 : " << to_binary(a) << " : " << a << '\n';
		a.zero();
		std::cout << "zero             : " << to_binary(a) << " : " << a << '\n';
		a.minneg();
		std::cout << "minneg  aloat16 : " << to_binary(a) << " : " << a << '\n';
		a.setbits(0x8080);  // negative min normal
		std::cout << "minnegnorm       : " << to_binary(a) << " : " << a << '\n';
		a.maxneg();
		std::cout << "maxneg  aloat16 : " << to_binary(a) << " : " << a << '\n';

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

		dd b1(f1), b2(f2), b3;
		b3 = b1 / b2;
		std::cout << "dd       : " << type_tag(b3) << '\n';
		std::cout << b1 << " / " << b2 << " = " << b3 << " : " << to_binary(b3) << '\n';

	}

	// constexpr and specific values
	std::cout << "+---------    constexpr and specific values   --------+\n";
	{
		using Real = dd;

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
		using Real = dd;

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
		dd a{ 0 }; // initialized
		std::cout << "maxpos : " << a.maxpos() << " : " << scale(a) << '\n';
		std::cout << "minpos : " << a.minpos() << " : " << scale(a) << '\n';
		std::cout << "zero   : " << a.zero()   << " : " << scale(a) << '\n';
		std::cout << "minneg : " << a.minneg() << " : " << scale(a) << '\n';
		std::cout << "maxneg : " << a.maxneg() << " : " << scale(a) << '\n';
		std::cout << dynamic_range<dd>() << std::endl;
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
	std::cout << "+---------    doubledouble bit progressions   --------+\n";
	{
		constexpr unsigned nbits = 128;
		constexpr unsigned fbits = 53;
		using Real = dd;
		Real a, b; // uninitialized

		std::streamsize precision = std::cout.precision();

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

		dd a(fa);
		if ((a < 0.0f && a > 0.0f && a != 0.0f)) {
			std::cout << "doubledouble (dd) is incorrectly implemented\n";
			++nrOfFailedTestCases;
		}
		else {
			std::cout << "dd NAN has no sign\n";
		}
	}

	{
		std::cout << "dd(INFINITY): " << dd(INFINITY) << "\n";
		std::cout << "dd(-INFINITY): " << dd(-INFINITY) << "\n";

		std::cout << "dd(std::numeric_limits<float>::infinity())  : " << dd(std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "dd(-std::numeric_limits<float>::infinity()) : " << dd(-std::numeric_limits<float>::infinity()) << "\n";

		std::cout << " 2 * std::numeric_limits<float>::infinity()  : " << 2 * std::numeric_limits<float>::infinity() << "\n";
		std::cout << " 2 * std::numeric_limits<double>::infinity() : " << 2 * std::numeric_limits<double>::infinity() << "\n";
		std::cout << "-2 * std::numeric_limits<dd>::infinity()     : " << -2 * std::numeric_limits<dd>::infinity() << "\n";

//		std::cout << "sw::universal::nextafter(dd(0), std::numeric_limits<dd>::infinity())  : " << sw::universal::nextafter(dd(-0), std::numeric_limits<dd>::infinity()) << "\n";
		std::cout << "std::nextafter(float(0), std::numeric_limits<float>::infinity())              : " << std::nextafter(float(-0), std::numeric_limits<float>::infinity()) << "\n";
//		std::cout << "sw::universal::nextafter(bfloat16(0), -std::numeric_limits<bfloat16>::infinity()) : " << sw::universal::nextafter(dd(0), -std::numeric_limits<dd>::infinity()) << "\n";
		std::cout << "std::nextafter(float(0), -std::numeric_limits<float>::infinity())             : " << std::nextafter(float(0), -std::numeric_limits<float>::infinity()) << "\n";

		std::cout << "cfloat(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_QUIET)      : " << dd(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_QUIET) << "\n";
		std::cout << "cfloat(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) : " << dd(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) << "\n";
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
