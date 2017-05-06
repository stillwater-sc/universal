#include "stdafx.h"


using namespace std;

#ifdef __GNUC__
#define CLZ(n) \
    __builtin_clz(n)
#endif

#ifdef _MSC_VER
#include <intrin.h>

#define CLZ(n) clz(n)

uint32_t __inline ctz(uint32_t value)
{
	unsigned long trailing_zero = 0;

	if (_BitScanForward(&trailing_zero, value))
	{
		return trailing_zero;
	} else {
		// This is undefined, I better choose 32 than 0
		return 32;
	}
}

uint32_t __inline clz(uint32_t value)
{
	unsigned long leading_zero = 0;

	if (_BitScanReverse(&leading_zero, value))
	{
		return 31 - leading_zero;
	} else {
		// Same remarks as above
		return 32;
	}
}
#endif

#define POW2(n) \
    (1 << (n))

#define MIN(a, b) \
    ((a) < (b) ? (a) : (b))

#define MAX(a, b) \
    ((a) > (b) ? (a) : (b))

#define LMASK(bits, size) \
    ((bits) & (POSIT_MASK << (POSIT_SIZE - (size))))

POSIT_UTYPE Posit::buildBits(bool neg, int reg, POSIT_UTYPE exp,
	POSIT_UTYPE frac)
{
	POSIT_UTYPE bits;
	POSIT_UTYPE regBits;
	POSIT_UTYPE expBits;
	int rs = MAX(-reg + 1, reg + 2);

	if (reg < 0) {
		regBits = POSIT_MSB >> -reg;
	}
	else {
		regBits = LMASK(POSIT_MASK, reg + 1);
	}
	expBits = LMASK(exp << (POSIT_SIZE - mEs), mEs);

	bits = frac;
	bits = expBits | (bits >> mEs);
	bits = regBits | (bits >> rs);
	bits = bits >> ss();

	if (neg) {
		bits = (bits ^ POSIT_MASK) + 1;
	}

	return LMASK(bits, mNbits);
}

void Posit::fromIeee(uint64_t fbits, int fes, int ffs)
{
	int fexpbias = POW2(fes - 1) - 1;
	int16_t fexp = (fbits >> ffs) & ((1 << fes) - 1);
	uint64_t ffrac = fbits & ((1ULL << ffs) - 1);

	// clip exponent
	int rminfexp = POW2(mEs) * (-mNbits + 2);
	int rmaxfexp = POW2(mEs) * (mNbits - 2);
	int rfexp = MIN(MAX(fexp - fexpbias, rminfexp), rmaxfexp);

	bool rsign = fbits >> (fes + ffs);
	int rreg = rfexp >> mEs; // floor(rfexp / 2^mEs)
	int rexp = rfexp - POW2(mEs) * rreg;
	POSIT_UTYPE rfrac;

	if (ffs <= POSIT_SIZE) {
		rfrac = ffrac << (POSIT_SIZE - ffs);
	}
	else {
		rfrac = ffrac >> (ffs - POSIT_SIZE);
	}

	mBits = buildBits(rsign, rreg, rexp, rfrac);
}

uint64_t Posit::toIeee(int fes, int ffs)
{
	uint64_t fbits;
	int exp;
	POSIT_UTYPE frac;

	if (isNeg()) {
		Posit p = neg();
		exp = POW2(p.mEs) * p.regime() + p.exponent();
		frac = p.lfraction();
	}
	else {
		exp = POW2(mEs) * regime() + exponent();
		frac = lfraction();
	}

	int rexpbias = POW2(fes - 1) - 1;
	int rexp = MIN(MAX(exp + rexpbias, 1), POW2(fes) - 2);
	uint64_t rfrac;

	if (exp + rexpbias < rexp) {
		// TODO: support subnormals (exponent 0)
		// underflow, set minimal fraction
		rfrac = 0;
	}
	else if (exp + rexpbias > rexp) {
		// overflow, set maximal fraction
		rfrac = 0-1ULL;
	}
	else {
		if (POSIT_SIZE <= ffs) {
			rfrac = (uint64_t)frac << (ffs - POSIT_SIZE);
		}
		else {
			rfrac = (uint64_t)frac >> (POSIT_SIZE - ffs);
		}
	}

	fbits = isNeg();
	fbits = rexp | (fbits << fes);
	fbits = rfrac | (fbits << ffs);

	return fbits;
}

