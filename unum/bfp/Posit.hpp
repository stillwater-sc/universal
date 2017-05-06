#pragma once

#define POSIT_LUTYPE    std::uint64_t
#define POSIT_UTYPE     std::uint32_t
#define POSIT_STYPE     std::int32_t
#define POSIT_SIZE      (8 * (int)sizeof(POSIT_UTYPE))
#define POSIT_ZERO      ((POSIT_UTYPE)0x00000000)
#define POSIT_ONE       ((POSIT_UTYPE)0x40000000)
#define POSIT_MONE      ((POSIT_UTYPE)0xC0000000)
#define POSIT_INF       ((POSIT_UTYPE)0x80000000)
#define POSIT_MSB       ((POSIT_UTYPE)0x80000000)
#define POSIT_MASK      ((POSIT_UTYPE)0xFFFFFFFF)

class Posit {
private:
	POSIT_UTYPE mBits;
	int mNbits;
	int mEs;
	bool mNan;

	POSIT_UTYPE buildBits(bool neg, int reg, POSIT_UTYPE exp, POSIT_UTYPE frac);

	void fromIeee(uint64_t fbits, int fes, int ffs);
	uint64_t toIeee(int fes, int ffs);

	Posit(POSIT_UTYPE bits, int nbits, int es, bool nan);
	Posit(int nbits, int es, bool nan);

public:
	Posit(int nbits, int es);

	bool isZero();                  // check for 0
	bool isOne();                   // check for +/- 1
	bool isInf();                   // check for +/- infinity
	bool isNeg();                   // check for negative
	bool isNan();                   // check for NaN

	int nbits();                    // size in bits
	int ss();                       // sign size in bits
	int rs();                       // regime size in bits
	int es();                       // exponent size in bits
	int fs();                       // fraction size in bits
	int useed();                    // value at 45°
	int regime();                   // regime rank
	POSIT_UTYPE exponent();         // exponent value
	POSIT_UTYPE lfraction();        // left-aligned fraction value

	Posit zero();                   // 0
	Posit one();                    // 1
	Posit inf();                    // +/-inf
	Posit nan();                    // NaN

	Posit neg();                    // -x
	Posit rec();                    // /x

	Posit add(Posit& p);            // x + p
	Posit sub(Posit& p);            // x - p as x + -p
	Posit mul(Posit& p);            // x * p
	Posit div(Posit& p);            // x / p as x * /p

	bool eq(Posit& p);              // x == p
	bool gt(Posit& p);              // x > p
	bool ge(Posit& p);              // x >= p
	bool lt(Posit& p);              // x < p
	bool le(Posit& p);              // x <= p

	void set(float n);              // x = n
	void set(double n);             // x = n

	float getFloat();               // n = x
	double getDouble();             // n = x

									// debug
	void setBits(POSIT_UTYPE bits);
	POSIT_UTYPE getBits();
	void print();
};

