// conversion_functions.cpp : api experiments for conversion algorithms
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
// minimum set of include files
#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"

template<size_t nbits, size_t es>
void GenerateLogicPattern(double input, const sw::unum::posit<nbits, es>& presult, const sw::unum::posit<nbits + 1, es>& pnext) {
	const int VALUE_WIDTH = 15;
	bool fail = presult != pnext;
	sw::unum::value<52> v(input);
	std::cout << std::setw(VALUE_WIDTH) << input << " "
		<< " result " << std::setw(VALUE_WIDTH) << presult
		<< "  scale= " << std::setw(3) << presult.scale()
		<< "  k= " << std::setw(3) << sw::unum::calculate_k<nbits, es>(v.scale())
		<< "  exp= " << std::setw(3) << presult.get_exponent() << "  "
		<< presult.get() << " "
		<< pnext.get() << " "
		<< std::setw(VALUE_WIDTH) << pnext << " "
		<< (fail ? "FAIL" : "    PASS")
		<< std::endl;
}

template<size_t nbits, size_t es>
void GenerateLogicPatternsForDebug() {
	// we are going to generate a test set that consists of all posit configs and their midpoints
	// we do this by enumerating a posit that is 1-bit larger than the test posit configuration
	const int NR_TEST_CASES = (1 << (nbits + 1));
	const int HALF = (1 << nbits);
	sw::unum::posit<nbits + 1, es> pref, pprev, pnext;

	// execute the test
	const double eps = 1.0e-10;  // TODO for big posits, eps is important to resolve differences
	double da, input;
	sw::unum::posit<nbits, es> pa;
	std::cout << sw::unum::dynamic_range(pa) << std::endl;
	for (int i = 0; i < NR_TEST_CASES; i++) {
		pref.set_raw_bits(i);
		da = double(pref);
		if (i % 2) {
			if (i == 1) {
				// special case of projecting to +minpos
				// even the -delta goes to +minpos
				input = da - eps;
				pa = input;
				pnext.set_raw_bits(i + 1);
				std::cout << "p"; // indicate that this needs to 'project'
				GenerateLogicPattern(input, pa, pnext);
				input = da + eps;
				pa = input;
				std::cout << "p"; // indicate that this needs to 'project'
				GenerateLogicPattern(input, pa, pnext);

			}
			else if (i == HALF - 1) {
				// special case of projecting to +maxpos
				input = da - eps;
				pa = input;
				pprev.set_raw_bits(HALF - 2);
				std::cout << "p"; // indicate that this needs to 'project'
				GenerateLogicPattern(input, pa, pprev);
			}
			else if (i == HALF + 1) {
				// special case of projecting to -maxpos
				input = da - eps;
				pa = input;
				pprev.set_raw_bits(HALF + 2);
				std::cout << "p"; // indicate that this needs to 'project'
				GenerateLogicPattern(input, pa, pprev);
			}
			else if (i == NR_TEST_CASES - 1) {
				// special case of projecting to -minpos
				// even the +delta goes to -minpos
				input = da - eps;
				pa = input;
				pprev.set_raw_bits(i - 1);
				std::cout << "p"; // indicate that this needs to 'project'
				GenerateLogicPattern(input, pa, pprev);
				input = da + eps;
				pa = input;
				std::cout << "p"; // indicate that this needs to 'project'
				GenerateLogicPattern(input, pa, pprev);
			}
			else {
				// for odd values, we are between posit values, so we create the round-up and round-down cases
				// round-down
				input = da - eps;
				pa = input;
				pprev.set_raw_bits(i - 1);
				std::cout << "d"; // indicate that this needs to round down
				GenerateLogicPattern(input, pa, pprev);
				// round-up
				input = da + eps;
				pa = input;
				pnext.set_raw_bits(i + 1);
				std::cout << "u"; // indicate that this needs to round up
				GenerateLogicPattern(input, pa, pnext);
			}
		}
		else {
			// for the even values, we generate the round-to-actual cases
			if (i == 0) {
				// special case of projecting to +minpos
				input = da + eps;
				pa = input;
				pnext.set_raw_bits(i + 2);
				std::cout << "p"; // indicate that this needs to 'project'
				GenerateLogicPattern(input, pa, pnext);
			}
			else if (i == NR_TEST_CASES - 2) {
				// special case of projecting to -minpos
				input = da - eps;
				pa = input;
				pprev.set_raw_bits(NR_TEST_CASES - 2);
				std::cout << "p"; // indicate that this needs to 'project'
				GenerateLogicPattern(input, pa, pprev);
			}
			else {
				// round-up
				input = da - eps;
				pa = input;
				std::cout << "u"; // indicate that this needs to round up
				GenerateLogicPattern(input, pa, pref);
				// round-down
				input = da + eps;
				pa = input;
				std::cout << "d"; // indicate that this needs to round down
				GenerateLogicPattern(input, pa, pref);
			}
		}
	}
}

