// conversion_functions.cpp : api experiments for conversion algorithms
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"

using namespace std;
using namespace sw::unum;

template<size_t nbits, size_t es>
void checkSpecialCases(posit<nbits, es> p) {
	cout << "posit is " << (p.isZero() ? "zero " : "non-zero ") << (p.isPositive() ? "positive " : "negative ") << (p.isInfinite() ? "+-infinite" : "not infinite") << endl;
}

void ConversionExamplesPositiveRegime() {
	const size_t nbits = 5;
	const size_t es = 1;
	posit<nbits, es> p0, p1, p2, p3, p4, p5, p6;

	cout << "Minpos = " << setprecision(21) << minpos_value<nbits,es>() << endl;
	cout << "Maxpos = " << maxpos_value<nbits,es>() << setprecision(0) << endl;

	int64_t number = 1;
	for (int i = 0; i < 8; i++) {
		p0 = number;
		cout << p0 << endl;
		number <<= 1;
	}

	return;

	p0 = int(0);  checkSpecialCases(p0);
	p1 = char(1);  cout << "p1 " << p1 << endl;
	p2 = long(2);  cout << "p2 " << p2 << endl;
	p3 =  4;  cout << "p3 " << p3 << endl;
	p4 =  8;  cout << "p4 " << p4 << endl;
	p5 = 16;  cout << "p5 " << p5 << endl;
	p6 = 32;  cout << "p6 " << p6 << endl;
}

void ConversionExamplesNegativeRegime() {
	const size_t nbits = 5;
	const size_t es = 1;
	posit<nbits, es> p0, p1, p2, p3, p4, p5, p6;

	cout << "Minpos = " << setprecision(21) << minpos_value<nbits, es>() << endl;
	cout << "Maxpos = " << maxpos_value<nbits, es>() << setprecision(0) << endl;

	p0 = 0;  checkSpecialCases(p0);
	p1 = -1;  cout << "p1 " << p1 << endl;
	p2 = -2;  cout << "p2 " << p2 << endl;
	p3 = -4;  cout << "p3 " << p3 << endl;
	p4 = -8;  cout << "p4 " << p4 << endl;
	p5 = -16; cout << "p5 " << p5 << endl;
	p6 = -32; cout << "p6 " << p6 << endl;
}

// what are you trying to capture with this method? TODO
// return the position of the msb of the largest binary number representable by this posit?
// this would be the scale of maxpos
template<size_t nbits, size_t es>
unsigned int maxpos_scale_f()  {
	return (nbits-2) * (1 << es);
}

unsigned int scale(unsigned int max_k, unsigned int es) {
	return max_k * (1 << es);
}

unsigned int base_regime(int64_t value, unsigned int es) {
	return (findMostSignificantBit(value) - 1) >> es;
}

unsigned int base_exponent(unsigned int msb, unsigned int es) {
	unsigned int value = 0;
	if (es > 0) {
		value = msb % (1 << es);
	}
	return value;
}

uint64_t base_fraction(int64_t value) {
	unsigned int hidden_bit_at = findMostSignificantBit(value) - 1;
	uint64_t mask = ~(uint64_t(1) << hidden_bit_at);
	return value & mask;
}

void EnumerationTests() {
	// set the max es we want to evaluate. useed grows very quickly as a function of es
	int max_es = 4;

	// cycle through the k values to test the scale calculation
	// since useed^k grows so quickly, we can't print the value, 
	// so instead we just print the scale of the number as measured in the binary exponent of useed^k = k*2^es
	cout << setw(10) << "posit size" << setw(6) << "max_k" << "   scale of max regime" << endl;
	cout << setw(16) << "           ";
	for (int i = 0; i < max_es; i++) {
		cout << setw(5) << "es@" << i;
	}
	cout << endl;
	for (int max_k = 1; max_k < 14; max_k++) {
		cout << setw(10) << max_k + 2 << setw(6) << max_k;
		for (int es = 0; es < max_es; es++) {
			cout << setw(6) << scale(max_k, es);
		}
		cout << endl;
	}

	// cycle through scales to test the regime determination
	cout << setw(10) << "Value";
	for (int i = 0; i < max_es; i++) {
		cout << setw(7) << "k";
	}
	cout << endl;
	unsigned int value = 1;
	for (int i = 0; i < 16; i++) {
		cout << setw(10) << value;
		for (int es = 0; es < max_es; es++) {
			cout << setw(7) << base_regime(value, es);
		}
		cout << endl;
		value <<= 1;
	}

	// cycle through a range to test the exponent extraction
	for (int i = 0; i < 32; i++) {
		cout << setw(10) << i;
		for (int es = 0; es < max_es; es++) {
			cout << setw(5) << base_exponent(i, es);
		}
		cout << endl;
	}

	// cycle through a range to test the faction extraction
	for (int i = 0; i < 32; i++) {
		cout << setw(10) << hex << i << setw(5) << base_fraction(i) << endl;
	}
}

