// float2posit.cpp: convert a floating-point value to a posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit1/posit1.hpp>

// convert a floating point value to a specific posit configuration. Semantically, p = v, return reference to p
template<size_t nbits, size_t es, typename Ty>
sw::universal::posit<nbits, es> convert_to_posit(Ty rhs) {
	constexpr size_t fbits = std::numeric_limits<Ty>::digits - 1;
	using namespace sw::universal;

	internal::value<fbits> v((Ty)rhs);
	posit<nbits, es> p;

	std::cout << std::setprecision(std::numeric_limits<Ty>::digits10) << v << "   input value\n";
	std::cout << "Test for ZERO\n";
	std::cout << to_triple(v);
	if (v.iszero()) {
		p.setzero();
		std::cout << " input value is zero\n";
		std::cout << info_print(p);
		return p;
	}
	else {
		std::cout << " input value is NOT zero\n";
	}
	std::cout << "Test for NaR\n";
	std::cout << to_triple(v);
	if (v.isnan() || v.isinf()) {
		p.setnar();
		std::cout << " input value is NaR\n";
		std::cout << info_print(p);
		return p;
	}
	else {
		std::cout << " input value is NOT NaR\n";
	}

	bool _sign = v.sign();
	int _scale = v.scale();
	sw::universal::bitblock<fbits> fraction_in = v.fraction();

	p.clear();
	std::cout << " construct the posit\n";
	// interpolation rule checks
	if (check_inward_projection_range<nbits, es>(_scale)) {    // regime dominated
															   // we are projecting to minpos/maxpos
		int k = calculate_unconstrained_k<nbits, es>(_scale);
		k < 0 ? p.setBitblock(minpos_pattern<nbits, es>(_sign)) : p.setBitblock(maxpos_pattern<nbits, es>(_sign));
		// we are done
		std::cout << "projection  rounding ";
	}
	else {
		constexpr size_t pt_len = nbits + 3 + es;
		bitblock<pt_len> pt_bits;
		bitblock<pt_len> regime;
		bitblock<pt_len> exponent;
		bitblock<pt_len> fraction;
		bitblock<pt_len> sticky_bit;
		bool s = _sign;
		int e = _scale;
		bool r = (e >= 0);

		unsigned run = (r ? 1 + (e >> es) : -(e >> es));
		regime.set(0, 1 ^ r);
		for (unsigned i = 1; i <= run; i++) regime.set(i, r);


		unsigned esval = e % (uint32_t(1) << es);
		exponent = convert_to_bitblock<pt_len>(esval);
		unsigned nf = (unsigned)std::max<int>(0, (nbits + 1) - (2 + run + es));

		// copy the most significant nf fraction bits into fraction
		unsigned lsb = nf <= fbits ? 0 : nf - fbits;
		for (unsigned i = lsb; i < nf; i++) fraction[i] = fraction_in[fbits - nf + i];
		std::cout << fraction_in << "  full fraction bits\n";

		int remaining_bits = fbits - 1 - nf;
		bool sb = false;
		if (remaining_bits > 0) {
			sb = anyAfter(fraction_in, fbits - 1 - nf);
			bitblock<fbits> sb_mask;
			//for (int i = 0; i < int(fbits) - 1 - nf; i++) sb_mask.set(i);
			for (int i = 0; i < remaining_bits; i++) sb_mask.set(i);
			std::cout << sb_mask << "  mask of remainder bits\n";
		}

		// construct the untruncated posit
		std::cout << pt_bits << "  unconstrained posit: length = nbits(" << nbits << ") + es(" << es << ") + 3 guard bits: " << pt_len << "\n";
		// pt    = BitOr[BitShiftLeft[reg, es + nf + 1], BitShiftLeft[esval, nf + 1], BitShiftLeft[fv, 1], sb];
		regime <<= es + nf + 1;
		std::cout << regime << "  runlength = " << run << '\n';
		exponent <<= nf + 1;
		std::cout << exponent << "  exponent value = " << std::hex << esval << std::dec << '\n';
		fraction <<= 1;
		std::cout << fraction << "  most significant " << nf << " fraction bits (nbits-1-run-es)\n";
		sticky_bit.set(0, sb);
		if (remaining_bits > 0) {
			std::cout << sticky_bit << "  sticky bit representing the truncated fraction bits\n";
		}
		else {
			std::cout << sticky_bit << "  sticky bit representing the fraction bits which are not truncated\n";
		}
		
		pt_bits |= regime;
		pt_bits |= exponent;
		pt_bits |= fraction;
		pt_bits |= sticky_bit;
		std::cout << pt_bits << "  unconstrained posit bits ";

		unsigned len = 1 + std::max<unsigned>((nbits + 1), (2 + run + es));
		std::cout << " length = " << len << '\n';
		bool blast = pt_bits.test(len - nbits);
		bitblock<pt_len> blast_bb;
		blast_bb.set(len - nbits);
		std::cout << blast_bb << "  last bit mask\n";
		bool bafter = pt_bits.test(len - nbits - 1);
		bitblock<pt_len> bafter_bb;
		bafter_bb.set(len - nbits - 1);
		std::cout << bafter_bb << "  bit after last bit mask\n";
		bool bsticky = anyAfter(pt_bits, len - nbits - 1 - 1);
		bitblock<pt_len> bsticky_bb;
		for (int i = len - nbits - 2; i >= 0; --i) bsticky_bb.set(i);
		std::cout << bsticky_bb << "  sticky bit mask\n";

		bool rb = (blast & bafter) | (bafter & bsticky);
		std::cout << "rounding decision (blast & bafter) | (bafter & bsticky): " << (rb ? "round up" : "round down") << '\n';

		bitblock<nbits> ptt;
		pt_bits <<= pt_len - len;
		std::cout << pt_bits << "  shifted posit\n";
		truncate(pt_bits, ptt);
		std::cout << ptt << "  truncated posit\n";
		if (rb) increment_bitset(ptt);
		std::cout << ptt << "  rounded posit\n";
		if (s) ptt = twos_complement(ptt);
		std::cout << ptt << "  final posit\n";
		p.setBitblock(ptt);
	}
	return p;
}