template<size_t nbits>
std::string LowerSegment(sw::unum::bitblock<nbits>& bits, unsigned msb) {
	std::stringstream ss;
	for (int i = msb; i >= 0; i--) {
		if (bits.test(i)) {
			ss << "1";
		}
		else {
			ss << "0";
		}
	}
	return ss.str();
}
template<size_t src_size, size_t nbits>
void CopyLowerSegment(sw::unum::bitblock<src_size>& src, sw::unum::bitblock<nbits>& tgt, unsigned msb = nbits-1) {
	for (int i = msb; i >= 0; i--) {
		tgt[i] = src[i];
	}
}
template<size_t nbits, size_t src_size>
sw::unum::bitblock<nbits> CopyInto(sw::unum::bitblock<src_size>& src) {
	sw::unum::bitblock<nbits> tgt;
	for (int i = nbits - 1; i >= 0; i--) {
		tgt.set(i, src[i]);
	}
	return tgt;
}

// calculate the 2's complement of a 2's complement encoded number
template<size_t nbits>
sw::unum::bitblock<nbits> _twos_complement(sw::unum::bitblock<nbits> number) {
	sw::unum::bitblock<nbits> complement;
	uint8_t _slice = 0;
	uint8_t carry = 1;
	for (size_t i = 0; i < nbits; i++) {
		_slice = uint8_t(!number[i]) + carry;
		carry = _slice >> 1;
		complement[i] = (0x1 & _slice);
	}
	return complement;
}

template<size_t nbits>
bool increment_unsigned(sw::unum::bitblock<nbits>& number, int nrBits = nbits - 1) {
	bool carry = 1;  // ripple carry
	int lsb = nbits - nrBits;
	for (size_t i = lsb; i < nbits; i++) {
		bool _a = number[i];
		number[i] = _a ^ carry;
		carry = (_a & false) | (carry & (_a ^ false));
	}
	return carry;
}

// increment the input bitblock in place, and return true if there is a carry generated.
template<size_t nbits>
bool increment_bitblock(sw::unum::bitblock<nbits>& number) {
	bool carry = true;  // ripple carry
	for (size_t i = 0; i < nbits; i++) {
		bool _a = number[i];
		number[i] = _a ^ carry;
		carry = carry & (_a ^ false);
	}
	return carry;
}

// decrement the input bitblock in place, and return true if there is a borrow generated.
template<size_t nbits>
bool decrement_bitblock(sw::unum::bitblock<nbits>& number) {
	bool borrow = true;
	for (size_t i = 0; i < nbits; i++) {
		bool _a = number[i];
		number[i] = _a ^ borrow;
		borrow = (!(!_a ^ true) & borrow);
	}
	return borrow;
}

// DANGER: this depends on the implicit type conversion of number to a uint64_t to sign extent a 2's complement number system
// if nbits > 64 then this code breaks.
template<size_t nbits, class Type>
sw::unum::bitblock<nbits> _convert_to_bitblock(Type number) {
	sw::unum::bitblock<nbits> _Bits;
	uint64_t mask = uint64_t(1);
	for (size_t i = 0; i < nbits; i++) {
		_Bits[i] = mask & number;
		mask <<= 1;
	}
	return _Bits;
}

// sticky bit representation of all the bits from [msb, lsb], that is, msb is included
template<size_t nbits>
bool _anyAfter(const sw::unum::bitblock<nbits>& bits, unsigned msb) {
	bool running = false;
	for (int i = msb; i >= 0; i--) {
		running |= bits.test(i);
	}
	return running;
}

