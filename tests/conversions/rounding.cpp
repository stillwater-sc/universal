//  rounding.cpp : rounding and assignment test suite for abitrary precision integers to real number types
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
// configure the posit arithmetic
#include <universal/posit/posit>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/integer/integer.hpp>
#include <universal/integer/numeric_limits.hpp>
// is representable
#include <universal/functions/isrepresentable.hpp>
// test helpers
#include "../test_helpers.hpp"

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

namespace sw {
namespace unum {


}
}

// generate a posit conversion test case
// process to convert an integer to a posit is to
// transform the integer into a 1.####eExp format
// find msb -> scale is msb
// shift all the msb-1 bits into a fraction, making the msb the hidden bit
// round the bits we have with respect to the scale of the number
template<size_t nbits, size_t es, size_t ibits>
void GeneratePositConversionTestCase(sw::unum::posit<nbits, es>& p, const sw::unum::integer<ibits>& w) {
	using namespace std;
	using namespace sw::unum;

	value<ibits> v;

	bool sign = w < 0;
	bool isZero = w == 0;
	bool isInf = false;
	bool isNan = false;
	long _scale = scale(w);
	int msb = findMsb(w);
	bitblock<ibits> fraction_without_hidden_bit;
	int fbit = ibits - 1;
	for (int i = msb - 1; i >= 0; --i) {
		fraction_without_hidden_bit.set(fbit, w.at(i));
		--fbit;
	}
	v.set(sign, _scale, fraction_without_hidden_bit, isZero, isInf, isNan);
	cout << "integer is " << w << endl;
	cout << "value is   " << v << endl;
	p = v;
	cout << "posit is   " << color_print(p) << " " << p << " " << hex_format(p) << endl;
}


////////////////// free form integer rounding operation /////////////////////////

/*
Rounding rules:
  ULP = Unit in the Last Place
  G   = guard bit
  R   = round bit
  S   = sticky bit
 ...ULP|GRS...
  GRS | Action
  0xx | round-down
  100 | tie: round-up to even when ULP = 1, else round down
  101 | round-up
  110 | round-up
  111 | round-up

  sticky = OR(remaining bits)
 */

 // rounding to even
template<size_t src_bits, size_t tgt_bits>
void round(const sw::unum::integer<src_bits>& src, sw::unum::integer<tgt_bits>& tgt) {
	tgt.bitcopy(src);
	// NOTE:  use integer operators that work for unsigned values
	if (1) {
	}
	else {
	}
}

template<size_t nbits>
void VerifyScale() {
	assert(nbits > 1); // we are representing numbers not booleans
	int cntr = 0;
	sw::unum::integer<nbits> i = 1;
	while (cntr < nbits) {
		std::cout << std::setw(20) << sw::unum::to_binary(i) << std::setw(20) << i << " scale is " << sw::unum::scale(i) << std::endl;
		i *= 2;
		++cntr;
	}
	// enumerate the negative integers
	// i is zero at this point
	i.set(nbits - 1, true); i >>= 1; i.set(nbits-1, true);
	cntr = 1;
	while (cntr < nbits) {
		std::cout << std::setw(20) << sw::unum::to_binary(i) << std::setw(20) << i << " scale is " << sw::unum::scale(i) << std::endl;
		i >>= 1; i.set(nbits-1, true);
		++cntr;
	}
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0


int main()
try {
	using namespace std;
	using namespace sw::unum;

	std::string tag = "Integer Rounding tests failed";

#if MANUAL_TESTING

	using int10_t = integer<10>;
	using int11_t = integer<11>;
	using int12_t = integer<12>;
	using int13_t = integer<13>;
	using int14_t = integer<14>;
	using int15_t = integer<15>;
	using int16_t = integer<16>;
	using int17_t = integer<17>;
	using int18_t = integer<18>;

	int14_t i14 = 0x1fff;
	int15_t i15 = 0x3fff;
	int16_t i16 = 0x7fff;

	cout << to_binary(i14) << " " << i14 << endl;
	cout << to_binary(i15) << " " << i15 << endl;
	cout << to_binary(i16) << " " << i16 << endl;

	using posit8_t = posit<8, 0>;
	posit8_t p8;
	GeneratePositConversionTestCase(p8, i14);
	GeneratePositConversionTestCase(p8, i15);
	GeneratePositConversionTestCase(p8, i16);

	using posit16_t = posit<16, 1>;
	posit16_t p16;
	GeneratePositConversionTestCase(p16, i14);
	GeneratePositConversionTestCase(p16, i15);
	GeneratePositConversionTestCase(p16, i16);

	using int32_t = integer<32>;
	// create the 5 rounding configurations for a 14bit integer
	int32_t tie = 0x00001fff;

	// if we take the posit around 1.0 then we know exactly that the scale is 0
	// and the rounding-down and rounding-up cases are then easily constructed.
	// say we have a posit<16,1>, it has 1 sign bit, 2 regime bits, 1 exponent
	// bit, and 12 mantissa bits

	// using posit32_t = posit<32, 2>;

	//VerifyScale<16>();
	//VerifyScale<24>();
	//VerifyScale<32>();

	//cout << "minimum for integer<16> " << min_int<16>() << endl;
	//cout << "maximum for integer<16> " << max_int<16>() << endl;

	cout << "done" << endl;

	return EXIT_SUCCESS;
#else
	std::cout << "Integer Rounding verfication" << std::endl;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

#if STRESS_TESTING

#endif // STRESS_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