typedef std::numeric_limits< double > dbl;
const char* msg = "$ ./float2posit.exe 1.234567890 32\n\
1.23456789   input value\n\
Test for ZERO\n\
(+, 0, 0011110000001100101001000010100000111101111000011011) input value is NOT zero\n\
Test for NaR\n\
(+, 0, 0011110000001100101001000010100000111101111000011011) input value is NOT NaR\n\
construct the posit\n\
0011'1100'0000'1100'1010'0100'0010'1000'0011'1101'1110'0001'1011  full fraction bits\n\
0000'0000'0000'0000'0000'0000'0000'0111'1111'1111'1111'1111'1111  mask of remainder bits\n\
0'0000'0000'0000'0000'0000'0000'0000'0000'0000  unconstrained posit : length = nbits(32) + es(2) + 3 guard bits : 37\n\
0'0001'0000'0000'0000'0000'0000'0000'0000'0000  runlength = 1\n\
0'0000'0000'0000'0000'0000'0000'0000'0000'0000  exponent value = 0\n\
0'0000'0000'0111'1000'0001'1001'0100'1000'0100  most significant 28 fraction bits(nbits - 1 - run - es)\n\
0'0000'0000'0000'0000'0000'0000'0000'0000'0001  sticky bit representing the truncated fraction bits\n\
0'0001'0000'0111'1000'0001'1001'0100'1000'0101  unconstrained posit bits  length = 34\n\
0'0000'0000'0000'0000'0000'0000'0000'0000'0100  last bit mask\n\
0'0000'0000'0000'0000'0000'0000'0000'0000'0010  bit after last bit mask\n\
0'0000'0000'0000'0000'0000'0000'0000'0000'0001  sticky bit mask\n\
rounding decision(blast & bafter) | (bafter & bsticky) : round down\n\
0'1000'0011'1100'0000'1100'1010'0100'0010'1000  shifted posit\n\
0100'0001'1110'0000'0110'0101'0010'0001  truncated posit\n\
0100'0001'1110'0000'0110'0101'0010'0001  rounded posit\n\
0100'0001'1110'0000'0110'0101'0010'0001  final posit\n\
";

// receive a float and print its components
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc != 3) {
		std::cerr << "Show the conversion of a float to a posit step-by-step.\n";
		std::cerr << "Usage: float2posit floating_point_value posit_size_in_bits[one of 8|16|32|48|64|80|96|128|256]\n";
		std::cerr << "Example: convert -1.123456789e17 32\n";
		std::cerr <<  msg << '\n';
		return EXIT_SUCCESS;  // signal successful completion for ctest
	}
	double d = atof(argv[1]);
	int size = atoi(argv[2]);
	posit<8, 0>  p8_0;
	posit<16, 1> p16_1;
	posit<32, 2> p32_2;
	posit<48, 2> p48_2;
	posit<64, 3> p64_3;
	posit<80, 3> p80_3;
	posit<96, 3> p96_3;
	posit<128, 4> p128_4;
	posit<256, 5> p256_5;

	//int precision = dbl::max_digits10;
	switch (size) {
	case 8:
		p8_0 = convert_to_posit<8, 0, double>(d);
		std::cout << color_print(p8_0);
		break;
	case 16:
		p16_1 = convert_to_posit<16, 1, double>(d);
		std::cout << color_print(p16_1);
		break;
	case 32:
		p32_2 = convert_to_posit<32, 2, double>(d);
		std::cout << color_print(p32_2);
		break;
	case 48:
		p48_2 = convert_to_posit<48, 2, double>(d);
		std::cout << color_print(p48_2);
		break;
	case 64:
		p64_3 = convert_to_posit<64, 3, double>(d);
		std::cout << color_print(p64_3);
		break;
	case 80:
		p80_3 = convert_to_posit<80, 3, double>(d);
		std::cout << color_print(p80_3);
		break;
	case 96:
		p96_3 = convert_to_posit<96, 3, double>(d);
		std::cout << color_print(p96_3);
		break;
	case 128:
		p128_4 = convert_to_posit<128, 4, double>(d);
		std::cout << color_print(p128_4);
		break;
	case 256:
		p256_5 = convert_to_posit<256, 5, double>(d);
		std::cout << color_print(p256_5);
		break;
	default:
		p32_2 = convert_to_posit<32, 2, double>(d);
		std::cout << color_print(p32_2);
		break;
	}

	return EXIT_SUCCESS;
}
catch (const char* const msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