/*
p[x_] := Module[{s, y, r, e, f, run, reg, esval, nf, len, fv, sb, pt, blast, bafter, bsticky, rb, ptt, p},
s     = Boole[x < 0];
y     = Max[minpos, Min[maxpos, Abs[x]]];
r     = Boole[y ≥ 1];
e     = Floor[Log[2, y]];
f     = y / 2^e - 1;
run   = Abs[Floor[e / 2^es]] + r;
reg   = BitOr[BitShiftLeft[r * (2^run - 1), 1], BitXor[1, r]];
esval = Mod[e, 2^es];
nf    = Max[0, (nbits + 1) - (2 + run + es)];
len   = 1 + Max[nbits + 1, 2 + run + es];
fv    = Floor[f * 2^nf];
sb    = Boole[f * 2^nf > fv];
pt    = BitOr[BitShiftLeft[reg, es + nf + 1], BitShiftLeft[esval, nf + 1], BitShiftLeft[fv, 1], sb];
blast   = BitGet[pt, len - nbits];
bafter  = BitGet[pt, len - nbits - 1];
bsticky = Boole[BitAnd[2len-nbits-1 - 1, pt] > 0];
rb      = BitOr[BitAnd[blast, bafter], BitAnd[bafter, bsticky]];
ptt     = BitShiftRight[pt, len - nbits] + rb;
BitXor[s * (2^nbits - 1), ptt] + s]
 */
