// api.cpp: class interface tests for arbitrary configuration posit types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the posit template environment
// first: enable general or specialized configurations
#define POSIT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
// third: enable support for native literals in logic and arithmetic operations
#define POSIT_ENABLE_LITERALS 1
// fourth: enable/disable error-free serialization I/O
#define POSIT_ERROR_FREE_IO_FORMAT 0
// fifth: potentiall trace conversions or arithmetic
//#define ALGORITHM_VERBOSE_OUTPUT
//#define ALGORITHM_TRACE_CONVERSION
// minimum set of include files to reflect source code dependencies
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/verification/test_case.hpp>

/*
Generate Posit Lookup table for a POSIT<4,2> in TXT format
   #           Binary         Decoded       k    sign   scale          regime        exponent        fraction                         value    posit_format
   0:             0000            0000      -3       0      -8             000              --               ~                             0           4.2x0p
   1:             0001            0001      -2       0      -8             001              --               ~                    0.00390625           4.2x1p
   2:             0010            0010      -1       0      -4             01-              0-               ~                        0.0625           4.2x2p
   3:             0011            0011      -1       0      -2             01-              1-               ~                          0.25           4.2x3p
   4:             0100            0100       0       0       0             10-              0-               ~                             1           4.2x4p
   5:             0101            0101       0       0       2             10-              1-               ~                             4           4.2x5p
   6:             0110            0110       1       0       4             110              --               ~                            16           4.2x6p
   7:             0111            0111       2       0       8             111              --               ~                           256           4.2x7p
   8:             1000            1000       3       1      -8             000              --               ~                           nar           4.2x8p
   9:             1001            1111       2       1       8             111              --               ~                          -256           4.2x9p
  10:             1010            1110       1       1       4             110              --               ~                           -16           4.2xAp
  11:             1011            1101       0       1       2             10-              1-               ~                            -4           4.2xBp
  12:             1100            1100       0       1       0             10-              0-               ~                            -1           4.2xCp
  13:             1101            1011      -1       1      -2             01-              1-               ~                         -0.25           4.2xDp
  14:             1110            1010      -1       1      -4             01-              0-               ~                       -0.0625           4.2xEp
  15:             1111            1001      -2       1      -8             001              --               ~                   -0.00390625 
*/

template<unsigned nbits, unsigned es, typename bt>
void VerifyToBinary() {
	using namespace sw::universal;
	constexpr unsigned NR_VALUES = (1 << nbits);

	posit<nbits, es, bt> p;
	for (unsigned i = 0; i < NR_VALUES; ++i) {
		p.setbits(i);
		std::cout << hex_format(p) << ' ' << color_print(p) << ' ' << to_binary(p) << ' ' << p << std::endl;
	}
}

template<unsigned nbits, unsigned es, typename bt = std::uint8_t>
void Convert(float f) {
	sw::universal::posit<nbits, es, bt> a{ f };
	std::cout << a << " : " << to_binary(a) << " : " << color_print(a) << '\n';
}

template<unsigned fbits, sw::universal::BlockTripleOperator op, typename Ty>
std::string convert(Ty f) {
	using default_bt = uint8_t;
	std::stringstream s;
	sw::universal::blocktriple<fbits, op, default_bt> a(f);
	s << std::setw(40) << sw::universal::to_binary(a) << " : " << std::setw(40) << std::left << to_triple(a) << " : " << std::setw(10) << a << " : " << type_tag(a);
	return s.str();
}

