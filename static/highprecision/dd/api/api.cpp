// api.cpp: application programming interface tests for double-double (dd) number system
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
//#include <universal/native/error_free_ops.hpp>  // integral part of double-double and quad-double but can be used standalone
#include <universal/common/string_utils.hpp>

namespace sw {
	namespace universal {

		void ReportDoubleDoubleOperation(const dd& a, const std::string& op, const dd& b, const dd& c, int precision = 32) {
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

			dd a{ v };
			std::cout << std::setprecision(35);
			std::cout << to_binary(a, true) << " : " << a << '\n';
			std::cout << std::setprecision(defaultPrecision);
		}

		dd parse(const std::string& str) {
			using namespace sw::universal;

			dd v(str);
			auto defaultPrecision = std::cout.precision();
			std::cout << std::setprecision(std::numeric_limits<double>::digits10);
			std::cout << "string: " << str << " = ( " << v.high() << ", " << v.low() << ") ";
			std::cout << std::setprecision(defaultPrecision);
			return v;
		}

		void print(std::ostream& ostr, dd const& v) {
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

		void construct_largest_double_double() {
			using Scalar = dd;

			double firstLimb = std::numeric_limits<double>::max();
			dd a = std::numeric_limits<Scalar>::max();
			std::cout << std::setprecision(32) << a << '\n';
			int expOfFirstLimb = scale(a);
			std::cout << to_binary(expOfFirstLimb) << " : " << expOfFirstLimb << '\n';
			// second limb exponent
			int expOfSecondLimb = expOfFirstLimb - 15;   // floor(log10(2^53)) = floor(15.9245) = 15
			std::cout << "exponent of the first  limb : " << expOfFirstLimb << '\n';
			std::cout << "exponent of the second limb : " << expOfSecondLimb << '\n';
			// construct the second limb
			double secondLimb = std::ldexp(1.0, expOfSecondLimb);
			std::cout << "1.0         " << to_binary(1.0) << '\n';
			std::cout << "first  limb " << to_binary(firstLimb) << '\n';
			std::cout << "second limb " << to_binary(secondLimb) << '\n';

			dd aa(firstLimb, secondLimb);
			std::cout << std::setprecision(16) << firstLimb << '\n';
			std::cout << std::setprecision(16) << aa << '\n';
			std::cout << std::setprecision(32) << aa << '\n';

			dd b = ulp(std::numeric_limits<double>::max());
			dd c = a + b;
			std::cout << c << '\n';
		}

		template<typename Real,
			typename = typename ::std::enable_if< ::std::is_floating_point<Real>::value, Real >::type
		>
		Real emulateNextAfter(Real x, Real y) {
			if (x == y) return y;
			int direction = (x < y) ? 1 : -1;
			Real eps = std::numeric_limits<Real>::epsilon();
			return x + direction * eps;
		}

		void ulp_progression(const std::string& tag, const dd& start) {
			std::cout << tag;
			for (dd from = start, to, delta;
				(delta = (to = nextafter(from, +INFINITY)) - from) < 10.0;
				from *= 10.0) {
				dd u = ulp(from);
				std::cout << "ulp(" << std::scientific << std::setprecision(0) << from
					<< ") gives "
//					<< std::fixed << std::setprecision(6) << u
					<< " : " << std::setprecision(6) << std::defaultfloat << u
					<< '\n';
			}
		}
	}
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "double-double (dd) API tests";
	int nrOfFailedTestCases = 0;

	auto defaultPrecision = std::cout.precision();

	// important behavioral traits
	{
		using TestType = dd;
		ReportTrivialityOfType<TestType>();
	}

	// default behavior
	std::cout << "+---------    Default double-double behavior     ---------+\n";
	{
		uint64_t big = (1ull << 53);
		ReportValue(big, "2^53", 20);
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
			dd a(big), b(1.0), c{};
			c = a + b;
			ReportValue(a, "a as double-double", 20, 16);
			ReportValue(b, "b as double-double", 20, 16);
			ReportValue(c, "c as double-double", 20, 16);
		}
	}

