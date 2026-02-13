// conversion.cpp: step-by-step example of conversion of values to posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//#include <universal/number/posit1/posit1.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/internal/value/value.hpp>   // for internal::value<> (float decomposition)
// value.hpp transitively includes bitblock.hpp for the step-by-step algorithm


// convert a floating point value to a specific posit configuration. Semantically, p = v, return reference to p
template<unsigned nbits, unsigned es, typename Ty>
sw::universal::posit<nbits, es> convert_to_posit(Ty rhs) {
	constexpr unsigned fbits = std::numeric_limits<Ty>::digits - 1;
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
	bitblock<fbits> fraction_in = v.fraction();

	p.clear();
	std::cout << " construct the posit\n";
	// interpolation rule checks
	if (check_inward_projection_range<nbits, es, uint8_t>(_scale)) {    // regime dominated
		// we are projecting to minpos/maxpos
		int k = calculate_unconstrained_k<nbits, es, uint8_t>(_scale);
		k < 0 ? (_sign ? p.minneg() : p.minpos()) : (_sign ? p.maxneg() : p.maxpos());
		// we are done
		std::cout << "projection  rounding ";
	}
	else {
		constexpr unsigned pt_len = nbits + 3 + es;
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
			for (int i = 0; i < remaining_bits; i++) sb_mask.set(i);
			std::cout << sb_mask << "  mask of remainder bits\n";
		}

		// construct the untruncated posit
		std::cout << pt_bits << "  unconstrained posit: length = nbits(" << nbits << ") + es(" << es << ") + 3 guard bits: " << pt_len << '\n';
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
		std::cout << "rounding decision (blast & bafter) | (bafter & bsticky): " << (rb ? "round up\n" : "round down\n");

		bitblock<nbits> ptt;
		pt_bits <<= pt_len - len;
		std::cout << pt_bits << "  shifted posit\n";
		truncate(pt_bits, ptt);
		std::cout << ptt << "  truncated posit\n";
		if (rb) increment_bitset(ptt);
		std::cout << ptt << "  rounded posit\n";
		if (s) ptt = twos_complement(ptt);
		std::cout << ptt << "  final posit\n";
		p.setbits(ptt.to_ullong());
	}
	return p;
}


int main(int argc, char** argv)
try {
	using namespace sw::universal;
	constexpr unsigned nbits = 16;
	constexpr unsigned es = 1;

#define ONE_SAMPLE 1
#if ONE_SAMPLE
	{
		posit<nbits, es> p(-1.0);
		--p;
		float sample = float(p);
		p = convert_to_posit<nbits, es, float>(sample);
		std::cout << color_print(p) << '\n';
		std::cout << hex_format(p) << '\n';
		std::cout << p << '\n';
	}


	{
		std::cout << "Tracing conversion algorithm\n";
		long long sample = 1614591918;
		posit<32, 2> p(sample);
		std::cout << "long : " << sample << " posit : " << hex_format(p) << " rounded : " << (long long)p << '\n';
		p = convert_to_posit<32, 2, long long>(sample);
		std::cout << color_print(p) << '\n';
		std::cout << hex_format(p) << '\n';
		std::cout << p << '\n';
	}

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
		std::cout << "Sample[" << i++ << "] = " << sample << endl;
		p = convert_to_posit<nbits,es,float>(sample);
		std::cout << "********************************************************************\n";
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