template<size_t nbits, size_t es>
void convert_to_posit(float x, bool bPrintIntermediateSteps = false) {
	using namespace std;
	using namespace sw::unum;

	cout << "convert to posit<" << nbits << "," << es << ">" << endl;
	// obtain the sign/scale/fraction representation of a float
	constexpr int nrfbits = std::numeric_limits<float>::digits - 1;
	value<nrfbits> v(x);
	// ignore for the sake of clarity the special cases 0 and NaR (Not a Real)
	//bool sign = v.sign();
	int scale = v.scale();
	bitblock<nrfbits> bits = v.fraction();
	cout << v << " = " << components(v) << endl;

	float minpos = (float)minpos_value<nbits, es>();
	float maxpos = (float)maxpos_value<nbits, es>();

	const size_t pt_len = nbits + 3 + es;
	bitblock<pt_len> pt_bits;
	bitblock<pt_len> regime;
	bitblock<pt_len> exponent;
	bitblock<pt_len> fraction;
	bitblock<pt_len> sticky_bit;

	bool s = (x < 0);
	if (bPrintIntermediateSteps) cout << "s        = " << (s ? "negative" : "positive") << endl;
	if (bPrintIntermediateSteps) cout << "x        = " << (float)x << endl;
	if (bPrintIntermediateSteps) cout << "Abs(x)   = " << (float)std::abs(x) << endl;
	float y = std::max<float>(minpos, std::min<float>(maxpos, (float)std::abs(x)));
	if (bPrintIntermediateSteps) cout << "y        = " << y << endl;
	bool r = (y >= 1.0f);
	if (bPrintIntermediateSteps) cout << "r        = " << (r ? "1" : "0") << endl;
#if (defined(__SUNPRO_C) || defined(__SUNPRO_CC)) && ((__SUNPRO_C < 0x5150) || (__SUNPRO_CC < 0x5150))
	float e = std::floor(log2f(y));
#else
	float e = std::floor(std::log2(y));
#endif
	if (bPrintIntermediateSteps) cout << "e        = " << e << endl;
	float f = y / float(pow(2.0, scale)) - 1.0f;
	if (bPrintIntermediateSteps) cout << "f        = " << f << endl;
	if (bPrintIntermediateSteps) cout << "bits     = " << bits << endl;
	int run = (int)std::abs(std::floor(e / pow(2, es))) + r;
	if (bPrintIntermediateSteps) cout << "run      = " << run << endl;
	unsigned _run = (r ? 1 + (scale >> es) : -(scale >> es));
	if (bPrintIntermediateSteps) cout << "_run     = " << _run << endl;
	// reg   = BitOr[BitShiftLeft[r * (2^run - 1), 1], BitXor[1, r]];
	regime.set(0, 1 ^ r);
	for (int i = 1; i <= run; i++) regime.set(i, r);
	if (bPrintIntermediateSteps) cout << "reg      = " << LowerSegment(regime,run) << endl;
	sw::unum::regime<nbits, es> _reg; _reg.assign(scale);
	if (bPrintIntermediateSteps) cout << "_reg     = " << _reg << endl;
	unsigned esval = scale % (uint32_t(1) << es);
	if (bPrintIntermediateSteps) cout << "esval    = " << esval << endl;
	exponent = _convert_to_bitblock<pt_len>(esval);
	unsigned nf = (unsigned)std::max<int>(0, (nbits + 1) - (2 + run + es));
	if (bPrintIntermediateSteps) cout << "nf       = " << nf << endl;
	// copy the most significant nf fraction bits into fraction
	for (int i = 0; i < (int)nf; i++) fraction[i] = bits[nrfbits - nf + i];
	if (bPrintIntermediateSteps) cout << "fraction = " << fraction << endl;
	float fv = (float)std::floor((double)(f * (unsigned(1) << nf)));
	if (bPrintIntermediateSteps) cout << "fv       = " << fv << endl;
	bool sb = ((f * (unsigned(1) << nf)) > fv);
	if (bPrintIntermediateSteps) cout << "sb       = " << (sb ? "1" : "0") << endl;

	// construct the bigger posit
	// pt    = BitOr[BitShiftLeft[reg, es + nf + 1], BitShiftLeft[esval, nf + 1], BitShiftLeft[fv, 1], sb];
	regime   <<= es + nf + 1;
	exponent <<= nf + 1;
	fraction <<= 1;
	sticky_bit.set(0, sb);

	if (bPrintIntermediateSteps) {
		cout << "regime   = " << regime << endl;
		cout << "exponent = " << exponent << endl;
		cout << "fraction = " << fraction << endl;
		cout << "sticky   = " << sticky_bit << endl;
	}
	pt_bits |= regime;
	pt_bits |= exponent;
	pt_bits |= fraction;
	pt_bits |= sticky_bit;

	if (bPrintIntermediateSteps) cout << "pt bits  = " << pt_bits << endl;
	if (bPrintIntermediateSteps) cout << "pt bits  = " << LowerSegment(pt_bits, 2 + run + es) << endl;
	unsigned len = 1 + std::max<unsigned>((nbits + 1), (2 + run + es));
	if (bPrintIntermediateSteps) cout << "pt_len   = " << pt_len << endl;
	if (bPrintIntermediateSteps) cout << "len      = " << len << endl;
	if (bPrintIntermediateSteps) cout << "blast at = " << len - nbits << endl;
	bool blast = pt_bits.test(len - nbits);
	bool bafter = pt_bits.test(len - nbits - 1);
	bool bsticky = _anyAfter(pt_bits, len - nbits - 1 - 1);
	if (bPrintIntermediateSteps) cout << "blast    = " << blast << endl;
	if (bPrintIntermediateSteps) cout << "bafter   = " << bafter << endl;
	if (bPrintIntermediateSteps) cout << "bsticky  = " << bsticky << endl;

	bool rb = (blast & bafter) | (bafter & bsticky);
	cout << "rb       = " << rb << endl;
	bitblock<pt_len> ptt = pt_bits;
	ptt >>= (len - nbits);
	if (bPrintIntermediateSteps) cout << "ptt      = " << ptt << endl;
	if (rb) increment_bitblock(ptt);
	if (s) ptt = _twos_complement(ptt);
	cout << "posit<" << nbits << "," << es << "> = " << LowerSegment(ptt, nbits-1) << endl;

	bitblock<nbits> ptt_t;
	CopyLowerSegment(ptt, ptt_t);
	posit<nbits, es> p;
	p.set_raw_bits(ptt_t.to_ullong());
	cout << "p = " << components(p) << endl;
}

