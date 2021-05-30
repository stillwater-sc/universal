// skeleton.cpp example showing the basic program structure to use custom posit configurations
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "common.hpp"
// when you define POSIT_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_CONVERSION
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit>
#include <universal/number/integer/integer>

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
	using namespace std;
	constexpr size_t NR_POSITS = (size_t(1) << nbits);
	sw::universal::posit<nbits, es> p;
	cout << "Color Printing a posit configuration\n";
	for (size_t i = 0; i < NR_POSITS; ++i) {
		p.setbits(i);
		cout << type_tag(p) << ": " << color_print(p) << " " << pretty_print(p) << std::endl;
	}
	cout << endl << endl;
}

void HexVsDefaultFloatPrinting()
{
	using namespace std;

	cout << hexfloat;
	cout << "hexfloat: " << 0.12345678 << endl;
	cout << defaultfloat;
	cout << "default : " << 0.12345678 << endl;
	cout << endl;
}

void LongDoubleExample()
{
	using namespace std;
	using namespace sw::universal;

	cout << "LongDouble values\n";
	double d = (double)0.79432823472428150206586100479;
	posit<32, 2> E_pos(d);
	auto old_precision = cout.precision();
	cout << setprecision(30) << fixed << d << setprecision(old_precision) << scientific << endl;
	cout << pretty_print(E_pos) << endl;

	long double ld = (long double)0.79432823472428150206586100479;
	E_pos = ld;
	cout << setprecision(30) << fixed << ld << setprecision(old_precision) << scientific << endl;
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
	using namespace sw::universal;

	cout << "Value of PI as a function of the posit configuration\n";
	posit<nbits, es> p;

	p = m_pi;
	cout << type_tag(p) << " value of PI    = " << p << " " << color_print(p) << " " << pretty_print(p) << endl;

	// convert posit back to float
	float f = float(p);
	cout << "float value               = " << f << endl;

	// calculate PI/2
	p = p / 2.0;  // implicit conversions of literals
	cout << type_tag(p) << " value of PI/2  = " << p << " " << color_print(p) << " " << pretty_print(p) << endl;
	
	cout << endl;
}

