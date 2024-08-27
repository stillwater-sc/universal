// api.cpp: application programming interface tests for quad-double (qd) number system
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
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/common/string_utils.hpp>

namespace sw {
	namespace universal {

		void ReportValue(const qd& a, const std::string& label = "", unsigned labelWidth = 20, unsigned precision = 7) {
			auto defaultPrecision = std::cout.precision();
			std::cout << std::setprecision(precision);
			std::cout << std::setw(labelWidth) << label << " : " <<  a << '\n';
			std::cout << to_quad(a) << '\n';
			std::cout << std::setprecision(defaultPrecision);
		}

		void ReportQuadDoubleOperation(const qd& a, const std::string& op, const qd& b, const qd& c, int precision = 64) {
			auto defaultPrecision = std::cout.precision();
			std::cout << std::setprecision(precision);
			std::cout << a << op << b << " = " << c << '\n';
			std::cout << std::setprecision(defaultPrecision);
		}

		template<typename Real>
		void Progression(Real v) {
			using namespace sw::universal;

			auto defaultPrecision = std::cout.precision();
			float f{ float(v) };
			std::cout << std::setprecision(7);
			std::cout << to_binary(f, true) << " : " << f << '\n';

			double d{ v };
			std::cout << std::setprecision(17);
			std::cout << to_binary(d, true) << " : " << d << '\n';

			qd a{ v };
			std::cout << std::setprecision(35);
			std::cout << to_binary(a, true) << " : " << a << '\n';
			std::cout << std::setprecision(defaultPrecision);
		}

		qd parse(const std::string& str) {
			using namespace sw::universal;

			qd v(str);
			auto defaultPrecision = std::cout.precision();
			std::cout << std::setprecision(std::numeric_limits<double>::digits10);
			std::cout << "string: " << str << " = ( " << v[0] << ", " << v[1] << ") ";
			std::cout << std::setprecision(defaultPrecision);
			return v;
		}

		void print(std::ostream& ostr, qd const& v) {
			std::ios_base::fmtflags fmt = ostr.flags();
			bool showpos = (fmt & std::ios_base::showpos) != 0;
			bool uppercase = (fmt & std::ios_base::uppercase) != 0;
			bool fixed = (fmt & std::ios_base::fixed) != 0;
			bool scientific = (fmt & std::ios_base::scientific) != 0;
			bool internal = (fmt & std::ios_base::internal) != 0;
			bool left = (fmt & std::ios_base::left) != 0;
			std::string str = v.to_string(ostr.precision(), ostr.width(), fixed, scientific, internal, left, showpos, uppercase, ostr.fill());
			ostr << str << '\n';
		}

	}
}