template<size_t nbits, size_t es, size_t nrfbits>
sw::unum::posit<nbits, es> convert_to_posit(sw::unum::value<nrfbits> v, bool bPrintIntermediateSteps = false) {
	using namespace std;
	using namespace sw::unum;

	cout << "convert to posit<" << nbits << "," << es << ">" << endl;
	// ignore for the sake of clarity the special cases 0 and NaR (Not a Real)
	bitblock<nrfbits> bits = v.fraction();

	//float minpos = (float)sw::unum::minpos_value<nbits, es>();
	//float maxpos = (float)sw::unum::maxpos_value<nbits, es>();

	const size_t pt_len = nbits + 3 + es;
	bitblock<pt_len> pt_bits;
	bitblock<pt_len> regime;
	bitblock<pt_len> exponent;
	bitblock<pt_len> fraction;
	bitblock<pt_len> sticky_bit;

	bool s = v.sign();
	int e = v.scale();
	bool r = (e >= 0);

	unsigned run = (r ? 1 + (e >> es) : -(e >> es));
	regime.set(0, 1 ^ r);
	for (unsigned i = 1; i <= run; i++) regime.set(i, r);

	unsigned esval = e % (uint32_t(1) << es);
	exponent = _convert_to_bitblock<pt_len>(esval);
	unsigned nf = (unsigned)std::max<int>(0, (nbits + 1) - (2 + run + es));
	// copy the most significant nf fraction bits into fraction
	for (int i = 0; i < (int)nf; i++) fraction[i] = bits[nrfbits - nf + i];
	
	//float f = y / float(pow(2.0, scale)) - 1.0f;
	//float fv = (float)std::floor((double)(f * (unsigned(1) << nf)));
	//bool sb = ((f * (unsigned(1) << nf)) > fv);
	bool sb = anyAfter(bits, nrfbits - 1 - nf);

	// construct the untruncated posit
	// pt    = BitOr[BitShiftLeft[reg, es + nf + 1], BitShiftLeft[esval, nf + 1], BitShiftLeft[fv, 1], sb];
	regime <<= es + nf + 1;
	exponent <<= nf + 1;
	fraction <<= 1;
	sticky_bit.set(0, sb);

	pt_bits |= regime;
	pt_bits |= exponent;
	pt_bits |= fraction;
	pt_bits |= sticky_bit;
	cout << "pt_bits  = " << pt_bits << endl;

	unsigned len = 1 + std::max<unsigned>((nbits + 1), (2 + run + es));
	bool blast = pt_bits.test(len - nbits);
	bool bafter = pt_bits.test(len - nbits - 1);
	bool bsticky = anyAfter(pt_bits, len - nbits - 1 - 1);

	bool rb = (blast & bafter) | (bafter & bsticky);

	pt_bits <<= pt_len - len;
	bitblock<nbits> ptt;
	truncate(pt_bits, ptt);
	cout << "ptt      = " << ptt << endl;
	//ptt >>= (len - nbits);
	if (rb) increment_bitblock(ptt);
	if (s) ptt = twos_complement(ptt);
	if (bPrintIntermediateSteps) {
		cout << "s        = " << (s ? "1" : "0") << endl;
		cout << "e        = " << e << endl;
		cout << "r        = " << (r ? "1" : "0") << endl;
		cout << "run      = " << run << endl;
		cout << "reg      = " << regime << endl;
		cout << "esval    = " << esval << endl;
		cout << "nf       = " << nf << endl;
		cout << "bits     = " << bits << endl;
		cout << "fraction = " << fraction << endl;
		cout << "sb       = " << sb << endl;
		cout << "pt_len   = " << pt_len << endl;
		cout << "len      = " << len << endl;
		cout << "blast at = " << len - nbits << endl;
		cout << "regime   = " << regime << endl;
		cout << "exponent = " << exponent << endl;
		cout << "fraction = " << fraction << endl;
		cout << "sticky   = " << sticky_bit << endl;
		cout << "pt_bits  = " << pt_bits << endl;
		cout << "blast    = " << blast << endl;
		cout << "bafter   = " << bafter << endl;
		cout << "bsticky  = " << bsticky << endl;
		cout << "rb       = " << rb << endl;

		cout << "ptt      = " << ptt << endl;
	}
	cout << "posit<" << nbits << "," << es << "> = " << LowerSegment(ptt, nbits - 1) << endl;
	
	sw::unum::posit<nbits, es> p;
	p.set(ptt);
	cout << "p = " << p.to_float() << endl;
	return p;
}