void DynamicRangeTable()
{
	using namespace std;
	using namespace sw::universal;

	cout << "Dynamic Range table of posit with nbits = 8\n";
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

template<size_t nbits = 8>
void OneMinusEps()
{
	using namespace std;
	using namespace sw::universal;

	cout << "1.0 - epsilon\n";
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

template<size_t nbits = 8>
void OnePlusEps()
{
	using namespace std;
	using namespace sw::universal;

	cout << "1.0 + epsilon\n";
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

void Conversions() {
	using namespace std;
	using namespace sw::universal;

	posit<8, 0> p8a;
	posit<16, 1> p16;
	posit<32, 2> p32;
	cout << "Arbitrary conversions\n";
	for (uint16_t i = 0; i < 256; ++i) {
		p8a.setbits(i);
		p16 = posit<16, 1>(p8a);
		p32 = posit<32, 2>(p8a);
		cout << "p8 " << setw(10) << p8a << " : " << color_print(p8a) << " p16 " << setw(10) << p16 << " : " << color_print(p16) << " p32 " << setw(10) << p32 << " : " << color_print(p32) << endl;
	}
}

void NumberTraits() {
	using namespace std;
	using namespace sw::universal;

	cout << "epsilon for floats       : " << number_traits<float>::epsilon() << endl;
	cout << "epsilon for doubles      : " << number_traits<double>::epsilon() << endl;
	cout << "epsilon for posit<8,0>   : " << number_traits<posit<8, 0> >::epsilon() << endl;
	cout << "epsilon for posit<16,1>  : " << number_traits<posit<16, 1> >::epsilon() << endl;
	cout << "epsilon for posit<32,2>  : " << number_traits<posit<32, 2> >::epsilon() << endl;
	cout << "epsilon for posit<64,3>  : " << number_traits<posit<64, 3> >::epsilon() << endl;
	cout << "epsilon for posit<128,4> : " << number_traits<posit<128, 4> >::epsilon() << endl;  // TODO
	cout << "epsilon for posit<256,5> : " << number_traits<posit<256, 5> >::epsilon() << endl;  // TODO

	/* call the raw implementation
	cout << "digit10 for floats       : " << sw::internal::default_digits10_impl<float, false, false>().run() << endl;
	cout << "digit10 for doubles      : " << sw::internal::default_digits10_impl<double, false, false>().run() << endl;
	cout << "digit10 for posit<8,0>   : " << sw::internal::default_digits10_impl<posit<8, 0>, false, false>().run() << endl;
	cout << "digit10 for posit<16,1>  : " << sw::internal::default_digits10_impl<posit<16, 1>, false, false>().run() << endl;
	cout << "digit10 for posit<32,2>  : " << sw::internal::default_digits10_impl<posit<32, 2>, false, false>().run() << endl;
	cout << "digit10 for posit<64,3>  : " << sw::internal::default_digits10_impl<posit<64, 3>, false, false>().run() << endl;
	cout << "digit10 for posit<128,4> : " << sw::internal::default_digits10_impl<posit<128, 4>, false, false>().run() << endl;
	cout << "digit10 for posit<256,5> : " << sw::internal::default_digits10_impl<posit<256, 5>, false, false>().run() << endl;
	*/

	cout << "digit10 for floats       : " << number_traits<float>::digits10() << endl;
	cout << "digit10 for doubles      : " << number_traits<double>::digits10() << endl;
	cout << "digit10 for posit<8,0>   : " << number_traits<posit<8, 0> >::digits10() << endl;
	cout << "digit10 for posit<16,1>  : " << number_traits<posit<16, 1> >::digits10() << endl;
	cout << "digit10 for posit<32,2>  : " << number_traits<posit<32, 2> >::digits10() << endl;
	cout << "digit10 for posit<64,3>  : " << number_traits<posit<64, 3> >::digits10() << endl;
	cout << "digit10 for posit<128,4> : " << number_traits<posit<128, 4> >::digits10() << endl;
	cout << "digit10 for posit<256,5> : " << number_traits<posit<256, 5> >::digits10() << endl;

	cout << "min pos for floats       : " << number_traits<float>::min() << endl;
	cout << "min pos for doubles      : " << number_traits<double>::min() << endl;
	cout << "min pos for posit<8,0>   : " << number_traits<posit<8, 0> >::min() << endl;
	cout << "min pos for posit<16,1>  : " << number_traits<posit<16, 1> >::min() << endl;
	cout << "min pos for posit<32,2>  : " << number_traits<posit<32, 2> >::min() << endl;
	cout << "min pos for posit<64,3>  : " << number_traits<posit<64, 3> >::min() << endl;  // TODO
	cout << "min pos for posit<128,4> : " << number_traits<posit<128, 4> >::min() << endl; // TODO
	cout << "min pos for posit<256,5> : " << number_traits<posit<256, 5> >::min() << endl; // TODO

	cout << "max pos for floats       : " << number_traits<float>::max() << endl;
	cout << "max pos for doubles      : " << number_traits<double>::max() << endl;
	cout << "max pos for posit<8,0>   : " << number_traits<posit<8, 0> >::max() << endl;
	cout << "max pos for posit<16,1>  : " << number_traits<posit<16, 1> >::max() << endl;
	cout << "max pos for posit<32,2>  : " << number_traits<posit<32, 2> >::max() << endl;
	cout << "max pos for posit<64,3>  : " << number_traits<posit<64, 3> >::max() << endl;  // TODO
	cout << "max pos for posit<128,4> : " << number_traits<posit<128, 4> >::max() << endl; // TODO
	cout << "max pos for posit<256,5> : " << number_traits<posit<256, 5> >::max() << endl; // TODO
}


int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	bool bSuccess = true;
	auto old_precision = cout.precision();

	HexVsDefaultFloatPrinting();
	LongDoubleExample();
	PiExamples<8, 0>();
	PiExamples<16, 1>();
	PiExamples<32, 2>();
	PiExamples<64, 3>();

	DynamicRangeTable();

	NumberTraits();
	OnePlusEps<8>();
	OneMinusEps<8>();
	OnePlusEps<16>();
	OneMinusEps<16>();
	OnePlusEps<32>();
	OneMinusEps<32>();
	OnePlusEps<64>();
	OneMinusEps<64>();
	OnePlusEps<128>();
	OneMinusEps<128>();
	OnePlusEps<256>();
	OneMinusEps<256>();

	ColorPrintTable<8, 3>(); 

	Conversions();

	posit<32, 2> p;
	p.setbits(0xb0bfe591u);
	cout << color_print(p) << " " << setprecision(30) << p << setprecision(old_precision) << endl;

	return (bSuccess ? EXIT_SUCCESS : EXIT_FAILURE);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
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
