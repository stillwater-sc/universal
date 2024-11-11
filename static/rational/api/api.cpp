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

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "rational<16,uint16_t> API tests";
	int nrOfFailedTestCases = 0;

	{
		rational<16,uint16_t> a;
		a = 1.0f;
		std::cout << to_binary(a) << " : " << a << '\n';
	}

	// important behavioral traits
	{
		using TestType = rational<16,uint16_t>;
		ReportTrivialityOfType<TestType>();
	}

	// default behavior
	std::cout << "+---------    Default rational<16,uint16_t> has subnormals, but no supernormals\n";
	{
		using Real = rational<16,uint16_t>;

		Real a(1.0f), b(0.5f);
		ArithmeticOperators(a, b);
	}

	// report on the dynamic range of some standard configurations
	std::cout << "+---------    Dynamic ranges of standard rational<16,uint16_t> configurations   --------+\n";
	{
		rational<16,uint16_t> bf; // uninitialized

		bf.maxpos();
		std::cout << "maxpos  rational<16,uint16_t> : " << to_binary(bf) << " : " << bf << '\n';
		bf.setbits(0x0080);  // positive min normal
		std::cout << "minnorm rational<16,uint16_t> : " << to_binary(bf) << " : " << bf << '\n';
		bf.minpos();
		std::cout << "minpos  rational<16,uint16_t> : " << to_binary(bf) << " : " << bf << '\n';
		bf.zero();
		std::cout << "zero             : " << to_binary(bf) << " : " << bf << '\n';
		bf.minneg();
		std::cout << "minneg  rational<16,uint16_t> : " << to_binary(bf) << " : " << bf << '\n';
		bf.setbits(0x8080);  // negative min normal
		std::cout << "minnegnorm       : " << to_binary(bf) << " : " << bf << '\n';
		bf.maxneg();
		std::cout << "maxneg  rational<16,uint16_t> : " << to_binary(bf) << " : " << bf << '\n';

		std::cout << "---\n";
	}


	// constexpr and specific values
	std::cout << "+---------    constexpr and specific values   --------+\n";
	{
		using Real = rational<16,uint16_t>;

		CONSTEXPRESSION Real a{}; // zero constexpr
		std::cout << type_tag(a) << '\n';

		 Real b(1.0f);  // constexpr of a native type conversion
		std::cout << to_binary(b) << " : " << b << '\n';

		CONSTEXPRESSION Real c(SpecificValue::minpos);  // constexpr of a special value in the encoding
		std::cout << to_binary(c) << " : " << c << " == minpos" << '\n';

		CONSTEXPRESSION Real d(SpecificValue::maxpos);  // constexpr of a special value in the encoding
		std::cout << to_binary(d) << " : " << d << " == maxpos" << '\n';
	}

/*
	std::cout << "+---------    set specific values of interest   --------+\n";
	{
		rational<16,uint16_t> a{ 0 }; // initialized
		std::cout << "maxpos : " << a.maxpos() << " : " << scale(a) << '\n';
		std::cout << "minpos : " << a.minpos() << " : " << scale(a) << '\n';
		std::cout << "zero   : " << a.zero()   << " : " << scale(a) << '\n';
		std::cout << "minneg : " << a.minneg() << " : " << scale(a) << '\n';
		std::cout << "maxneg : " << a.maxneg() << " : " << scale(a) << '\n';
		std::cout << dynamic_range<rational<16,uint16_t>>() << std::endl;
	}

	std::cout << "+---------    rational<16,uint16_t>   --------+\n";
	{
		using Bfloat = rational<16,uint16_t>;
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

	std::cout << "+---------    special value properties rational<16,uint16_t> vs IEEE-754   --------+\n";
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

		rational<16,uint16_t> a(fa);
		if ((a < 0.0f && a > 0.0f && a != 0.0f)) {
			std::cout << "rational<16,uint16_t> is incorrectly implemented\n";
			++nrOfFailedTestCases;
		}
		else {
			std::cout << "rational<16,uint16_t> NAN has no sign\n";
		}
	}

	{
		std::cout << "rational<16,uint16_t>(INFINITY): " << rational<16,uint16_t>(INFINITY) << "\n";
		std::cout << "rational<16,uint16_t>(-INFINITY): " << rational<16,uint16_t>(-INFINITY) << "\n";

		std::cout << "rational<16,uint16_t>(std::numeric_limits<float>::infinity())  : " << rational<16,uint16_t>(std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "rational<16,uint16_t>(-std::numeric_limits<float>::infinity()) : " << rational<16,uint16_t>(-std::numeric_limits<float>::infinity()) << "\n";

		std::cout << " 2 * std::numeric_limits<float>::infinity()  : " << 2 * std::numeric_limits<float>::infinity() << "\n";
		std::cout << " 2 * std::numeric_limits<rational<16,uint16_t>>::infinity() : " << 2 * std::numeric_limits<rational<16,uint16_t>>::infinity() << "\n";
		std::cout << "-2 * std::numeric_limits<rational<16,uint16_t>>::infinity() : " << -2 * std::numeric_limits<rational<16,uint16_t>>::infinity() << "\n";

		std::cout << "sw::universal::nextafter(rational<16,uint16_t>(0), std::numeric_limits<rational<16,uint16_t>>::infinity())  : " << sw::universal::nextafter(rational<16,uint16_t>(-0), std::numeric_limits<rational<16,uint16_t>>::infinity()) << "\n";
		std::cout << "std::nextafter(float(0), std::numeric_limits<float>::infinity())                  : " << std::nextafter(float(-0), std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "sw::universal::nextafter(rational<16,uint16_t>(0), -std::numeric_limits<rational<16,uint16_t>>::infinity()) : " << sw::universal::nextafter(rational<16,uint16_t>(0), -std::numeric_limits<rational<16,uint16_t>>::infinity()) << "\n";
		std::cout << "std::nextafter(float(0), -std::numeric_limits<float>::infinity())                 : " << std::nextafter(float(0), -std::numeric_limits<float>::infinity()) << "\n";

		std::cout << "rational<16,uint16_t>(std::numeric_limits<rational<16,uint16_t>>::quiet_NaN()).isnan(sw::universal::NAN_TYPE_QUIET)          : " << rational<16,uint16_t>(std::numeric_limits<rational<16,uint16_t>>::quiet_NaN()).isnan(sw::universal::NAN_TYPE_QUIET) << "\n";
		std::cout << "rational<16,uint16_t>(std::numeric_limits<rational<16,uint16_t>>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) : " << rational<16,uint16_t>(std::numeric_limits<rational<16,uint16_t>>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) << "\n";
		std::cout << "rational<16,uint16_t>(std::numeric_limits<float>::quiet_NaN()).isnan(sw::universal::NAN_TYPE_QUIET)             : " << rational<16,uint16_t>(std::numeric_limits<float>::quiet_NaN()).isnan(sw::universal::NAN_TYPE_QUIET) << "\n";
		std::cout << "rational<16,uint16_t>(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING)    : " << rational<16,uint16_t>(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) << "\n";

		float float_sNaN{ std::numeric_limits<float>::signaling_NaN() };
		ReportValue(float_sNaN, "float_sNaN");
		rational<16,uint16_t> rational_sNaN{ float_sNaN };
		ReportValue(rational_sNaN, "rational_sNaN");
		to_binary(rational_sNaN);

		float float_qNaN{ std::numeric_limits<float>::quiet_NaN() };
		ReportValue(float_qNaN, "float_qNaN");
		rational<16,uint16_t> rational_qNaN{ float_qNaN };
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