Posit::Posit(POSIT_UTYPE bits, int nbits, int es, bool nan) :
	mBits(bits),
	mNbits(nbits),
	mEs(es),
	mNan(nan)
{
}

Posit::Posit(int nbits, int es, bool nan) :
	Posit(0, nbits, es, nan)
{
}

Posit::Posit(int nbits, int es) :
	Posit(nbits, es, false)
{
}

bool Posit::isZero()
{
	return mBits == POSIT_ZERO;
}

bool Posit::isOne()
{
	return mBits == POSIT_ONE || mBits == POSIT_MONE;
}

bool Posit::isInf()
{
	return mBits == POSIT_INF;
}

bool Posit::isNeg()
{
	return (POSIT_STYPE)mBits < 0 && mBits != POSIT_INF;
}

bool Posit::isNan()
{
	return mNan;
}

int Posit::nbits()
{
	return mNbits;
}

int Posit::ss()
{
	return 1;
}

int Posit::rs()
{
	int lz = CLZ(mBits << ss());
	int lo = CLZ(~mBits << ss());
	int rs = MAX(lz, lo) + 1;

	return MIN(rs, mNbits - ss());
}

int Posit::es()
{
	return MIN(MAX(mNbits - ss() - rs(), 0), mEs);
}

int Posit::fs()
{
	return MAX(mNbits - ss() - rs() - mEs, 0);
}

int Posit::useed()
{
	return POW2(POW2(mEs));
}

int Posit::regime()
{
	POSIT_UTYPE bits = isNeg() ? neg().mBits : mBits;
	int lz = CLZ(bits << ss());
	int lo = CLZ(~bits << ss());

	return (lz == 0 ? lo - 1 : -lz);
}

POSIT_UTYPE Posit::exponent()
{
	POSIT_UTYPE lExpBits = mBits << (ss() + rs());

	return lExpBits >> (POSIT_SIZE - mEs);
}

POSIT_UTYPE Posit::lfraction()
{
	return mBits << (ss() + rs() + mEs);
}

Posit Posit::zero()
{
	return Posit(POSIT_ZERO, mNbits, mEs, false);
}

Posit Posit::one()
{
	return Posit(POSIT_ONE, mNbits, mEs, false);
}

Posit Posit::inf()
{
	return Posit(POSIT_INF, mNbits, mEs, false);
}

Posit Posit::nan()
{
	return Posit(mNbits, mEs, true);
}

Posit Posit::neg()
{
	// reverse all bits and add one
	POSIT_UTYPE bits = LMASK(0-mBits, mNbits);

	return Posit(bits, mNbits, mEs, false);
}

Posit Posit::rec()
{
	// reverse all bits but the first one and add one
	POSIT_UTYPE bits = LMASK((mBits ^ (POSIT_MASK >> ss())) + 1, mNbits);

	return Posit(bits, mNbits, mEs, false);
}

Posit Posit::add(Posit& p)
{
	// fast exit
	if (isZero()) {
		return p;
	}
	else if (p.isZero()) {
		return *this;
	}
	else if (isInf() && p.isInf()) {
		return nan();
	}
	else if (isInf() || p.isInf()) {
		return inf();
	}
	else if (neg().eq(p)) {
		return zero();
	}

	// TODO implement
	return *this;
}

Posit Posit::sub(Posit& p)
{
	// no loss on negation
	Posit np = p.neg();

	return add(np);
}

Posit Posit::mul(Posit& p)
{
	// fast exit
	if (isZero()) {
		return (p.isInf() ? nan() : zero());
	}
	else if (p.isZero()) {
		return (isInf() ? nan() : zero());
	}
	else if (isOne()) {
		return (isNeg() ? p.neg() : p);
	}
	else if (p.isOne()) {
		return (p.isNeg() ? neg() : *this);
	}
	else if (isInf() || p.isInf()) {
		return inf();
	}
	else if (rec().eq(p)) {
		return one();
	}
	else if (rec().neg().eq(p)) {
		return one().neg();
	}

	int xfexp = POW2(mEs) * regime() + exponent();
	int pfexp = POW2(mEs) * p.regime() + p.exponent();

	// fractions have a hidden bit
	POSIT_UTYPE xfrac = POSIT_MSB | (lfraction() >> 1);
	POSIT_UTYPE pfrac = POSIT_MSB | (p.lfraction() >> 1);
	POSIT_UTYPE mfrac = ((POSIT_LUTYPE)xfrac *
		(POSIT_LUTYPE)pfrac) >> POSIT_SIZE;

	// shift is either 0 or 1
	int shift = CLZ(mfrac);

	// clip exponent to avoid underflow and overflow
	int rminfexp = POW2(mEs) * (-mNbits + 2);
	int rmaxfexp = POW2(mEs) * (mNbits - 2);
	int rfexp = MIN(MAX(xfexp + pfexp - shift + 1, rminfexp), rmaxfexp);

	bool rsign = isNeg() ^ p.isNeg();
	int rreg = rfexp >> mEs; // floor(rfexp / 2^mEs)
	int rexp = rfexp - POW2(mEs) * rreg;
	POSIT_UTYPE rfrac = mfrac << (shift + 1);

	return Posit(buildBits(rsign, rreg, rexp, rfrac), mNbits, mEs, false);
}

