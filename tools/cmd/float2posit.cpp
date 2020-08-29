// float2posit.cpp: convert a floating-point value to a posit
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/posit/posit>

// convert a floating point value to a specific posit configuration. Semantically, p = v, return reference to p
template<size_t nbits, size_t es, typename Ty>
sw::unum::posit<nbits, es> convert_to_posit(Ty rhs) {
	constexpr size_t fbits = std::numeric_limits<Ty>::digits - 1;

	using namespace std;
	using namespace sw::unum;

	value<fbits> v((Ty)rhs);
	posit<nbits, es> p;

	cout << setprecision(numeric_limits<Ty>::digits10) << v << "   input value\n";
	cout << "Test for ZERO\n";
	cout << components(v);
	if (v.iszero()) {
		p.setzero();
		cout << " input value is zero\n";
		cout << info_print(p);
		return p;
	}
	else {
		cout << " input value is NOT zero\n";
	}
	cout << "Test for NaR\n";
	cout << components(v);
	if (v.isnan() || v.isinf()) {
		p.setnar();
		cout << " input value is NaR\n";
		cout << info_print(p);
		return p;
	}
	else {
		cout << " input value is NOT NaR\n";
	}


	bool _sign = v.sign();
	int _scale = v.scale();
	sw::unum::bitblock<fbits> fraction_in = v.fraction();

	p.clear();
	cout << " construct the posit\n";
	// interpolation rule checks
	if (check_inward_projection_range<nbits, es>(_scale)) {    // regime dominated
															   // we are projecting to minpos/maxpos
		int k = calculate_unconstrained_k<nbits, es>(_scale);
		k < 0 ? p.set(minpos_pattern<nbits, es>(_sign)) : p.set(maxpos_pattern<nbits, es>(_sign));
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
		cout << fraction_in << "  full fraction bits\n";

		int remaining_bits = fbits - 1 - nf;
		bool sb = false;
		if (remaining_bits > 0) {
			sb = anyAfter(fraction_in, fbits - 1 - nf);
			bitblock<fbits> sb_mask;
			//for (int i = 0; i < int(fbits) - 1 - nf; i++) sb_mask.set(i);
			for (int i = 0; i < remaining_bits; i++) sb_mask.set(i);
			cout << sb_mask << "  mask of remainder bits\n";
		}

		// construct the untruncated posit
		cout << pt_bits << "  unconstrained posit: length = nbits(" << nbits << ") + es(" << es << ") + 3 guard bits: " << pt_len << "\n";
		// pt    = BitOr[BitShiftLeft[reg, es + nf + 1], BitShiftLeft[esval, nf + 1], BitShiftLeft[fv, 1], sb];
		regime <<= es + nf + 1;
		cout << regime << "  runlength = " << run << endl;
		exponent <<= nf + 1;
		cout << exponent << "  exponent value = " << hex << esval << dec << endl;
		fraction <<= 1;
		cout << fraction << "  most significant " << nf << " fraction bits (nbits-1-run-es)\n";
		sticky_bit.set(0, sb);
		if (remaining_bits > 0) {
			cout << sticky_bit << "  sticky bit representing the truncated fraction bits\n";
		}
		else {
			cout << sticky_bit << "  sticky bit representing the fraction bits which are not truncated\n";
		}
		
		pt_bits |= regime;
		pt_bits |= exponent;
		pt_bits |= fraction;
		pt_bits |= sticky_bit;
		cout << pt_bits << "  unconstrained posit bits ";

		unsigned len = 1 + std::max<unsigned>((nbits + 1), (2 + run + es));
		cout << " length = " << len << endl;
		bool blast = pt_bits.test(len - nbits);
		bitblock<pt_len> blast_bb;
		blast_bb.set(len - nbits);
		cout << blast_bb << "  last bit mask\n";
		bool bafter = pt_bits.test(len - nbits - 1);
		bitblock<pt_len> bafter_bb;
		bafter_bb.set(len - nbits - 1);
		cout << bafter_bb << "  bit after last bit mask\n";
		bool bsticky = anyAfter(pt_bits, len - nbits - 1 - 1);
		bitblock<pt_len> bsticky_bb;
		for (int i = len - nbits - 2; i >= 0; --i) bsticky_bb.set(i);
		cout << bsticky_bb << "  sticky bit mask\n";

		bool rb = (blast & bafter) | (bafter & bsticky);
		cout << "rounding decision (blast & bafter) | (bafter & bsticky): " << (rb ? "round up" : "round down") << endl;

		bitblock<nbits> ptt;
		pt_bits <<= pt_len - len;
		cout << pt_bits << "  shifted posit\n";
		truncate(pt_bits, ptt);
		cout << ptt << "  truncated posit\n";
		if (rb) increment_bitset(ptt);
		cout << ptt << "  rounded posit\n";
		if (s) ptt = twos_complement(ptt);
		cout << ptt << "  final posit\n";
		p.set(ptt);
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
	using namespace std;
	using namespace sw::unum;

	if (argc != 3) {
		cerr << "Show the conversion of a float to a posit step-by-step." << endl;
		cerr << "Usage: float2posit floating_point_value posit_size_in_bits[one of 8|16|32|48|64|80|96|128|256]" << endl;
		cerr << "Example: convert -1.123456789e17 32" << endl;
		cerr <<  msg << endl;
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
		cout << color_print(p8_0);
		break;
	case 16:
		p16_1 = convert_to_posit<16, 1, double>(d);
		cout << color_print(p16_1);
		break;
	case 32:
		p32_2 = convert_to_posit<32, 2, double>(d);
		cout << color_print(p32_2);
		break;
	case 48:
		p48_2 = convert_to_posit<48, 2, double>(d);
		cout << color_print(p48_2);
		break;
	case 64:
		p64_3 = convert_to_posit<64, 3, double>(d);
		cout << color_print(p64_3);
		break;
	case 80:
		p80_3 = convert_to_posit<80, 3, double>(d);
		cout << color_print(p80_3);
		break;
	case 96:
		p96_3 = convert_to_posit<96, 3, double>(d);
		cout << color_print(p96_3);
		break;
	case 128:
		p128_4 = convert_to_posit<128, 4, double>(d);
		cout << color_print(p128_4);
		break;
	case 256:
		p256_5 = convert_to_posit<256, 5, double>(d);
		cout << color_print(p256_5);
		break;
	default:
		p32_2 = convert_to_posit<32, 2, double>(d);
		cout << color_print(p32_2);
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
