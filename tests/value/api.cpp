// api.cpp: functional tests of the value type API
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/bit_cast.hpp>
#include <limits>
#include <universal/native/integers.hpp>
// configure the value<> environment
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#define VALUE_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/internal/value/value.hpp>  // TODO remove: INTERNAL class: not part of the public Universal API
#include <universal/number/decimal/decimal.hpp>
#include <universal/number/integer/integer.hpp>         // TODO remove: temporary as we are developing the string conversion functionality
#include <universal/number/adaptiveint/adaptiveint.hpp> // TODO remove: temporary
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/native/ieee754.hpp>

using namespace sw::universal;
using namespace sw::universal::internal;

template<unsigned fbits>
int Check(const value<fbits>& v, double ref, bool reportTestCases) {
	int fails = 0;
	if (v.to_double() != ref) {
		++fails;
		if (reportTestCases) {
			std::cout << v << " != " << ref << '\n';
		}
	}
	return fails;
}

// check all native type conversions
int CheckConversion(bool reportTestCases) { 
	constexpr double reference = 8;
	signed char        sc = (signed char)reference;
	short              ss = (short)reference;
	int                si = (int)reference;
	long               sl = (long)reference;
	long long          sll = (long long)reference;
	char               uc = (char)reference;
	unsigned short     us = (unsigned short)reference;
	unsigned int       ui = (unsigned int)reference;
	unsigned long      ul = (unsigned long)reference;
	unsigned long long ull = (unsigned long long)reference;
	float              f = (float)reference;
	double             d = (double)reference;
	long double        ld = (long double)reference;

	int nrOfFailedTestCases = 0;
	sw::universal::internal::value<11> v;
	v = sc;
	nrOfFailedTestCases += Check(v, reference, reportTestCases);
	v = ss;
	nrOfFailedTestCases += Check(v, reference, reportTestCases);
	v = si;
	nrOfFailedTestCases += Check(v, reference, reportTestCases);
	v = sl;
	nrOfFailedTestCases += Check(v, reference, reportTestCases);
	v = sll;
	nrOfFailedTestCases += Check(v, reference, reportTestCases);
	v = uc;
	nrOfFailedTestCases += Check(v, reference, reportTestCases);
	v = us;
	nrOfFailedTestCases += Check(v, reference, reportTestCases);
	v = ui;
	nrOfFailedTestCases += Check(v, reference, reportTestCases);
	v = ul;
	nrOfFailedTestCases += Check(v, reference, reportTestCases);
	v = ull;
	nrOfFailedTestCases += Check(v, reference, reportTestCases);
	v = f;
	nrOfFailedTestCases += Check(v, reference, reportTestCases);
	v = d;
	nrOfFailedTestCases += Check(v, reference, reportTestCases);
	v = ld;
	nrOfFailedTestCases += Check(v, reference, reportTestCases);

	return nrOfFailedTestCases;
}

template<typename Real,
	typename = typename std::enable_if<std::is_floating_point<Real>::value, Real>::type>
