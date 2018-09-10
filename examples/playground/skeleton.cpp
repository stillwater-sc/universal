// skeleton.cpp example showing the basic program structure to use custom posit configurations
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "common.hpp"
// when you define POSIT_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_CONVERSION
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <posit>

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

template<size_t nbits, size_t es>
void ColorPrintTable()
{
	constexpr size_t NR_POSITS = (size_t(1) << nbits);
	sw::unum::posit<nbits, es> p;
	for (size_t i = 0; i < NR_POSITS; ++i) {
		p.set_raw_bits(i);
		std::cout << type_tag(p) << ": " << color_print(p) << " " << pretty_print(p) << std::endl;
	}
}

void HexVsDefaultFloatPrinting()
{
	using namespace std;

	cout << hexfloat;
	cout << "hexfloat: " << 0.12345678 << endl;
	cout << defaultfloat;
	cout << "default : " << 0.12345678 << endl;
}
void LongDoubleExample()
{
	using namespace std;
	using namespace sw::unum;

	double d = (double)0.79432823472428150206586100479;
	posit<32, 2> E_pos(d);
	cout << setprecision(30) << fixed << d << setprecision(6) << endl;
	cout << pretty_print(E_pos) << endl;

	long double ld = (long double)0.79432823472428150206586100479;
	E_pos = ld;
	cout << setprecision(30) << fixed << ld << setprecision(6) << endl;
	cout << pretty_print(E_pos) << endl;


	int _exp;
	union {
		long double fr;
		unsigned char bytes[16];
	} u;
	u.fr = frexpl(ld, &_exp);
	cout << "bytes of fraction: " << hex;
	for (int i = 15; i >= 0; i--) {
		cout << setw(2) << unsigned(u.bytes[i]) << " ";
	}
	cout << dec << endl;
}

template<size_t nbits, size_t es>
void PiExamples()
{
	using namespace std;
	using namespace sw::unum;

	posit<nbits, es> p;

	p = m_pi;
	cout << type_tag(p) << " value of PI    = " << p << " " << color_print(p) << " " << pretty_print(p) << endl;

	// convert posit back to float
	float f = float(p);
	cout << "float value               = " << f << endl;

	// calculate PI/2
	p = p / 2.0;  // implicit conversions of literals
	cout << type_tag(p) << " value of PI/2  = " << p << " " << color_print(p) << " " << pretty_print(p) << endl;
}

void DynamicRangeTable()
{
	using namespace std;
	using namespace sw::unum;

	constexpr size_t nbits = 8;
	{
		posit<nbits, 0> p(1.0); --p;
		cout << dynamic_range(p) << endl;
	}
	{
		posit<nbits, 1> p(1.0); --p;
		cout << dynamic_range(p) << endl;
	}
	{
		posit<nbits, 2> p(1.0); --p;
		cout << dynamic_range(p) << endl;
	}
	{
		posit<nbits, 3> p(1.0); --p;
		cout << dynamic_range(p) << endl;
	}
	{
		posit<nbits, 4> p(1.0); --p;
		cout << dynamic_range(p) << endl;
	}
	{
		posit<nbits, 5> p(1.0); --p;
		cout << dynamic_range(p) << endl;
	}
	{
		posit<nbits, 6> p(1.0); --p;
		cout << dynamic_range(p) << endl;
	}
	cout << endl;
}

void OnePlusEps()
{
	using namespace std;
	using namespace sw::unum;

	constexpr size_t nbits = 8;
	{
		posit<nbits, 0> p(1.0); --p;
		cout << type_tag(p) << ": " << color_print(p) << " " << pretty_print(p) << endl;
	}
	{
		posit<nbits, 1> p(1.0); --p;
		cout << type_tag(p) << ": " << color_print(p) << " " << pretty_print(p) << endl;
	}
	{
		posit<nbits, 2> p(1.0); --p;
		cout << type_tag(p) << ": " << color_print(p) << " " << pretty_print(p) << endl;
	}
	{
		posit<nbits, 3> p(1.0); --p;
		cout << type_tag(p) << ": " << color_print(p) << " " << pretty_print(p) << endl;
	}
	{
		posit<nbits, 4> p(1.0); --p;
		cout << type_tag(p) << ": " << color_print(p) << " " << pretty_print(p) << endl;
	}
	{
		posit<nbits, 5> p(1.0); --p;
		cout << type_tag(p) << ": " << color_print(p) << " " << pretty_print(p) << endl;
	}
	{
		posit<nbits, 6> p(1.0); --p;
		cout << type_tag(p) << ": " << color_print(p) << " " << pretty_print(p) << endl;
	}
	cout << endl;
}

void OneMinusEps()
{
	using namespace std;
	using namespace sw::unum;

	constexpr size_t nbits = 8;
	{
		posit<nbits, 0> p(1.0); ++p;
		cout << type_tag(p) << ": " << color_print(p) << " " << pretty_print(p) << endl;
	}
	{
		posit<nbits, 1> p(1.0); ++p;
		cout << type_tag(p) << ": " << color_print(p) << " " << pretty_print(p) << endl;
	}
	{
		posit<nbits, 2> p(1.0); ++p;
		cout << type_tag(p) << ": " << color_print(p) << " " << pretty_print(p) << endl;
	}
	{
		posit<nbits, 3> p(1.0); ++p;
		cout << type_tag(p) << ": " << color_print(p) << " " << pretty_print(p) << endl;
	}
	{
		posit<nbits, 4> p(1.0); ++p;
		cout << type_tag(p) << ": " << color_print(p) << " " << pretty_print(p) << endl;
	}
	{
		posit<nbits, 5> p(1.0); ++p;
		cout << type_tag(p) << ": " << color_print(p) << " " << pretty_print(p) << endl;
	}
	{
		posit<nbits, 6> p(1.0); ++p;
		cout << type_tag(p) << ": " << color_print(p) << " " << pretty_print(p) << endl;
	}
	cout << endl;
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bSuccess = true;

	HexVsDefaultFloatPrinting(); cout << endl;
	LongDoubleExample(); cout << endl;
	PiExamples<8,1>();	cout << endl;

	DynamicRangeTable();

	OnePlusEps();
	OneMinusEps();

	ColorPrintTable<8, 3>();

	return (bSuccess ? EXIT_SUCCESS : EXIT_FAILURE);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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