template<size_t nbits, size_t es>
void GenerateLogicPattern(double input, const posit<nbits, es>& presult, const posit<nbits + 1, es>& pnext) {
	const int VALUE_WIDTH = 15;
	bool fail = fabs(presult.to_double() - pnext.to_double()) > 0.000000001;
	value<52> v(input);
	std::cout << setw(VALUE_WIDTH) << input << " "
		<< " result " << setw(VALUE_WIDTH) << presult
		<< "  scale= " << std::setw(3) << presult.scale()
		<< "  k= " << std::setw(3) << calculate_k<nbits, es>(v.scale())
		<< "  exp= " << std::setw(3) << presult.get_exponent() << "  "
		<< presult.get() << " "
		<< pnext.get() << " "
		<< setw(VALUE_WIDTH) << pnext << " "
		<< (fail ? "FAIL" : "    PASS")
		<< std::endl;
}

template<size_t nbits, size_t es>
void GenerateLogicPatternsForDebug() {
	// we are going to generate a test set that consists of all posit configs and their midpoints
	// we do this by enumerating a posit that is 1-bit larger than the test posit configuration
	const int NR_TEST_CASES = (1 << (nbits + 1));
	const int HALF = (1 << nbits);
	posit<nbits + 1, es> pref, pprev, pnext;

	// execute the test
	int nrOfFailedTests = 0;
	const double eps = 1.0e-10;  // TODO for big posits, eps is important to resolve differences
	double da, input;
	posit<nbits, es> pa;
	std::cout << spec_to_string(pa) << std::endl;
	for (int i = 0; i < NR_TEST_CASES; i++) {
		pref.set_raw_bits(i);
		da = pref.to_double();
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
std::string LowerSegment(std::bitset<nbits>& bits, unsigned msb) {
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

template<size_t nbits>
bool Any(const std::bitset<nbits>& bits, unsigned msb) {
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
void convert_to_posit(float x) {
	cout << "<" << nbits << "," << es << ">" << endl;
	// obtain the sign/scale/fraction representation of a float
	constexpr int nrfbits = std::numeric_limits<float>::digits - 1;
	value<nrfbits> v(x);
	// ignore for the sake of clarity the special cases 0 and inf
	bool sign = v.sign();
	int scale = v.scale();
	std::bitset<nrfbits> bits = v.fraction();
	cout << v << " = " << components(v) << endl;

	float minpos = (float)minpos_value<nbits, es>();
	float maxpos = (float)maxpos_value<nbits, es>();


	const size_t pt_len = nbits + 3 + es;
	std::bitset<pt_len> pt_bits;
	std::bitset<pt_len> regime;
	std::bitset<pt_len> exponent;
	std::bitset<pt_len> fraction;
	std::bitset<pt_len> sticky_bit;

	cout << "Abs(x)   = " << (float)std::abs(x) << endl;
	float y = std::max<float>(minpos, std::min<float>(maxpos, (float)std::abs(x)));
	cout << "y        = " << y << endl;
	bool r = (y >= 1.0f);
	cout << "r        = " << (r ? "1" : "0") << endl;
	float e = std::floor(std::log2(y));
	cout << "e        = " << e << endl;
	float f = x / float(pow(2.0, scale)) - 1.0f;
	cout << "f        = " << f << endl;
	cout << "bits     = " << bits << endl;
	int run = (int)std::abs(std::floor(e / pow(2, es))) + r;
	cout << "run      = " << run << endl;
	// reg   = BitOr[BitShiftLeft[r * (2^run - 1), 1], BitXor[1, r]];
	regime.set(0, 1 ^ r);
	for (int i = 1; i <= run; i++) regime.set(i, r);
	cout << "reg      = " << LowerSegment(regime,run) << endl;
	unsigned esval = scale % (uint32_t(1) << es);
	cout << "esval    = " << esval << endl;
	exponent = convert_to_bitset<pt_len>(esval);
	unsigned nf = std::max<unsigned>(0, (nbits + 1) - (2 + run + es));
	cout << "nf       = " << nf << endl;
	unsigned len = 1 + std::max<unsigned>((nbits + 1), (2 + run + es));
	cout << "len      = " << len << endl;
	unsigned fv = (unsigned)std::floor((double)(f * (unsigned(1) << nf)));
	cout << "fv       = " << to_binary(int64_t(fv)) << endl;
	bool sb = ((f * (unsigned(1) << nf)) > fv);
	cout << "sb       = " << (sb ? "1" : "0") << endl;

	// construct the bigger posit
	// pt    = BitOr[BitShiftLeft[reg, es + nf + 1], BitShiftLeft[esval, nf + 1], BitShiftLeft[fv, 1], sb];
	regime   <<= es + nf + 1;
	exponent <<= nf + 1;
	fraction <<= 1;
	sticky_bit.set(0, sb);

	pt_bits |= regime;
	pt_bits |= exponent;
	pt_bits |= fraction;
	pt_bits |= sticky_bit;

	cout << "pt bits  = " << LowerSegment(pt_bits, 2+run+es) << endl;

	bool blast = pt_bits.test(len - nbits);
	bool bafter = pt_bits.test(len - nbits - 1);
	bool bsticky = Any(pt_bits, len - nbits - 1 - 1);
	cout << "blast    = " << blast << endl;
	cout << "bafter   = " << bafter << endl;
	cout << "bsticky  = " << bsticky << endl;

	bool rb = (blast & bafter) | (bafter & bsticky);
	cout << "rb       = " << rb << endl;
	std::bitset<pt_len> ptt = pt_bits;
	ptt >>= (len - nbits);
	cout << "ptt      = " << ptt << endl;
	if (rb) {
		increment_bitset(ptt);
	}
	cout << "final    = " << LowerSegment(ptt, nbits) << endl;
}

template<size_t nbits, size_t es>
void posit_component_conversion(float x) {
	int k = calculate_k<nbits, es>(scale);
	cout << "k        = " << k << endl;
	regime<nbits, es> _regime;
	unsigned nr_of_regime_bits = _regime.assign_regime_pattern(k);
	cout << "regime   = " << _regime << " rbits " << nr_of_regime_bits << endl;
	exponent<nbits, es> _exponent;
	//_exponent.assign_exponent_bits(scale, k, nr_of_regime_bits);
	_exponent.assign_exponent_bits(scale, k, 2);
	cout << "exponent = " << _exponent << endl;
	fraction<nbits + 1 + es> _fraction;
	_fraction.assign(nf, bits);
	cout << "ff       = " << _fraction << endl;
}

void basic_algorithm_for_conversion() {
	const size_t nbits = 5;
	const size_t es = 1;

	int64_t value;
	unsigned int msb;
	unsigned int maxpos_scale = maxpos_scale_f<nbits, es>();

	value = 8;
	msb = findMostSignificantBit(value) - 1;
	if (msb > maxpos_scale) {
		cerr << "msb = " << msb << " and maxpos_scale() = " << maxpos_scale << endl;
		cerr << "Can't represent " << value << " with posit<" << nbits << "," << es << ">: maxpos = " << (1 << maxpos_scale) << endl;
	}
	// we need to find the regime for this rhs
	// regime represents a scale factor of useed ^ k, where k ranges from [1-nbits, nbits-2]
	// regime @ k = 0 -> 1
	// regime @ k = 1 -> (1 << (1 << es) ^ 1 = 2
	// regime @ k = 2 -> (1 << (1 << es) ^ 2 = 4
	// regime @ k = 3 -> (1 << (1 << es) ^ 3 = 8
	// the left shift of the regime is simply k * 2^es
	// which means that the msb of the regime is simply k*2^es

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
	// We'll be using m to convert from posit to integer/float.
	// We'll be using k to convert from integer/float to posit

	// The first step to convert an integer to a posit is to find the base regime scale
	// The base is defined as the biggest k where useed^k < integer
	// => k*2^es < msb && (k+1)*2^es > msb
	// => k < msb/2^es && k > msb/2^es - 1
	// => k = (msb >> es)

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
	value = 33;
	cout << hex << "0x" << value << dec << setw(12) << value << endl;
	msb = findMostSignificantBit(value) - 1;
	cout << "MSB      = " << msb << endl;
	cout << "Regime   = " << base_regime(value, es) << endl;
	cout << "Exponent = " << base_exponent(msb, es) << endl;
	cout << "Fraction = 0x" << hex << base_fraction(value) << dec << endl;
}

void PositIntegerConversion() {
	cout << "Conversion examples: (notice the rounding errors)" << endl;
	posit<5, 1> p1;
	int64_t value;
	value =  1;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value =  2;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value =  3;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value =  4;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value =  5;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value =  7;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value =  8;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = 15;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	cout << endl;
}

bool TestPositInitialization() {
	// posit initialization and assignment
	bool bValid = false;
	cout << "Posit copy constructor, assignment, and test" << endl;
	posit<16, 1> p16_1_1;
	p16_1_1 = (1 << 16);
	posit<16, 1> p16_1_2(p16_1_1);
	if (p16_1_1 != p16_1_2) {
		cerr << "Copy constructor failed" << endl;
		cerr << "value: " << (1 << 16) << " posits " << p16_1_1 << " == " << p16_1_2 << endl;
	}
	else {
		cout << "PASS" << endl;
		bValid = true;
	}
	cout << endl;
	return bValid;
}

void PositFloatConversion()
// posit float conversion
{
	cout << "Posit float conversion" << endl;
	float value;
	posit<4, 0> p1;
	value = 0.0;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = 0.25;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = 0.5;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = 0.75;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = 1.0;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = 1.5;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = 2.0;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = 4.0;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = INFINITY;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	
	cout << endl;
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Conversion failed: ";

#if MANUAL_TESTING
	const size_t nbits = 8;
	const size_t es = 1;
	/*
	posit< 8,1> useed scale     2     minpos scale        -12     maxpos scale         12
	00000000      00000000 Sign :  1 Regime :   0 Exponent :     1 Fraction :        1 Value :                0
	00000001      00000001 Sign :  1 Regime :  -6 Exponent :     1 Fraction :        1 Value :   0.000244140625
	00000010      00000010 Sign :  1 Regime :  -5 Exponent :     1 Fraction :        1 Value :     0.0009765625
	00000011      00000011 Sign :  1 Regime :  -5 Exponent :     2 Fraction :        1 Value :      0.001953125
	*/
	posit<nbits, es> p;
	cout << spec_to_string(p) << endl;
	cout << components_to_string(p) << endl;
	p.set_raw_bits(1);	cout << components_to_string(p) << endl; 	float f1 = p.to_float();
	p.set_raw_bits(2);	cout << components_to_string(p) << endl;	float f2 = p.to_float();
	p.set_raw_bits(3);	cout << components_to_string(p) << endl;	float f3 = p.to_float();

	float f_mineps, f, f_pluseps;
	f         = std::sqrt(f1 * f2);
	f_mineps  = (float)(f - 0.000000001);
	f_pluseps = (float)(f + 0.000000001);
	value<23> v_mineps(f_mineps);
	value<23> v(f);
	value<23> v_pluseps(f_pluseps);
	cout << "geometric mean - eps: " << f_mineps  << " " << components(v_mineps) << endl;
	cout << "geometric mean      : " << f         << " " << components(v) << endl;
	cout << "geometric mean + eps: " << f_pluseps << " " << components(v_pluseps) << endl;
	/*
	geometric mean - eps: 0.000488280 (+,-12,11111111111111111011110)
	geometric mean      : 0.000488281 (+,-11,00000000000000000000000)
	geometric mean + eps: 0.000488282 (+,-11,00000000000000000010001)
	*/
	convert_to_posit<nbits, es>(f_mineps);
	convert_to_posit<nbits, es>(f);	
	convert_to_posit<nbits, es>(f_pluseps);

	posit<nbits, es> p1(f1), p2(f2), p3(f3);
	cout << components_to_string(p1) << endl;
	cout << components_to_string(p2) << endl;
	cout << components_to_string(p3) << endl;

#else
	ReportPositScales();

	GenerateLogicPatternsForDebug<5, 0>();
	GenerateLogicPatternsForDebug<5, 1>();
	GenerateLogicPatternsForDebug<5, 2>();

#if STRESS_TESTING
	
	PositIntegerConversion();
	PositFloatConversion();
	if (!TestPositInitialization()) {
		cerr << "initialization failed" << endl;
	}
	ConversionExamplesPositiveRegime();
	ConversionExamplesNegativeRegime();
#endif // STRESS_TESTING

#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}