Posit Posit::div(Posit& p)
{
	// no loss on reciprocation!
	Posit rp = p.rec();

	return mul(rp);
}

bool Posit::eq(Posit& p)
{
	return mBits == p.mBits;
}

bool Posit::gt(Posit& p)
{
	if (isInf() || p.isInf()) {
		return false;
	}

	return mBits > p.mBits;
}

bool Posit::ge(Posit& p)
{
	return gt(p) || eq(p);
}

bool Posit::lt(Posit& p)
{
	return !gt(p) && !eq(p);
}

bool Posit::le(Posit& p)
{
	return !gt(p);
}

void Posit::set(float n)
{
	union {
		float f;
		uint32_t bits;
	};

	switch (fpclassify(n)) {
	case FP_INFINITE:
		mBits = POSIT_INF;
		mNan = false;
		break;
	case FP_NAN:
		mNan = true;
		break;
	case FP_ZERO:
		mBits = POSIT_ZERO;
		mNan = false;
		break;
	case FP_SUBNORMAL:
		// TODO: support subnormals
		mBits = POSIT_ZERO;
		mNan = false;
		break;
	case FP_NORMAL:
		f = n;
		fromIeee(bits, 8, 23);
		mNan = false;
		break;
	}
}

void Posit::set(double n)
{
	union {
		double f;
		uint64_t bits;
	};

	switch (fpclassify(n)) {
	case FP_INFINITE:
		mBits = POSIT_INF;
		mNan = false;
		break;
	case FP_NAN:
		mNan = true;
		break;
	case FP_ZERO:
		mBits = POSIT_ZERO;
		mNan = false;
		break;
	case FP_SUBNORMAL:
		// TODO: support subnormals
		mBits = POSIT_ZERO;
		mNan = false;
		break;
	case FP_NORMAL:
		f = n;
		fromIeee(bits, 11, 52);
		mNan = false;
		break;
	}
}

float Posit::getFloat()
{
	union {
		float f;
		uint32_t bits;
	};

	if (isZero()) {
		return 0.f;
	}
	else if (isInf()) {
		return .0; // 1.f / 0.f;
	}
	else if (isNan()) {
		return .0; //  0.f / 0.f;
	}

	bits = (uint32_t)toIeee(8, 23);

	return f;
}

double Posit::getDouble()
{
	union {
		double f;
		uint64_t bits;
	};

	if (isZero()) {
		return 0.0;
	}
	else if (isInf()) {
		return .0f; // 1.0 / 0.0;
	}
	else if (isNan()) {
		return .0f; //  0.0 / 0.0;
	}

	bits = toIeee(11, 52);

	return f;
}

void Posit::setBits(POSIT_UTYPE bits)
{
	mBits = bits << (POSIT_SIZE - mNbits);
}

POSIT_UTYPE Posit::getBits()
{
	return mBits >> (POSIT_SIZE - mNbits);
}

void Posit::print()
{
	Posit p = isNeg() || isInf() ? neg() : *this;

	printf("{%d, %d} ", mNbits, mEs);

	for (int i = POSIT_SIZE - 1; i >= POSIT_SIZE - mNbits; i--) {
		printf("%d", (mBits >> i) & 1);
	}

	printf(" (%d) -> ", regime());
	printf(isNeg() || isInf() ? "-" : "+");

	for (int i = POSIT_SIZE - ss() - 1; i >= POSIT_SIZE - mNbits; i--) {
		printf("%d", (p.mBits >> i) & 1);

		if (i != POSIT_SIZE - mNbits &&
			((i == POSIT_SIZE - ss() - p.rs()) ||
			(i == POSIT_SIZE - ss() - p.rs() - mEs))) {
			printf(" ");
		}
	}

	printf(" = %lg\n", getDouble());
}
