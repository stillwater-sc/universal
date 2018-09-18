// conversion.cpp: step-by-step example of conversion of values to posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
#include <posit>


// convert a floating point value to a specific posit configuration. Semantically, p = v, return reference to p
template<size_t nbits, size_t es, typename Ty>
sw::unum::posit<nbits, es> convert_to_posit(Ty rhs) {
	constexpr size_t fbits = std::numeric_limits<Ty>::digits - 1;

	using namespace std;
	using namespace sw::unum;

	value<fbits> v((Ty)rhs);
	posit<nbits, es> p;

	streamsize prec = cout.precision();
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
		cout << to_bit_string(fraction_in) << "  full fraction bits\n";

		int remaining_bits = fbits - 1 - nf;
		bool sb = false;
		if (remaining_bits > 0) {
			sb = anyAfter(fraction_in, fbits - 1 - nf);
			bitblock<fbits> sb_mask;
			for (int i = 0; i < fbits - 1 - nf; i++) sb_mask.set(i);
			cout << to_bit_string(sb_mask) << "  mask of remainder bits\n";
		}

		// construct the untruncated posit
		cout << to_bit_string(pt_bits) << "  unconstrained posit: length = nbits(" << nbits << ") + es(" << es << ") + 3 guard bits: " << pt_len << "\n";
		// pt    = BitOr[BitShiftLeft[reg, es + nf + 1], BitShiftLeft[esval, nf + 1], BitShiftLeft[fv, 1], sb];
		regime <<= es + nf + 1;
		cout << to_bit_string(regime) << "  runlength = " << run << endl;
		exponent <<= nf + 1;
		cout << to_bit_string(exponent) << "  exponent value = " << hex << esval << dec << endl;
		fraction <<= 1;
		cout << to_bit_string(fraction) << "  most significant " << nf << " fraction bits (nbits-1-run-es)\n";
		sticky_bit.set(0, sb);
		if (remaining_bits > 0) {
			cout << to_bit_string(sticky_bit) << "  sticky bit representing the truncated fraction bits\n";
		}
		else {
			cout << to_bit_string(sticky_bit) << "  sticky bit representing the fraction bits which are not truncated\n";
		}

		pt_bits |= regime;
		pt_bits |= exponent;
		pt_bits |= fraction;
		pt_bits |= sticky_bit;
		cout << to_bit_string(pt_bits) << "  unconstrained posit bits ";

		unsigned len = 1 + std::max<unsigned>((nbits + 1), (2 + run + es));
		cout << " length = " << len << endl;
		bool blast = pt_bits.test(len - nbits);
		bitblock<pt_len> blast_bb;
		blast_bb.set(len - nbits);
		cout << to_bit_string(blast_bb) << "  last bit mask\n";
		bool bafter = pt_bits.test(len - nbits - 1);
		bitblock<pt_len> bafter_bb;
		bafter_bb.set(len - nbits - 1);
		cout << to_bit_string(bafter_bb) << "  bit after last bit mask\n";
		bool bsticky = anyAfter(pt_bits, len - nbits - 1 - 1);
		bitblock<pt_len> bsticky_bb;
		for (int i = len - nbits - 2; i >= 0; --i) bsticky_bb.set(i);
		cout << to_bit_string(bsticky_bb) << "  sticky bit mask\n";

		bool rb = (blast & bafter) | (bafter & bsticky);
		cout << "rounding decision (blast & bafter) | (bafter & bsticky): " << (rb ? "round up" : "round down") << endl;

		bitblock<nbits> ptt;
		pt_bits <<= pt_len - len;
		cout << to_bit_string(pt_bits) << "  shifted posit\n";
		truncate(pt_bits, ptt);
		cout << to_bit_string(ptt) << "  truncated posit\n";
		if (rb) increment_bitset(ptt);
		cout << to_bit_string(ptt) << "  rounded posit\n";
		if (s) ptt = twos_complement(ptt);
		cout << to_bit_string(ptt) << "  final posit\n";
		p.set(ptt);
	}
	return p;
}


int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;
	constexpr size_t nbits = 16;
	constexpr size_t es = 1;

#define ONE_SAMPLE 1
#if ONE_SAMPLE
	posit<nbits, es> p(-1.0);
	--p;
	float sample = float(p);
	p = convert_to_posit<nbits, es, float>(sample);
	cout << color_print(p) << endl;

#else
	float samples[20];
	
	posit<nbits, es> p_one_minus_eps, p_one, p_one_plus_eps;
	p_one = 1.0;
	p_one_minus_eps = 1.0; --p_one_minus_eps;
	p_one_plus_eps = 1.0; ++p_one_plus_eps;

	posit<nbits, es> p_minpos, p_maxpos(NAR);
	++p_minpos;
	--p_maxpos;

	samples[0] = 0.0f;
	samples[1] = float(p_minpos) / 2.0f;
	samples[2] = float(p_minpos);
	samples[3] = float(++p_minpos);
	samples[4] = float(p_one_minus_eps);
	samples[5] = float(p_one);
	samples[6] = float(p_one_plus_eps);
	samples[7] = float(--p_maxpos);
	samples[8] = float(p_maxpos);
	samples[9] = float(p_maxpos) * 2.0f;
	samples[10] = INFINITY;
	samples[11] = -samples[9];
	samples[12] = -samples[8];
	samples[13] = -samples[7];
	samples[14] = -samples[6];
	samples[15] = -1.0f;
	samples[16] = -samples[4];
	samples[17] = -samples[3];
	samples[18] = -samples[2];
	samples[19] = -samples[1];

	posit<nbits, es> p;

	int i = 0;
	for (auto sample : samples) {
		cout << "Sample[" << i++ << "] = " << sample << endl;
		p = convert_to_posit<nbits,es,float>(sample);
		cout << "********************************************************************\n";
	}
#endif

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
