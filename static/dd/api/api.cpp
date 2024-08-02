// api.cpp: application programming interface tests for doubledouble number system
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

		template<typename Real>
		void Progression(Real v) {
			using namespace sw::universal;

			int oldPrec = std::cout.precision();
			float f{ float(v) };
			std::cout << std::setprecision(7);
			std::cout << to_binary(f, true) << " : " << f << '\n';

			double d{ v };
			std::cout << std::setprecision(17);
			std::cout << to_binary(d, true) << " : " << d << '\n';

			dd a{ v };
			std::cout << std::setprecision(35);
			std::cout << to_binary(a, true) << " : " << a << '\n';
			std::cout << std::setprecision(oldPrec);
		}

		dd parse(const std::string& str) {
			using namespace sw::universal;

			dd v(str);
			auto oldPrec = std::cout.precision();
			std::cout << std::setprecision(std::numeric_limits<double>::digits10);
			std::cout << "string: " << str << " = ( " << v.high() << ", " << v.low() << ") ";
			std::cout << std::setprecision(oldPrec);
			return v;
		}

		void print(std::ostream& ostr, dd const& v) {
			bool showpos = (ostr.flags() & std::ios_base::showpos) != 0;
			bool uppercase = (ostr.flags() & std::ios_base::uppercase) != 0;

			std::string str = v.to_string(ostr.precision(), ostr.width(), ostr.flags(), showpos, uppercase, ostr.fill());
			ostr << str << '\n';
		}

	}
}