	// arithmetic behavior
	std::cout << "+---------    Default dd has subnormals, but no max-exponent values     ---------+\n";
	{
		dd a(2.0), b(4.0), c{};
		// these are integers, so we don't need much precision
		int precision = 2;
		c = a + b;
		ReportDoubleDoubleOperation(a, "+", b, c, precision);
		c = a - b;
		ReportDoubleDoubleOperation(a, "-", b, c, precision);
		c = a * b;
		ReportDoubleDoubleOperation(a, "*", b, c, precision);
		c = a / b;
		ReportDoubleDoubleOperation(a, "/", b, c, precision);

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
			dd d(0.0);
			if (iszero(d)) std::cout << d << " is zero\n";
			++d;
			if (isdenorm(d)) std::cout << d << " is a subnormal number\n";
		}
	}

	// helper api
	std::cout << "+---------    helpers to go from double to double-double     ---------+\n";
	{
		double a, b, c;
		a = 1.0;
		b = ulp(1.0) / 2.0;
		c = a + b;
		dd dd_c = add(a, b);
		std::cout << "demonstrating cancellation of information when adding\n";
		ReportValue(a, "a = 1.0");
		ReportValue(c, "c = a + ulp(1.0)/2");
		std::cout << "double c = " << std::setprecision(16) << c << std::setprecision(defaultPrecision) << '\n';
		std::cout << "dd     c = " << std::setprecision(32) << dd_c << std::setprecision(defaultPrecision) << '\n';

		std::cout << "demonstrating cancellation of information when subtracting\n";
		c = a - b;
		dd_c = sub(a, b);
		ReportValue(a, "a = 1.0");
		ReportValue(c, "c = a - ulp(1.0)/2");
		std::cout << "double c = " << std::setprecision(16) << c << std::setprecision(defaultPrecision) << '\n';
		std::cout << "dd     c = " << std::setprecision(32) << dd_c << std::setprecision(defaultPrecision) << '\n';

		std::cout << "demonstrating cancellation of information when multiplying\n";
		double x = ulp(1.0);
		double y = 1.5 + x;
		double z = x * y;
		dd dd_z = mul(x, y);
		ReportValue(z, "z = y * x");
		std::cout << "double z = " << std::setprecision(16) << z << std::setprecision(defaultPrecision) << '\n';
		std::cout << "dd     z = " << std::setprecision(32) << dd_z << std::setprecision(defaultPrecision) << '\n';

		std::cout << "demonstrating cancellation of information when dividing\n";
		x = ulp(1.0);
		y = 1.5 + x;
		z = y / x;
		dd_z = div(y, x);
		ReportValue(z, "z = y / x");
		std::cout << "double z = " << std::setprecision(16) << z << std::setprecision(defaultPrecision) << '\n';
		std::cout << "dd     z = " << std::setprecision(32) << dd_z << std::setprecision(defaultPrecision) << '\n';

	}

	// fraction bit behavior
	std::cout << "+---------    fraction bit progressions      ---------+\n";
	{
		// what is the value that adds a delta one below the least significant fraction bit of the high double?
		// dd = high + lo
		//    = 1*2^0 + 1*2^-53
		//    = 1.0e00 + 1.0elog10(2^-53)
		double x0{ std::pow(2.0, 0.0) };
		double x1{ std::pow(2.0, -53.0) };

		// now let's walk that bit down to the ULP
		unsigned precisionForRange = 16;
		std::cout << std::setprecision(precisionForRange);
		x0 = 1.0;
		dd a(x0, x1);
		std::cout << centered("double-double", precisionForRange + 6u) << " : ";
		std::cout << centered("binary form of x0", 68) << " : ";
		std::cout << centered("real value of x0", 15) << '\n';
		std::cout << a << " : " << to_binary(x0) << " : " << x0 << '\n';
		for (int i = 1; i < 53; ++i) {
			x0 = 1.0 + (std::pow(2.0, -double(i)));
			a.set(x0, x1);
			std::cout << a << " : " << to_binary(x0) << " : " << std::setprecision(17) << x0 << std::setprecision(precisionForRange) << '\n';
		}
		// x0 is 1.0 + eps() at this point
		std::cout << to_binary(dd(x0, x1)) << '\n';
		x0 = 1.0;
		precisionForRange = 32;
		std::cout << std::setprecision(precisionForRange);
		std::cout << centered("double-double", precisionForRange + 6u) << " : ";
		std::cout << centered("binary form of x1", 68) << " : ";
		std::cout << centered("real value of x1", 15) << '\n';
		for (int i = 0; i < 54; ++i) {
			x1 = (std::pow(2.0, -53.0 - double(i)));
			a.set(x0, x1);
			std::cout << a << " : " << to_binary(x1) << " : " << std::setprecision(17) << x1 << std::setprecision(precisionForRange) << '\n';
		}
		// print the full double-double binary pattern
		std::cout << "\nvalue and binary pattern of the double-double\n";
		precisionForRange = 32;
		std::cout << std::setprecision(precisionForRange);
		std::cout << centered("double-double", precisionForRange + 6u) << " : ";
		std::cout << centered("binary form of double-double", 110) << '\n';
		for (int i = 0; i < 54; ++i) {
			x1 = (std::pow(2.0, -53.0 - double(i)));
			a.set(x0, x1);
			std::cout << a << " : " << to_binary(a) << '\n';
		}
		// NOTE: the value of the last lower limb is half an ulp below the dd ulp at 1.0.
		// We cannot represent that bit in the binary form, but it rounds up in the decimal form as information
		// see below for the pattern
		// ....
		// 1.00000000000000000000000000000010e+00 : 0b0.01111111111.0000000000000000000000000000000000000000000000000000|00000000000000000000000000000000000000000000000000100
		// 1.00000000000000000000000000000005e+00 : 0b0.01111111111.0000000000000000000000000000000000000000000000000000|00000000000000000000000000000000000000000000000000010
		// 1.00000000000000000000000000000002e+00 : 0b0.01111111111.0000000000000000000000000000000000000000000000000000|00000000000000000000000000000000000000000000000000001
		// 1.00000000000000000000000000000001e+00 : 0b0.01111111111.0000000000000000000000000000000000000000000000000000|00000000000000000000000000000000000000000000000000000
		std::cout << std::setprecision(defaultPrecision);
	}

	std::cout << "+---------    set specific values of interest   --------+\n";
	{
		dd a; // uninitialized

		std::cout << std::setprecision(32);
		a.maxpos();
		std::cout << "maxpos  double-double : " << to_binary(a) << " : " << a << " : " << scale(a) << '\n';
		a.minpos();
		std::cout << "minpos  double-double : " << to_binary(a) << " : " << a << " : " << scale(a) << '\n';
		a = std::numeric_limits<dd>::denorm_min();
		std::cout << "smallest double-double: " << to_binary(a) << " : " << a << " : " << scale(a) << '\n';
		a.zero();
		std::cout << "zero                  : " << to_binary(a) << " : " << a << " : " << scale(a) << '\n';
		a.minneg();
		std::cout << "minneg  double-double : " << to_binary(a) << " : " << a << " : " << scale(a) << '\n';
		a.maxneg();
		std::cout << "maxneg  double-double : " << to_binary(a) << " : " << a << " : " << scale(a) << '\n';

		std::cout << "Notice that minpos is the smallest normal number, not the smallest number, which is a denorm\n";
		std::cout << std::setprecision(defaultPrecision);
		std::cout << "---\n";
	}

	std::cout << "+---------    Dynamic range double-double configuration   ---------+\n";
	{
		std::cout << dynamic_range<float>() << '\n';
		std::cout << dynamic_range<double>() << '\n';
		std::cout << dynamic_range<dd>() << '\n';

		std::cout << '\n';
		std::cout << symmetry_range<float>() << '\n';
		std::cout << symmetry_range<double>() << '\n';
		std::cout << symmetry_range<dd>() << '\n';
	}

	// constexpr and specific values
	std::cout << "+---------    constexpr and specific values   ---------+\n";
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
	std::cout << "+---------    set bit patterns API   ---------+\n";
	{
		using Real = dd;

		Real a{ 0 }; // remove warning of uninitialized local variable
		std::cout << type_tag(a) << '\n';

		// the high and low limb of the double-double have a strict exponent relationship
		// the setbit(s) API doesn't know anything about that relationship, and it is 
		// the repsonsbility of the driver to assure the relationship is adhered to
		// otherwise it is not a valid double-double
		a.setbits(0x0000);
		std::cout << to_binary(a) << " : " << a << '\n';

		// if we are going to set lower limb bits, we are creating a non-zero lower limb
		// which will need a specific relative exponent to the high limb
		// set that first
		double high = (1ull << 53);
		double low = 1.0;
		a.set(high, low);
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
	std::cout << "+---------    parse API   ---------+\n";
	{
		std::string ddstr;
		dd v;

		v = parse("0.0");
		ddstr = v.to_string(25, 25, true, false, false, false, true, false, ' ');
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
		std::cout << std::setprecision(defaultPrecision);
	}

	std::cout << "+---------    double-double subnormal behavior   --------+\n";
	{
		constexpr double minpos = std::numeric_limits<double>::min();
		std::cout << to_binary(minpos) << " : " << minpos << '\n';
		double subnormal = minpos / 2.0;
		std::cout << to_binary(subnormal) << " : " << subnormal << '\n';
		dd a(minpos);
		for (int i = 0; i < 10/*106*/; ++i) {
			std::string str = a.to_string(30, 40, false, true, false, false, false, false, ' ');
			std::cout << to_binary(a) << " : " << a << " : " << str << '\n';
			a /= 2.0;
		}
	}

	std::cout << "+---------    special value properties double-double vs IEEE-754   --------+\n";
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
			std::cout << "double-double (dd) is incorrectly implemented\n";
			++nrOfFailedTestCases;
		}
		else {
			std::cout << "double-double (dd) NAN has no sign\n";
		}
	}

	std::cout << "----------    Unit in the Last Place --------+\n";
	{
		ulp_progression("\nULP progression for dd:\n", dd(10.0e01));

		for (int i = -5; i < 6; ++i) {
			dd a(std::pow(2.0, double(i)));
			dd ulpAtI = ulp(a);
			std::string label = "ulpAt<dd>(2^" + std::to_string(i) + ")";
			ReportValue(ulpAtI, label, 20, 32);
		}
	}

	std::cout << "+---------    numeric_limits of double-double vs IEEE-754   --------+\n";
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

		std::cout << "dd(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_QUIET)      : " << dd(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_QUIET) << "\n";
		std::cout << "dd(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) : " << dd(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) << "\n";
	}

	std::cout << "+----------   numeric traits of double-double ----------+\n";
	{
		numberTraits<dd>(std::cout);
		constexpr bool hasSubnormals = true;
		using Cfloat = cfloat<1 + 11 + 105, 11, uint32_t, hasSubnormals>;
		numberTraits<Cfloat>(std::cout);
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