template<unsigned fbits, typename bt = std::uint8_t>
void RealToBlockTriple(float f) {
	std::cout << "real -> blocktriple\n";
	sw::universal::blocktriple<fbits, sw::universal::BlockTripleOperator::REP, std::uint8_t> triple(f);
	std::cout << to_binary(triple) << " : " << to_triple(triple) << " : " << triple << '\n';
	std::cout << triple.fraction() << '\n';
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "generalized posit number system API";
	std::string test_tag    = "api";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	//// posit construction, initialization, assignment and comparisions

	{
		int start = nrOfFailedTestCases;
		// maxpos of a posit<8,0> = 64
		posit<8, 0> a(-64), b(-128), c(64), d(-64);
		// b initialized to -128 in saturating arithmetic becomes -64
		if (0 != (c + d)) ++nrOfFailedTestCases; //cout << to_binary(c + d) << endl;
		if (a != b) ++nrOfFailedTestCases;

		if (a != (d - 32)) ++nrOfFailedTestCases; // saturating to maxneg
		if (a != (d - 0.5)) ++nrOfFailedTestCases; // saturating to maxneg
		if (nrOfFailedTestCases - start > 0) {
			std::cout << to_binary(a) << ' ' << to_binary(b) << ' ' << to_binary(c) << ' ' << to_binary(d) << '\n';
			std::cout << to_binary(d - 1) << ' ' << to_binary(d - 0.5) << '\n';
		}
	}


	// type tag to identify the type without having to depend on demangle
	{
		using Posit = posit<16, 2>;
		Posit a{ 0 };
		std::cout << "type identifier : " << type_tag(a) << '\n';
		std::cout << "standard posit  : " << type_tag(posit< 8, 2>()) << '\n';
		std::cout << "standard posit  : " << type_tag(posit< 16, 2>()) << '\n';
		std::cout << "standard posit  : " << type_tag(posit< 32, 2>()) << '\n';
		std::cout << "standard posit  : " << type_tag(posit< 64, 2>()) << '\n';
		std::cout << "standard posit  : " << type_tag(posit<128, 2>()) << '\n';
		std::cout << "standard posit  : " << type_tag(posit<256, 2>()) << '\n';
	}

		std::cout << "*** special cases\n";
	{
		using BlockType = std::uint8_t;
		using Posit = posit<8, 0, BlockType>;
		Posit a;
		a.setnar();		ReportValue(a, "NaR     ");
		a.maxpos();		ReportValue(a, "maxpos  ");
		a = maxprecision_max<8, 0, BlockType>(); ReportValue(a, "maxr0   ");
		a = 1;  		ReportValue(a, "  1     ");
		a = maxprecision_min<8, 0, BlockType>(); ReportValue(a, "minr-1  ");
		a.minpos();		ReportValue(a, "minpos  ");
		a.setzero();	ReportValue(a, "zero    ");
		a.minneg();		ReportValue(a, "minneg  ");
		a = -1;			ReportValue(a, " -1     ");
		a.maxneg();		ReportValue(a, "maxneg  ");
	}

	std::cout << "*** binary, color, and value printing\n";
	{
		using Posit = posit<5, 1>;
		Posit a;
		for (unsigned i = 0; i < 32; ++i) {
			a.setbits(i);
			std::cout << to_binary(a) << " : " << color_print(a) << " : " << a << '\n';
		}
	}

	std::cout << "*** pretty and info printing\n";
	{
		using Posit = posit<5, 1>;
		Posit a;
		for (unsigned i = 0; i < 32; ++i) {
			a.setbits(i);
			std::cout << std::left << std::setw(30) << pretty_print(a) << " : " << info_print(a) << '\n';
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//// improving efficiency for posits through explicit BlockType specification

	{
		int start = nrOfFailedTestCases;

		posit<16, 2> a{ 0 }, b{ -0.984375f }, c{ 0.984375 }, d{ -0.984375 };
		if (a != (c + d)) ++nrOfFailedTestCases;
		if (a != (-b - c)) ++nrOfFailedTestCases;
		//		cout << to_binary(a, true) << ' ' << to_binary(b, true) << ' ' << to_binary(c, true) << ' ' << to_binary(d, true) << endl;
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : construction " << to_binary(a) << ' ' << to_binary(b) << ' ' << to_binary(c) << ' ' << to_binary(d) << '\n';
			std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// selectors

	{
		int start = nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : selectors\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// modifiers

	{
		int start = nrOfFailedTestCases;
		// state/bit management

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : modifiers\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	// complements
	{
		int start = nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : complements 1\n";
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// parsing of text input
	{
		// TODO: implement parse
		constexpr unsigned nbits = 128;
		constexpr unsigned rbits = 64;
		//parse<nbits, rbits> a, b, c, d;
		//a.assign("123456789.987654321");
		//parse("123456789.987654321", b);

	}

	///////////////////////////////////////////////////////////////////////////////////
	// arithmetic
	{
		int start = nrOfFailedTestCases;
		constexpr unsigned nbits = 16;
		constexpr unsigned es    =  2;
		using BlockType          = std::uint16_t;
		posit<nbits, es, BlockType> a, b, c, d;
		a.maxpos();
		b.maxneg();
		c.minpos();
		d.minneg();
		if ((c + d) != 0) ++nrOfFailedTestCases;

		if ((a + c) != a) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: min/max\n";
			std::cout << to_binary(c + d) << " vs " << to_binary(posit<nbits, es>(0)) << '\n';
			std::cout << to_binary(a + c) << " vs " << to_binary(b) << '\n';
		}
	}

	///////////////////////////////////////////////////////////////////////////////////
	// logic, in particular, all the literal constant combinations
	{
		int start = nrOfFailedTestCases;
		constexpr unsigned nbits = 8;
		constexpr unsigned es    = 2;
		posit<nbits, es> a, b, c, d;
		a = 1;
		b = 2l;
		c = 3ll;
		d = 0ull;
		// unsigned literals
		if (a != 1u) ++nrOfFailedTestCases;
		if (b != 2ul) ++nrOfFailedTestCases;
		if (c != 3ull) ++nrOfFailedTestCases;
		if (1u != a) ++nrOfFailedTestCases;
		if (2ul != b) ++nrOfFailedTestCases;
		if (3ull != c) ++nrOfFailedTestCases;
		if (d != c - b - a) ++nrOfFailedTestCases;
		// signed literals
		if (-a != -1) ++nrOfFailedTestCases;
		if (-b != -2l) ++nrOfFailedTestCases;
		if (-c != -3ll) ++nrOfFailedTestCases;
		if (-1 != -a) ++nrOfFailedTestCases;
		if (-2l != -b) ++nrOfFailedTestCases;
		if (-3ll != -c) ++nrOfFailedTestCases;

		// less than unsigned literal
		d = 4.0f;
		if (d < 1u) ++nrOfFailedTestCases;
		if (d < 2ul) ++nrOfFailedTestCases;
		if (d < 3ull) ++nrOfFailedTestCases;
		d = 0.0;
		if (1u < d) ++nrOfFailedTestCases;
		if (2ul < d) ++nrOfFailedTestCases;
		if (3ull < d) ++nrOfFailedTestCases;

		// greater than unsigned literal
		if (d > 1u) ++nrOfFailedTestCases;
		if (d > 2ul) ++nrOfFailedTestCases;
		if (d > 3ull) ++nrOfFailedTestCases;
		d = 4ll;
		std::cout << to_binary(d) << " : " << d << '\n';
		if (1u > d) ++nrOfFailedTestCases;
		if (2ul > d) ++nrOfFailedTestCases;
		if (3ull > d) ++nrOfFailedTestCases;

		// less than or equal unsigned literal
		if (d <= 1u) ++nrOfFailedTestCases;
		if (d <= 2ul) ++nrOfFailedTestCases;
		if (d <= 3ull) ++nrOfFailedTestCases;
		d = 0.0f;
		if (1u <= d) ++nrOfFailedTestCases;
		if (2ul <= d) ++nrOfFailedTestCases;
		if (3ull <= d) ++nrOfFailedTestCases;

		// greater than or equal unsigned literal
		if (d >= 1u) ++nrOfFailedTestCases;
		if (d >= 2ul) ++nrOfFailedTestCases;
		if (d >= 3ull) ++nrOfFailedTestCases;
		d = 4.0;
		if (1u >= d) ++nrOfFailedTestCases;
		if (2ul >= d) ++nrOfFailedTestCases;
		if (3ull >= d) ++nrOfFailedTestCases;

		// comparisons with signed literals
		// less than signed literal
		d = 4.0f;
		if (d < 1) ++nrOfFailedTestCases;
		if (d < 2l) ++nrOfFailedTestCases;
		if (d < 3ll) ++nrOfFailedTestCases;
		d = 0.0;
		if (1 < d) ++nrOfFailedTestCases;
		if (2l < d) ++nrOfFailedTestCases;
		if (3ll < d) ++nrOfFailedTestCases;

		// greater than signed literal
		if (d > 1) ++nrOfFailedTestCases;
		if (d > 2l) ++nrOfFailedTestCases;
		if (d > 3ll) ++nrOfFailedTestCases;
		d = 4ll;
		if (1 > d) ++nrOfFailedTestCases;
		if (2l > d) ++nrOfFailedTestCases;
		if (3ll > d) ++nrOfFailedTestCases;

		// less than or equal signed literal
		if (d <= 1) ++nrOfFailedTestCases;
		if (d <= 2l) ++nrOfFailedTestCases;
		if (d <= 3ll) ++nrOfFailedTestCases;
		d = 0.0f;
		if (1 <= d) ++nrOfFailedTestCases;
		if (2l <= d) ++nrOfFailedTestCases;
		if (3ll <= d) ++nrOfFailedTestCases;

		// greater than or equal signed literal
		if (d >= 1) ++nrOfFailedTestCases;
		if (d >= 2l) ++nrOfFailedTestCases;
		if (d >= 3ll) ++nrOfFailedTestCases;
		d = 4.0;
		if (1 >= d) ++nrOfFailedTestCases;
		if (2l >= d) ++nrOfFailedTestCases;
		if (3ll >= d) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: logic operators\n";
		}
	}

#ifdef SHOW_STATE_SPACE
	{
		constexpr unsigned nbits = 7;
		constexpr unsigned es = 4;
		constexpr unsigned NR_VALUES = (1 << nbits);

		posit<nbits, es> a, b, c, d;
		for (unsigned i = 0; i < NR_VALUES; ++i) {
			a.set_raw_bits(i);
			float f = float(a);
			b = int(f);
			c = f;
			d = double(a);
			if (a != c && a != d) ++nrOfFailedTestCases;
			cout << setw(3) << i << ' ' << to_binary(a) << ' ' << setw(10) << a << ' ' << setw(3) << int(f) << ' ' << to_binary(b) << ' ' << b << ' ' << to_binary(c) << ' ' << to_binary(d) << endl;
		}
	}

	{
		constexpr unsigned nbits = 8;
		constexpr unsigned es = 4;
		posit<nbits, es> a, b, c, d;

		for (int i = -16; i < 16; ++i) {
			a = i;
			cout << to_binary(i) << ' ' << a << ' ' << to_binary(a) << ' ' << to_binary(-a) << ' ' << -a << ' ' << to_binary(-i) << endl;
		}
	}
#endif // SHOW_STATE_SPACE

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
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