int main()
try {
	using namespace sw::universal;

	std::string test_suite = "doubledouble (dd) API tests";
	int nrOfFailedTestCases = 0;

	auto oldPrec = std::cout.precision();

	// important behavioral traits
	{
		using TestType = dd;
		ReportTrivialityOfType<TestType>();
	}

	// default behavior
	std::cout << "+---------    Default dd has subnormals, but no supernormals\n";
	{
		uint64_t big = (1ull << 53);
		std::cout << to_binary(big) << " : " << big << '\n';
		dd a(big), b(1.0), c{};
		c = a + b;
		ReportValue(a, "a");
		ReportValue(b, "b");
		ReportValue(c, "c");
	}

	// arithmetic behavior
	std::cout << "+---------    Default dd has subnormals, but no supernormals\n";
	{
		dd a(2.0), b(4.0);
		ArithmeticOperators(a, b);
	}

	std::cout << "+---------    fraction bit progressions \n";
	{
		float fulp = ulp(1.0f);
		Progression(1.0f + fulp);
		Progression(1.0 + ulp(2.0));
		double v = ulp(1.0);
		Progression( 1.0 - v/2.0 );
		std::cout << to_pair(dd(1.0 - v / 2.0)) << '\n';
	}

	// report on the dynamic range of some standard configurations
	std::cout << "+---------    Dynamic range doubledouble configurations   --------+\n";
	{
		dd a; // uninitialized

		a.maxpos();
		std::cout << "maxpos  doubledouble : " << to_binary(a) << " : " << a << '\n';
		a.setbits(0x0080);  // positive min normal
		std::cout << "minnorm doubledouble : " << to_binary(a) << " : " << a << '\n';
		a.minpos();
		std::cout << "minpos  doubledouble : " << to_binary(a) << " : " << a << '\n';
		a.zero();
		std::cout << "zero                 : " << to_binary(a) << " : " << a << '\n';
		a.minneg();
		std::cout << "minneg  doubledouble : " << to_binary(a) << " : " << a << '\n';
		a.maxneg();
		std::cout << "maxneg  doubledouble : " << to_binary(a) << " : " << a << '\n';

		std::cout << "---\n";
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

	// parse decimal strings
	std::cout << "+---------    parse API   --------+\n";
	{
		std::string ddstr;
		dd v;

		v = parse("0.0");
		ddstr = v.to_string(25, 25, std::cout.flags(), true, false, ' ');
		std::cout << ddstr << '\n';

		std::cout << std::setprecision(7);
		print(std::cout, parse("0.5"));
		print(std::cout, parse("1.0"));
		print(std::cout, parse("2.0"));

		// 100 digits of e
		//  10 2.7182818284
		//  20 2.71828182845904523536
		//  30 2.718281828459045235360287471352
		//  40 2.7182818284590452353602874713526624977572
		//  50 2.71828182845904523536028747135266249775724709369995
		//  60 2.718281828459045235360287471352662497757247093699959574966967
		//  70 2.7182818284590452353602874713526624977572470936999595749669676277240766
		//  80 2.71828182845904523536028747135266249775724709369995957496696762772407663035354759
		//  90 2.718281828459045235360287471352662497757247093699959574966967627724076630353547594571382178
		// 100 2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274
		ReportValue(std::numbers::e, "e", 10, 25);
		std::cout << std::setprecision(10);
		print(std::cout, parse("2.7182818284")); // 10 digits
		std::cout << std::setprecision(15);
		print(std::cout, parse("2.71828182845904")); // 15 digits
		std::cout << std::setprecision(20);
		print(std::cout, parse("2.71828182845904523536")); // 20 digits
		std::cout << std::setprecision(30);
		print(std::cout, parse("2.718281828459045235360287471352")); // 30 digits
		std::cout << std::setprecision(40);
		print(std::cout, parse("2.7182818284590452353602874713526624977572")); // 40 digits

		std::cout << std::setprecision(37);
		print(std::cout, parse("2.718281828459045235360287471352662498")); //37 digits
		std::cout << std::setprecision(oldPrec);
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

	std::cout << "+---------    doubledouble subnormal behavior   --------+\n";
	{
		constexpr double minpos = std::numeric_limits<double>::min();
		std::cout << to_binary(minpos) << " : " << minpos << '\n';
		double subnormal = minpos / 2.0;
		std::cout << to_binary(subnormal) << " : " << subnormal << '\n';
		dd a(minpos);
		for (int i = 0; i < 10/*106*/; ++i) {
			std::string str = a.to_string(30, 40, std::cout.flags(), false, false, ' ');
			std::cout << to_binary(a) << " : " << a << " : " << str << '\n';
			a /= 2.0;
		}
	}

	std::cout << "+---------    special value properties doubledouble vs IEEE-754   --------+\n";
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

	std::cout << "+---------    numeric_limits of doubledouble vs IEEE-754   --------+\n";
	{
		std::cout << "dd(INFINITY): " << dd(INFINITY) << "\n";
		std::cout << "dd(-INFINITY): " << dd(-INFINITY) << "\n";

		std::cout << "dd(std::numeric_limits<float>::infinity())  : " << dd(std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "dd(-std::numeric_limits<float>::infinity()) : " << dd(-std::numeric_limits<float>::infinity()) << "\n";

		std::cout << " 2 * std::numeric_limits<float>::infinity()  : " << 2 * std::numeric_limits<float>::infinity() << "\n";
		std::cout << " 2 * std::numeric_limits<double>::infinity() : " << 2 * std::numeric_limits<double>::infinity() << "\n";
		std::cout << "-2 * std::numeric_limits<dd>::infinity()     : " << -2 * std::numeric_limits<dd>::infinity() << "\n";

		std::cout << "sw::universal::nextafter(dd(0), std::numeric_limits<dd>::infinity())  : " << sw::universal::nextafter(dd(-0), std::numeric_limits<dd>::infinity()) << "\n";
		std::cout << "std::nextafter(float(0), std::numeric_limits<float>::infinity())              : " << std::nextafter(float(-0), std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "sw::universal::nextafter(dd(0), -std::numeric_limits<dd>::infinity()) : " << sw::universal::nextafter(dd(0), -std::numeric_limits<dd>::infinity()) << "\n";
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