int main()
try {
	using namespace sw::universal;

	std::string test_suite = "quad-double (qd) API tests";
	int nrOfFailedTestCases = 0;

	auto defaultPrecision = std::cout.precision();

	// important behavioral traits
	{
		using TestType = qd;
		ReportTrivialityOfType<TestType>();
	}

	// default behavior
	std::cout << "+---------    Default quad-double bheavior   ----------+\n";
	{
		double big = std::pow(2.0, 3*53);
		ReportValue(big, "2^159", 20);
		big = std::pow(2.0, 4 * 53);
		ReportValue(big, "2^212", 20);
		// if we use double, we would not be able to capture the information of the variable b == 1.0 in the sum of a + b
		{
			double a(big), b(1.0), c{};
			c = a + b;
			ReportValue(a, "a as double", 20, 16);
			ReportValue(b, "b as double", 20, 16);
			ReportValue(c, "c as double", 20, 16);
		}
		// the extra precision of the double-double makes it possible to use that information
		{
			qd a(big), b(1.0), c{};
			c = a + b;
			ReportValue(a, "a as quad-double", 20, 64);
			ReportValue(b, "b as quad-double", 20, 64);
			ReportValue(c, "c as quad-double", 20, 64);
		}
	}

	// arithmetic behavior
	std::cout << "+---------    Default qd has subnormals, but no supernormals\n";
	{
		qd a(2.0), b(4.0), c{};
		// these are integers, so we don't need much precision
		int precision = 2;
		c = a + b;
		ReportQuadDoubleOperation(a, "+", b, c, precision);
		c = a - b;
		ReportQuadDoubleOperation(a, "-", b, c, precision);
		c = a * b;
		ReportQuadDoubleOperation(a, "*", b, c, precision);
		c = a / b;
		ReportQuadDoubleOperation(a, "/", b, c, precision);

		// increment
		a = 0.0;
		ReportValue(a, "          0.0");
		++a;
		ReportValue(a, "nextafter 0.0");
		a = 1.0;
		ReportValue(a, "          1.0");
		++a;
		ReportValue(a, "nextafter 1.0", 20, 32);

		// decrement
		a = 0.0;
		ReportValue(a, "          0.0");
		--a;
		ReportValue(a, "nextbelow 0.0");
		a = 1.0;
		ReportValue(a, "          1.0");
		--a;
		ReportValue(a, "nextbelow 1.0", 20, 32);

		{
			// iszero() and isdenorm() are defined in the sw::universal namespace
			// In clang there is an ambiguity in math.h
			// and for some reason isdenorm is not in std namespace
			// so make the call explicit for double
			double d(0.0);
			if (sw::universal::iszero(d)) std::cout << d << " is zero\n";
			d = std::nextafter(d, +INFINITY);
			if (sw::universal::isdenorm(d)) std::cout << d << " is a subnormal number\n";
		}
		{
			qd d(0.0);
			if (iszero(d)) std::cout << d << " is zero\n";
			++d;
			if (isdenorm(d)) std::cout << d << " is a subnormal number\n";
		}
	}

	std::cout << "+---------    fraction bit progressions \n";
	{
		float fulp = ulp(1.0f);
		Progression(1.0f + fulp);
		Progression(1.0 + ulp(2.0));
		double v = ulp(1.0);
		Progression( 1.0 - v/2.0 );
		std::cout << to_quad(qd(1.0 - v / 2.0)) << '\n';
	}

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
			x0 = 1.0 + (std::pow(2.0, -double(i)));
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

	// report on the dynamic range of some standard configurations
	std::cout << "+---------    Dynamic range quad-double configurations   --------+\n";
	{
		qd a; // uninitialized

		a.maxpos();
		std::cout << "maxpos  quad-double :\n" << to_binary(a, true) << " : " << a << " : " << scale(a) << '\n';
		a.minpos();
		std::cout << "minpos  quad-double :\n" << to_binary(a, true) << " : " << a << " : " << scale(a) << '\n';
		a = std::numeric_limits<qd>::denorm_min();
		std::cout << "smallest quad-double:\n" << to_binary(a, true) << " : " << a << " : " << scale(a) << '\n';
		a.zero();
		std::cout << "zero                :\n" << to_binary(a, true) << " : " << a << " : " << scale(a) << '\n';
		a.minneg();
		std::cout << "minneg  quad-double :\n" << to_binary(a, true) << " : " << a << " : " << scale(a) << '\n';
		a.maxneg();
		std::cout << "maxneg  quad-double :\n" << to_binary(a, true) << " : " << a << " : " << scale(a) << '\n';

		std::cout << "---\n";
	}

	return 0;
	// constexpr and specific values
	std::cout << "+---------    constexpr and specific values   --------+\n";
	{
		using Real = qd;

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
		using Real = qd;

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
		std::string qdstr;
		qd v;

		v = parse("0.0");
		qdstr = v.to_string(25, 25, true, false, false, false, true, false, ' ');
		std::cout << qdstr << '\n';

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
		std::cout << std::setprecision(defaultPrecision);
	}

	std::cout << "+---------    set specific values of interest   --------+\n";
	{
		qd a{ 0 }; // initialized
		std::cout << "maxpos : " << a.maxpos() << " : " << scale(a) << '\n';
		std::cout << "minpos : " << a.minpos() << " : " << scale(a) << '\n';
		std::cout << "zero   : " << a.zero()   << " : " << scale(a) << '\n';
		std::cout << "minneg : " << a.minneg() << " : " << scale(a) << '\n';
		std::cout << "maxneg : " << a.maxneg() << " : " << scale(a) << '\n';
		std::cout << dynamic_range<qd>() << std::endl;
	}

	std::cout << "+---------    quad-double subnormal behavior   --------+\n";
	{
		constexpr double minpos = std::numeric_limits<double>::min();
		std::cout << to_binary(minpos) << " : " << minpos << '\n';
		double subnormal = minpos / 2.0;
		std::cout << to_binary(subnormal) << " : " << subnormal << '\n';
		qd a(minpos);
		for (int i = 0; i < 10/*106*/; ++i) {
			std::string str = a.to_string(30, 40, false, true, false, false, false, false, ' ');
			std::cout << to_binary(a) << " : " << a << " : " << str << '\n';
			a /= 2.0;
		}
	}

	std::cout << "+---------    special value properties quad-double vs IEEE-754   --------+\n";
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

		qd a(fa);
		if ((a < 0.0f && a > 0.0f && a != 0.0f)) {
			std::cout << "quad-double (qd) is incorrectly implemented\n";
			++nrOfFailedTestCases;
		}
		else {
			std::cout << "qd NAN has no sign\n";
		}
	}

	std::cout << "+---------    numeric_limits of quad-double vs IEEE-754   --------+\n";
	{
		std::cout << "qd(INFINITY): " << qd(INFINITY) << "\n";
		std::cout << "qd(-INFINITY): " << qd(-INFINITY) << "\n";

		std::cout << "qd(std::numeric_limits<float>::infinity())  : " << qd(std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "qd(-std::numeric_limits<float>::infinity()) : " << qd(-std::numeric_limits<float>::infinity()) << "\n";

		std::cout << " 2 * std::numeric_limits<float>::infinity()  : " << 2 * std::numeric_limits<float>::infinity() << "\n";
		std::cout << " 2 * std::numeric_limits<double>::infinity() : " << 2 * std::numeric_limits<double>::infinity() << "\n";
		std::cout << "-2 * std::numeric_limits<qd>::infinity()     : " << -2 * std::numeric_limits<qd>::infinity() << "\n";

//		std::cout << "sw::universal::nextafter(qd(0), std::numeric_limits<qd>::infinity())  : " << sw::universal::nextafter(qd(-0), std::numeric_limits<qd>::infinity()) << "\n";
		std::cout << "std::nextafter(float(0), std::numeric_limits<float>::infinity())              : " << std::nextafter(float(-0), std::numeric_limits<float>::infinity()) << "\n";
//		std::cout << "sw::universal::nextafter(qd(0), -std::numeric_limits<qd>::infinity()) : " << sw::universal::nextafter(qd(0), -std::numeric_limits<qd>::infinity()) << "\n";
		std::cout << "std::nextafter(float(0), -std::numeric_limits<float>::infinity())             : " << std::nextafter(float(0), -std::numeric_limits<float>::infinity()) << "\n";

		std::cout << "qd(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_QUIET)      : " << qd(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_QUIET) << "\n";
		std::cout << "qd(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) : " << qd(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) << "\n";
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
