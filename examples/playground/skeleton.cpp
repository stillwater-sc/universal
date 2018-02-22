// skeleton.cpp example showing the basic program structure to use custom posit configurations
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable the mathematical constants in cmath
#define _USE_MATH_DEFINES
#include "stdafx.h"

// when you define POSIT_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_CONVERSION

#include <posit>

using namespace std;
using namespace sw::unum;

/*

Mathematical 	C++ Symbol	Decimal Representation
Expression
pi				M_PI		3.14159265358979323846
pi/2			M_PI_2		1.57079632679489661923
pi/4			M_PI_4		0.785398163397448309616
1/pi			M_1_PI		0.318309886183790671538
2/pi			M_2_PI		0.636619772367581343076
2/sqrt(pi)		M_2_SQRTPI	1.12837916709551257390
sqrt(2)			M_SQRT2		1.41421356237309504880
1/sqrt(2)		M_SQRT1_2	0.707106781186547524401
e				M_E			2.71828182845904523536
log_2(e)		M_LOG2E		1.44269504088896340736
log_10(e)		M_LOG10E	0.434294481903251827651
log_e(2)		M_LN2		0.693147180559945309417
log_e(10)		M_LN10		2.30258509299404568402

*/

// best practice for C++11
constexpr double m_pi       = 3.14159265358979323846;  
constexpr double m_pi_2     = 1.57079632679489661923;
constexpr double m_pi_4     = 0.785398163397448309616;
constexpr double m_1_pi     = 0.318309886183790671538;
constexpr double m_2_pi     = 0.636619772367581343076;
constexpr double m_2_sqrtpi = 1.12837916709551257390;
constexpr double m_sqrt2    = 1.41421356237309504880;
constexpr double m_sqrt1_2  = 0.707106781186547524401;
constexpr double m_e        = 2.71828182845904523536;
constexpr double m_log2e    = 1.44269504088896340736;
constexpr double m_log10e   = 0.434294481903251827651;
constexpr double m_ln2      = 0.693147180559945309417;
constexpr double m_ln10     = 2.30258509299404568402;



namespace sw {
	namespace unum {
		enum ColorCode {
			FG_DEFAULT = 39,
			FG_BLACK = 30,
			FG_RED = 31,
			FG_GREEN = 32,
			FG_YELLOW = 33,
			FG_BLUE = 34,
			FG_MAGENTA = 35,
			FG_CYAN = 36,
			FG_LIGHT_GRAY = 37,
			FG_DARK_GRAY = 90,
			FG_LIGHT_RED = 91,
			FG_LIGHT_GREEN = 92,
			FG_LIGHT_YELLOW = 93,
			FG_LIGHT_BLUE = 94,
			FG_LIGHT_MAGENTA = 95,
			FG_LIGHT_CYAN = 96,
			FG_WHITE = 97,

			BG_DEFAULT = 49,
			BG_BLACK = 40,
			BG_RED = 41,
			BG_GREEN = 42,
			BG_YELLOW = 43,
			BG_BLUE = 44,
			BG_MAGENTA = 45,
			BG_CYAN = 46,
			BG_LIGHT_GRAY = 47,
			BG_DARK_GRAY = 100,
			BG_LIGHT_RED = 101,
			BG_LIGHT_GREEN = 102,
			BG_LIGHT_YELLOW = 103,
			BG_LIGHT_BLUE = 104,
			BG_LIGHT_MAGENTA = 105,
			BG_LIGHT_CYAN = 106,
			BG_WHITE = 107

		};
		class Color {
			ColorCode code;
		public:
			Color(ColorCode pCode) : code(pCode) {}
			friend std::ostream&
				operator<<(std::ostream& os, const Color& mod) {
				return os << "\033[" << mod.code << "m";
			}
		};

		template<size_t nbits, size_t es>
		std::string color_print(const posit<nbits, es>& p) {
			std::stringstream ss;
			Color red(ColorCode::FG_RED);
			Color yellow(ColorCode::FG_YELLOW);
			Color blue(ColorCode::FG_BLUE);
			Color magenta(ColorCode::FG_MAGENTA);
			Color cyan(ColorCode::FG_CYAN);
			Color white(ColorCode::FG_WHITE);
			Color def(ColorCode::FG_DEFAULT);
			ss << red << (p.isNegative() ? "1" : "0");

			std::bitset<nbits - 1> r = p.get_regime().get();
			int regimeBits = (int)p.get_regime().nrBits();
			int nrOfRegimeBitsProcessed = 0;
			for (int i = nbits - 2; i >= 0; --i) {
				if (regimeBits > nrOfRegimeBitsProcessed++) {
					ss << yellow << (r[i] ? "1" : "0");
				}
			}

			std::bitset<es> e = p.get_exponent().get();
			int exponentBits = (int)p.get_exponent().nrBits();
			int nrOfExponentBitsProcessed = 0;
			for (int i = int(es) - 1; i >= 0; --i) {
				if (exponentBits > nrOfExponentBitsProcessed++) {
					ss << cyan << (e[i] ? "1" : "0");
				}
			}

			std::bitset<posit<nbits,es>::fbits> f = p.get_fraction().get();
			int fractionBits = (int)p.get_fraction().nrBits();
			int nrOfFractionBitsProcessed = 0;
			for (int i = int(p.fbits) - 1; i >= 0; --i) {
				if (fractionBits > nrOfFractionBitsProcessed++) {
					ss << magenta << (f[i] ? "1" : "0");
				}
			}

			ss << def;
			return ss.str();
		}
	};
};



int main(int argc, char** argv)
try {
	bool bSuccess = true;

	{
		double d = (double)0.79432823472428150206586100479;
		posit<32, 2> E_pos(d);
		cout << setprecision(30) << fixed << d << setprecision(6) << endl;
		cout << pretty_print(E_pos) << endl;

		long double ld = (long double)0.79432823472428150206586100479;
		E_pos = ld;
		cout << setprecision(30) << fixed << ld << setprecision(6) << endl;
		cout << pretty_print(E_pos) << endl;

	}
	return 0;

	{
		constexpr size_t nbits = 8;
		constexpr size_t es = 1;

		posit<nbits, es> p;

		// assign PI to posit<8,1>
		p = m_pi;
		cout << "posit<8,1> value of PI    = " << p << " " << color_print(p) << " " << pretty_print(p) << endl;

		// convert posit back to float
		float f = float(p);
		cout << "float value               = " << f << endl;

		// calculate PI/2
		p = p / posit<nbits, es>(2.0);  // explicit conversions of literals
		cout << "posit<8,1> value of PI/2  = " << p << " " << color_print(p) << " " << pretty_print(p) << endl;
	}

	return (bSuccess ? EXIT_SUCCESS : EXIT_FAILURE);
}
catch (char const* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
catch (...) {
	cerr << "Caught unknown exception" << endl;
	return EXIT_FAILURE;
}