// basic concept is that we are building a 'maximum size' posit, apply the rounding to it
// and then apply the nbits constraint to truncate to the final posit size.
template<size_t nbits, size_t es>
void posit_component_conversion(float x, bool bPrintIntermediateSteps = false) {
	sw::unum::value<23> v(x);
	int scale = v.scale();
	
	unsigned run = (scale >= 0 ? 1 + (scale >> es) : -scale >> es);
	int k = sw::unum::calculate_k<nbits, es>(scale);
	if (bPrintIntermediateSteps) std::cout << "k        = " << k << std::endl;
	sw::unum::regime<nbits, es> _regime;
	unsigned nr_of_regime_bits = _regime.assign(scale);
	if (bPrintIntermediateSteps) std::cout << "regime   = " << _regime << " rbits " << nr_of_regime_bits << std::endl;
	sw::unum::exponent<nbits, es> _exponent;
	_exponent.assign(scale);
	if (bPrintIntermediateSteps) std::cout << "exponent = " << _exponent << std::endl;
	unsigned nf = (unsigned)std::max<int>(0, (nbits + 1) - (2 + run + es));
	if (bPrintIntermediateSteps) std::cout << "nf       = " << nf << std::endl;
	sw::unum::bitblock<23> fraction_bitblock = v.fraction();
	sw::unum::fraction<23> _fraction;
	bool sb = _fraction.assign<23>(nf, fraction_bitblock, nf+1);  // assign and create sticky bit
	if (bPrintIntermediateSteps) std::cout << "sb       = " << sb << std::endl;
	// 	assess if we need to round up the truncated posit
/*
		unsigned len = 1 + std::max<unsigned>((nbits + 1), (2 + run + es));
		if (bPrintIntermediateSteps) std::cout << "len      = " << len << std::endl;
		if (bPrintIntermediateSteps) std::cout << "blast at = " << len - nbits << std::endl;
		bool blast = pt_bits.test(len - nbits);
		bool bafter = pt_bits.test(len - nbits - 1);
		bool bsticky = Any(pt_bits, len - nbits - 1 - 1);
		if (bPrintIntermediateSteps) std::cout << "blast    = " << blast << std::endl;
		if (bPrintIntermediateSteps) std::cout << "bafter   = " << bafter << std::endl;
		if (bPrintIntermediateSteps) std::cout << "bsticky  = " << bsticky << std::endl;
		bool rb = (blast & bafter) | (bafter & bsticky);
		if (bPrintIntermediateSteps) std::cout << "rb       = " << rb << std::endl;
		bitblock<pt_len> ptt = pt_bits;
		ptt >>= (len - nbits);

	if (roundUp) {
		bool carry = _fraction.increment();
		if (carry && es > 0) carry = _exponent.increment();
		if (carry) carry = _regime.increment();
		if (carry) std::cout << "Error" << std::endl;
	}		
*/
}



	// a posit has the form: useed^k * 2^exp * 1.fraction
	// useed^k is the regime and is encoded by the run length m of:
	//   - a string of 0's for numbers [0,1), and 
	//   - a string of 1's for numbers [1,inf)

	// The value k ranges from [1-nbits,nbits-2]
	//  m  s-regime   k  
	//  ...
	//  4  0-00001   -4
	//  3  0-0001    -3
	//  2  0-001     -2
	//  1  0-01      -1
	//  1  0-10       0
	//  2  0-110      1
	//  3  0-1110     2
	//  4  0-11110    3
	//  ...
	//

	// algorithm: convert int64 to posit<nbits,es>
	// step 1: find base regime
	//         if int64 is positive
	//            base regime = useed ^ k, where k = msb_of_int64 >> es
	//         else
	//            negate int64
	//            base regime = useed ^ k, where k = msb_of_negated_int64 >> es
	// step 2: find exponent
	//         exp = msb % 2^es
	// step 3: extract remaining fraction
	//         remove hidden bit
	// step 4: if int64 is negative, take 2's complement the posit of positive int64 calculated above
	//