void ShowComponentsOfNativeReal(Real fp) {
	constexpr unsigned fbits = sw::universal::ieee754_parameter<Real>::fbits;
	auto components = ieee_components(fp);
	auto oldPrecision = std::cout.precision();
	auto max_digits = std::numeric_limits<long double>::digits10 + 1;
	std::cout << std::setprecision(std::numeric_limits<Real>::digits10);
	std::cout << "components of a " 
		<< std::setw(25) << typeid(Real).name() << " : " 
		<< std::setw(max_digits) << fp 
		<< " : (" << std::get<0>(components) << ", " << std::get<1>(components) << ", " << to_binary(std::get<2>(components), fbits, true) << ")\n";
	std::cout << std::setprecision(oldPrecision);
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::internal;

	std::string test_suite  = "value class API";
	std::string test_tag    = "value";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';
	std::cout << (reportTestCases ? " " : "not ") << "reporting individual testcases\n";

#if MANUAL_TESTING

	nrOfFailedTestCases += CheckConversion(reportTestCases);

	long double fp = 1.234567890123456789012345l;
	ShowComponentsOfNativeReal<float>(static_cast<float>(fp));
	ShowComponentsOfNativeReal<double>(static_cast<double>(fp));
	ShowComponentsOfNativeReal<long double>(fp);

	//1.000'000'000'1 * 2 ^ -10 = 1 / 1024 + 1 / 1M
	//1.000'000'000'1 * 2 ^ 0 = 1 + 1 / 1024 = 0.0009765625
	//1.000'000'000'1 * 2 ^ 10 = 1025
	/*
	
	(+,-10,0b000'0000'0010'0000'0000'0000) : 0.000977516174316406
	(+,-10,0b000'0000'0010'0000'0000'0000) : 0.000977516174316406
	(+,  0,0b000'0000'0010'0000'0000'0000) : 1.0009765625
	(+,  0,0b000'0000'0010'0000'0000'0000) : 1.0009765625
	(+, 10,0b000'0000'0010'0000'0000'0000) : 1025
	(+, 10,0b000'0000'0010'0000'0000'0000) : 1025
	*/
	{ 
		auto oldPrecision = std::cout.precision();
		std::cout << std::setprecision(15);
		value<23> a;
		bitblock<23> bb;
		float f;

		bb.set(13, true);
		std::cout << bb << '\n';

		a.set(false, -10, bb, false, false, false);
		std::cout << to_triple(a) << " : " << (float)a << '\n';
		f = (1.0 + 1.0 / 1024) / 1024;
		std::cout << to_triple(f, true) << " : " << f << '\n';

		a.set(false, 0, bb, false, false, false);
		std::cout << to_triple(a) << " : " << (float)a << '\n';
		f = 1.0 + 1.0 / 1024;
		std::cout << to_triple(f, true) << " : " << f << '\n';

		a.set(false, 10, bb, false, false, false);
		std::cout << to_triple(a) << " : " << (float)a << '\n';
		f = (1.0 + 1.0 / 1024) * 1024;
		std::cout << to_triple(f, true) << " : " << f << '\n';

		// reset the stream
		std::cout << std::setprecision(oldPrecision);
	}

#define LATER_
#ifdef LATER
	{
		integer<512, std::uint32_t> a, b, c;

		a.assign("123456789012345678901234567890123456789012345678901234567890");
		b = 10;
		c = a + b;
		std::cout << "a : " << a << '\n';
		std::cout << "b : " << b << '\n';
		std::cout << "c : " << c << '\n';

		a = 1024;
		while (a > 0) {
			a *= 2;
			std::cout << a << '\n';
		}
		while (a < 0) {
			a /= 2;
			std::cout << a << '\n';
		}
	}
#endif

	{
		double d = 3.14159265358979323846264338327950;
		std::cout << std::scientific << d << '\n';

		float f = 3.1415926535f;
		std::cout << std::fixed << f << '\n';
		std::cout.unsetf(std::ios_base::fixed);
		std::cout << f << '\n';

		uint32_t i = 0xAAAA;
		std::cout << std::showbase;
		std::cout << std::oct << i << '\n'; // shows 0125252
		std::cout << std::hex << i << '\n'; // shows 0xaaaa
		std::cout << std::dec << i << '\n'; // shows 43690
		std::cout.unsetf(std::ios_base::showbase);
		std::cout << std::oct << i << '\n'; // shows 125252
		std::cout << std::hex << i << '\n'; // shows aaaa
		std::cout << std::dec << i << '\n'; // shows 43690	
			
			
		value<23> v;
		v.setnan();
		std::cout << v << '\n';
		v.setinf();
		std::cout << v << '\n';
		v.setsign(false);
		std::cout << v << '\n';
		std::cout << std::showpos << v << '\n';
		std::cout.unsetf(std::ios_base::showpos);
		v = f;
		std::cout << v << '\n';                     // flags: 0b0000'0010'0000'0001
		std::cout << std::scientific << v << '\n';  // flags: 0b0001'0010'0000'0001
		std::cout << std::fixed << v << '\n';       // flags: 0b0010'0010'0000'0001
		std::cout << std::hexfloat << v << '\n';    // flags: 0b0011'0010'0000'0001
		std::cout << std::defaultfloat << v << '\n';// flags: 0b0000'0010'0000'0001
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1

#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
