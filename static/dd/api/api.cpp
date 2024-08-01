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

void Progression(double v) {
	using namespace sw::universal;

	float f{ float(v) };
	std::cout << to_binary(f, true) << " : " << f << '\n';

	double d{ v };
	std::cout << to_binary(d, true) << " : " << d << '\n';

	dd a{ v };
	std::cout << to_binary(a, true) << " : " << a << '\n';
}

#if defined(DOUBLEDOUBLE_CONSTANTS)
dd _zero(0.0);
dd _one(1.0);
dd _ten(10.0);

dd _tenth("0.1");
dd _third("0.333333333333333333333333333333333333");

dd _2pi("6.283185307179586476925286766559005768");
dd _pi("3.141592653589793238462643383279502884");
dd _pi2("1.570796326794896619231321691639751442");
dd _pi4("0.785398163397448309615660845819875721");
dd _3pi4 = _pi2 + _pi4;

dd _e("2.718281828459045235360287471352662498");

dd _ln2("0.693147180559945309417232121458176568");
dd _ln10("2.302585092994045684017991454684364208");

dd _lge("1.442695040888963407359924681001892137");
dd _lg10("3.321928094887362347870319429489390176");

dd _log2("0.301029995663981195213738894724493027");
dd _loge("0.434294481903251827651128918916605082");

dd _sqrt2("1.414213562373095048801688724209698079");

dd _inv_pi("0.318309886183790671537767526745028724");
dd _inv_pi2("0.636619772367581343075535053490057448");
dd _inv_e("0.367879441171442321595523770161460867");
dd _inv_sqrt2("0.707106781186547524400844362104849039");
#endif

namespace sw {
	namespace universal {

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

		void dd_binary(dd const& v) {
			std::cout << to_binary(v.high()) << " : " << v.high() << '\n';
			std::cout << to_binary(v.low()) << " : " << v.low() << '\n';
		}

		void adjust(dd const& a) {
			dd r = abs(a);
			dd ten(10.0);
			int e{ 0 };
			dd_binary(r);
			frexp(r, &e);
			std::cout << "exponent : " << e << '\n';

			if (e < 0) {
				if (e > 300) {
					r = ldexp(r, 53);		dd_binary(r);
					r *= pown(ten, -e);	dd_binary(r);
					r = ldexp(r, -53);	dd_binary(r);
				}
				else {
					r *= pown(ten, -e);	dd_binary(r);
				}
			}
			else {
				if (e > 0) {
					if (e > 300) {
						r = ldexp(r, -53);	dd_binary(r);
						r /= pown(ten, e);		dd_binary(r);
						r = ldexp(r, 53);		dd_binary(r);
					}
					else {
						r /= pown(ten, -e);	dd_binary(r);
					}
				}
			}
		}
	}
}




int main()
try {
	using namespace sw::universal;

	std::string test_suite = "doubledouble (dd) API tests";
	int nrOfFailedTestCases = 0;

	auto oldPrec = std::cout.precision();

	{
		constexpr double minpos = std::numeric_limits<double>::min();
		dd a(minpos);
		for (int i = 0; i < 10; ++i) {

		}

	}
	{
		dd a{ 1.0e-1 }, ten(10.0);
		int e{ 0 };
		for (int i = 0; i < 5; ++i) {
			adjust(a);
			a /= 2.0;
		}
		a = 2.0;
		for (int i = 0; i < 5; ++i) {
			adjust(a);
			a *= 2.0;
		}

	}
	return 0;

	{
		constexpr double minpos = std::numeric_limits<double>::min();
		std::cout << to_binary(minpos) << " : " << minpos << '\n';
		double subnormal = minpos / 32.0;
		std::cout << to_binary(subnormal) << " : " << subnormal << '\n';
		dd a(subnormal);
		std::string str = a.to_string(30, 40, std::cout.flags(), false, false, ' ');
		std::cout << to_binary(a) << " : " << a << " : " << str << '\n';
	}

	return 0;

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

	// important behavioral traits
	{
		using TestType = dd;
		ReportTrivialityOfType<TestType>();
	}

	std::cout << "+---------    fraction bit progressions \n";
	{
//		Progression(1.0 + ulp(1.0));
//		Progression(1.0 + ulp(2.0));
//		Progression(1ull << 53);
	}

	// default behavior
	std::cout << "+---------    Default dd has subnormals, but no supernormals\n";
	{
		using Real = dd;

		Real a(1ull << 53), b(1.0), c{};
		c = a + b;
		ReportBinaryOperation(a, "+", b, c);

	}

	// default behavior
	std::cout << "+---------    Default dd has subnormals, but no supernormals\n";
	{
		using Real = dd;

		Real a(1ull << 53), b(1.0);
		ArithmeticOperators(a, b);
	}

	// report on the dynamic range of some standard configurations
	std::cout << "+---------    Dynamic ranges of standard bfloat16 configurations   --------+\n";
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
		a.setbits(0x8080);  // negative min normal
		std::cout << "minneg  doubledouble      : " << to_binary(a) << " : " << a << '\n';
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

	/* TBD
	doubledouble is a pair of two floating-point values that need to be 'computed' to generate a value
	this implies that it isn't as easy as walking a bit pattern.
	We need a bit more experience with this data type to figure out how to progress its values in an education manner
	ETLO 7/31/2024

	std::cout << "+---------    doubledouble bit progressions   --------+\n";
	{
		constexpr unsigned nbits = 128;
		constexpr unsigned fbits = 106;
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
	*/
bla:
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

	std::cout << "+---------    special value properties doubledouble vs IEEE754   --------+\n";
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