constexpr int SE_QUANDRANT = 0;
constexpr int NE_QUANDRANT = 1;
constexpr int NW_QUANDRANT = 2;
constexpr int SW_QUANDRANT = 3;

template<size_t nbits, size_t es>
void GenerateTestSample(int quadrant, bool bPrintIntermediateSteps = false) {
	using namespace std;
	using namespace sw::unum;

	posit<nbits, es> p;
	cout << endl << endl << "-------------------------------------------" << endl;
	cout << dynamic_range(p) << endl;
	cout << components_to_string(p) << endl;

	int index;
	float sign_factor = 1.0;
	switch (quadrant) {
	case SE_QUANDRANT:
		index = 1;
		break;
	case NE_QUANDRANT:
		index = ( int(1) << (nbits-1) ) - 2;
		break;
	case NW_QUANDRANT:
		index = (int(1) << (nbits - 1) ) + 1;
		sign_factor = -1.0;
		break;
	case SW_QUANDRANT:
		index = -2;
		sign_factor = -1.0;
		break;
	}
	p.set_raw_bits(index);		cout << components_to_string(p) << endl; 	float f1 = p.to_float();
	p.set_raw_bits(index+1);	cout << components_to_string(p) << endl;	float f2 = p.to_float();
	p.set_raw_bits(index+2);	cout << components_to_string(p) << endl;	float f3 = p.to_float();

	float eps = float(f1 / 100000.0);
	float f_mineps, f, f_pluseps;
	std::string roundingType;
	if (es > 0) {
		// geometric rounding in this region
		f = sign_factor * std::sqrt(f1 * f2);
		roundingType = "geometric";
	} else {
		// arithmetic rounding in this region
		f = (float)((f1 + f2)/2.0);
		roundingType = "arithmetic";
	}
	f_mineps = (float)(f - eps);
	f_pluseps = (float)(f + eps);
	value<23> v_mineps(f_mineps);
	value<23> v(f);
	value<23> v_pluseps(f_pluseps);
	cout << roundingType << " mean - eps: " << f_mineps  << " " << components(v_mineps) << endl;
	cout << roundingType << " mean      : " << f         << " " << components(v) << endl;
	cout << roundingType << " mean + eps: " << f_pluseps << " " << components(v_pluseps) << endl;
	convert_to_posit<nbits, es>(f_mineps, bPrintIntermediateSteps);
	posit_component_conversion<nbits, es>(f_mineps, bPrintIntermediateSteps);
	convert_to_posit<nbits, es>(f, bPrintIntermediateSteps);
	posit_component_conversion<nbits, es>(f, bPrintIntermediateSteps);
	convert_to_posit<nbits, es>(f_pluseps, bPrintIntermediateSteps);
	posit_component_conversion<nbits, es>(f_pluseps, bPrintIntermediateSteps);

	posit<nbits, es> p1(f1), p2(f2), p3(f3);
	cout << components_to_string(p1) << endl;
	cout << components_to_string(p2) << endl;
	cout << components_to_string(p3) << endl;
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::unum;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Conversion failed: ";

#if MANUAL_TESTING
	const size_t nbits = 5;
	const size_t es = 0;
/*
	bool bPrintIntermediateResults = true;
	GenerateTestSample<nbits, es>(SE_QUANDRANT, bPrintIntermediateResults);
	GenerateTestSample<nbits, es>(NE_QUANDRANT, bPrintIntermediateResults);
	GenerateTestSample<nbits, es>(NW_QUANDRANT, bPrintIntermediateResults);
	GenerateTestSample<nbits, es>(SW_QUANDRANT, bPrintIntermediateResults);
*/
	float f1 = 1.125f;
	float f2 = 1.126f;
	float f3 = 1.25f;
	convert_to_posit<nbits, es>(f1, true);
	convert_to_posit<nbits, es>(f2, true);
	convert_to_posit<nbits, es>(f3, true);
	GeneratePositTable<5, 0>(cout);

#else
	ReportPositScales();

	GenerateLogicPatternsForDebug<5, 0>();
	GenerateLogicPatternsForDebug<5, 1>();
	GenerateLogicPatternsForDebug<5, 2>();

#if STRESS_TESTING

#endif // STRESS_TESTING

#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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


