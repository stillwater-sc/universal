/*============================================================================

This C source file is part of the SoftPosit Posit Arithmetic Package
by S. H. Leong (Cerlane).

Copyright 2017, 2018 A*STAR.  All rights reserved.

This C source file was based on SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3d, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015, 2016, 2017 The Regents of the
University of California.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions, and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions, and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the University nor the names of its contributors may
be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS", AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#pragma warning ( disable : 4146 4805)
#define signP32UI( a ) ((bool) ((uint32_t) (a)>>31))
#define signregP32UI( a ) ((bool) (((uint32_t) (a)>>30) & 0x1))
#define packToP32UI(regime, expA, fracA) ( (uint32_t) regime + (uint32_t) expA + ((uint32_t)(fracA)) )

using posit32_t = uint32_t;

const uint_fast16_t softposit_approxRecipSqrt0[16] = {
	0xb4c9, 0xffab, 0xaa7d, 0xf11c, 0xa1c5, 0xe4c7, 0x9a43, 0xda29,
	0x93b5, 0xd0e5, 0x8ded, 0xc8b7, 0x88c6, 0xc16d, 0x8424, 0xbae1
};
const uint_fast16_t softposit_approxRecipSqrt1[16] = {
	0xa5a5, 0xea42, 0x8c21, 0xc62d, 0x788f, 0xaa7f, 0x6928, 0x94b6,
	0x5cc7, 0x8335, 0x52a6, 0x74e2, 0x4a3e, 0x68fe, 0x432b, 0x5efd
};

posit32_t softposit_addMagsP32(uint_fast32_t, uint_fast32_t);
posit32_t softposit_subMagsP32(uint_fast32_t, uint_fast32_t);
posit32_t softposit_mulAddP32(uint_fast32_t, uint_fast32_t, uint_fast32_t, uint_fast32_t);

// add reference from SoftPosit
posit32_t softposit_addMagsP32(uint_fast32_t uiA, uint_fast32_t uiB) {
	uint_fast16_t regA;
	uint_fast64_t frac64A = 0, frac64B = 0;
	uint_fast32_t fracA = 0, regime, tmp;
	bool sign, regSA, regSB, rcarry = 0, bitNPlusOne = 0, bitsMore = 0;
	int_fast8_t kA = 0;
	int_fast32_t expA;
	int_fast16_t shiftRight;
	posit32_t uZ;

	sign = signP32UI(uiA);
	if (sign) {
		uiA = -uiA & 0xFFFFFFFF;
		uiB = -uiB & 0xFFFFFFFF;
	}

	if ((int_fast32_t)uiA < (int_fast32_t)uiB) {
		uiA ^= uiB;
		uiB ^= uiA;
		uiA ^= uiB;
	}
	regSA = signregP32UI(uiA);
	regSB = signregP32UI(uiB);

	tmp = (uiA << 2) & 0xFFFFFFFF;
	if (regSA) {
		while (tmp >> 31) {
			kA++;
			tmp = (tmp << 1) & 0xFFFFFFFF;
		}
	}
	else {
		kA = -1;
		while (!(tmp >> 31)) {
			kA--;
			tmp = (tmp << 1) & 0xFFFFFFFF;
		}
		tmp &= 0x7FFFFFFF;
	}

	expA = tmp >> 29; //to get 2 bits
	frac64A = ((0x40000000ULL | tmp << 1) & 0x7FFFFFFFULL) << 32;
	shiftRight = kA;

	tmp = (uiB << 2) & 0xFFFFFFFF;
	if (regSB) {
		while (tmp >> 31) {
			shiftRight--;
			tmp = (tmp << 1) & 0xFFFFFFFF;
		}
	}
	else {
		shiftRight++;
		while (!(tmp >> 31)) {
			shiftRight++;
			tmp = (tmp << 1) & 0xFFFFFFFF;
		}
		tmp &= 0x7FFFFFFF;
	}
	frac64B = ((0x40000000ULL | tmp << 1) & 0x7FFFFFFFULL) << 32;
	//This is 4kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)
	shiftRight = (shiftRight << 2) + expA - (tmp >> 29);

	//Manage CLANG (LLVM) compiler when shifting right more than number of bits
	(shiftRight>63) ? (frac64B = 0) : (frac64B >>= shiftRight); //frac64B >>= shiftRight

	frac64A += frac64B;

	rcarry = 0x8000000000000000 & frac64A; //first left bit
	if (rcarry) {
		expA++;
		if (expA>3) {
			kA++;
			expA &= 0x3;
		}
		frac64A >>= 1;
	}
	if (kA<0) {
		regA = -kA;
		regSA = 0;
		regime = 0x40000000 >> regA;
	}
	else {
		regA = kA + 1;
		regSA = 1;
		regime = 0x7FFFFFFF - (0x7FFFFFFF >> regA);
	}

	if (regA>30) {
		//max or min pos. exp and frac does not matter.
		(regSA) ? (uZ = 0x7FFFFFFF) : (uZ = 0x1);
	}
	else {
		//remove hidden bits
		frac64A = (frac64A & 0x3FFFFFFFFFFFFFFF) >> (regA + 2); // 2 bits exp

		fracA = frac64A >> 32;

		if (regA <= 28) {
			bitNPlusOne |= (0x80000000 & frac64A);
			expA <<= (28 - regA);
		}
		else {
			if (regA == 30) {
				bitNPlusOne = expA & 0x2;
				bitsMore = (expA & 0x1);
				expA = 0;
			}
			else if (regA == 29) {
				bitNPlusOne = expA & 0x1;
				expA >>= 1;
			}
			if (fracA>0) {
				fracA = 0;
				bitsMore = 1;
			}
		}

		uZ = packToP32UI(regime, expA, fracA);
		//n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
		if (bitNPlusOne) {
			(0x7FFFFFFF & frac64A) ? (bitsMore = 1) : (bitsMore = 0);
			uZ += (uZ & 1) | bitsMore;
		}
	}
	if (sign) uZ = -uZ & 0xFFFFFFFF;
	return uZ;
}

posit32_t softposit_subMagsP32(uint_fast32_t uiA, uint_fast32_t uiB) {
	uint_fast16_t regA;
	uint_fast64_t frac64A = 0, frac64B = 0;
	uint_fast32_t fracA = 0, regime, tmp;
	bool sign, regSA, regSB, ecarry = 0, bitNPlusOne = 0, bitsMore = 0;
	int_fast8_t kA = 0;
	int_fast32_t expA = 0;
	int_fast16_t shiftRight;
	posit32_t uZ;

	sign = signP32UI(uiA);
	if (sign)
		uiA = -uiA & 0xFFFFFFFF;
	else
		uiB = -uiB & 0xFFFFFFFF;

	if (uiA == uiB) { //essential, if not need special handling
		uZ = 0;
		return uZ;
	}
	if ((int_fast32_t)uiA < (int_fast32_t)uiB) {
		uiA ^= uiB;
		uiB ^= uiA;
		uiA ^= uiB;
		(sign) ? (sign = 0) : (sign = 1); //A becomes B
	}
	regSA = signregP32UI(uiA);
	regSB = signregP32UI(uiB);

	tmp = (uiA << 2) & 0xFFFFFFFF;
	if (regSA) {
		while (tmp >> 31) {
			kA++;
			tmp = (tmp << 1) & 0xFFFFFFFF;
		}
	}
	else {
		kA = -1;
		while (!(tmp >> 31)) {
			kA--;
			tmp = (tmp << 1) & 0xFFFFFFFF;
		}
		tmp &= 0x7FFFFFFF;
	}

	expA = tmp >> 29; //to get 2 bits
	frac64A = ((0x40000000ULL | tmp << 1) & 0x7FFFFFFFULL) << 32;
	shiftRight = kA;


	tmp = (uiB << 2) & 0xFFFFFFFF;
	if (regSB) {
		while (tmp >> 31) {
			shiftRight--;
			tmp = (tmp << 1) & 0xFFFFFFFF;
		}

	}
	else {
		shiftRight++;
		while (!(tmp >> 31)) {
			shiftRight++;
			tmp = (tmp << 1) & 0xFFFFFFFF;
		}
		tmp &= 0x7FFFFFFF;

	}
	frac64B = ((0x40000000ULL | tmp << 1) & 0x7FFFFFFFULL) << 32;

	//This is 4kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)
	shiftRight = (shiftRight << 2) + expA - (tmp >> 29);
	if (shiftRight>63) {
		uZ = uiA;
		if (sign) uZ = -uZ & 0xFFFFFFFF;
		return uZ;
	}
	else
		(frac64B >>= shiftRight);

	frac64A -= frac64B;

	while ((frac64A >> 59) == 0) {
		kA--;
		frac64A <<= 4;
	}
	ecarry = (0x4000000000000000 & frac64A);//(0x4000000000000000 & frac64A)>>62;
	while (!ecarry) {
		if (expA == 0) {
			kA--;
			expA = 3;
		}
		else
			expA--;
		frac64A <<= 1;
		ecarry = (0x4000000000000000 & frac64A);
	}

	if (kA<0) {
		regA = -kA;
		regSA = 0;
		regime = 0x40000000 >> regA;
	}
	else {
		regA = kA + 1;
		regSA = 1;
		regime = 0x7FFFFFFF - (0x7FFFFFFF >> regA);
	}
	if (regA>30) {
		//max or min pos. exp and frac does not matter.
		(regSA) ? (uZ = 0x7FFFFFFF) : (uZ = 0x1);
	}
	else {
		//remove hidden bits
		frac64A = (frac64A & 0x3FFFFFFFFFFFFFFF) >> (regA + 2); // 2 bits exp

		fracA = frac64A >> 32;

		if (regA <= 28) {
			bitNPlusOne |= (0x80000000 & frac64A);
			expA <<= (28 - regA);
		}
		else {
			if (regA == 30) {
				bitNPlusOne = expA & 0x2;
				bitsMore = (expA & 0x1);
				expA = 0;
			}
			else if (regA == 29) {
				bitNPlusOne = expA & 0x1;
				expA >>= 1;
			}
			if (fracA>0) {
				fracA = 0;
				bitsMore = 1;
			}

		}

		uZ = packToP32UI(regime, expA, fracA);
		//n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
		if (bitNPlusOne) {
			(0x7FFFFFFF & frac64A) ? (bitsMore = 1) : (bitsMore = 0);
			uZ += (uZ & 1) | bitsMore;
		}
	}
	if (sign) uZ = -uZ & 0xFFFFFFFF;
	return uZ;
}

posit32_t p32_add(posit32_t a, posit32_t b) {
	posit32_t uZ;
	uint_fast32_t uiA, uiB;

	uiA = a;
	uiB = b;

	//Zero or infinity
	if (uiA == 0 || uiB == 0) { // Not required but put here for speed
		uZ = uiA | uiB;
		return uZ;
	}
	else if (uiA == 0x80000000 || uiB == 0x80000000) {
		//printf("in infinity\n");
		uZ = 0x80000000;
		return uZ;
	}

	//different signs
	if ((uiA^uiB) >> 31)
		return softposit_subMagsP32(uiA, uiB);
	else
		return softposit_addMagsP32(uiA, uiB);

}

posit32_t p32_sub(posit32_t a, posit32_t b) {
	posit32_t uZ;
	uint_fast32_t uiA, uiB;

	uiA = a;
	uiB = b;

	//infinity
	if (uiA == 0x80000000 || uiB == 0x80000000) {
		uZ = 0x80000000;
		return uZ;
	}
	//Zero
	else if (uiA == 0 || uiB == 0) {
		uZ = (uiA | -uiB);
		return uZ;
	}

	//different signs
	if ((uiA^uiB) >> 31)
		return softposit_addMagsP32(uiA, (-uiB & 0xFFFFFFFF));
	else
		return softposit_subMagsP32(uiA, (-uiB & 0xFFFFFFFF));
}

posit32_t p32_mul(posit32_t pA, posit32_t pB) {
	posit32_t uZ;
	uint_fast32_t uiA, uiB;
	uint_fast32_t regA, fracA, regime, tmp;
	bool signA, signB, signZ, regSA, regSB, bitNPlusOne = 0, bitsMore = 0, rcarry;
	int_fast32_t expA;
	int_fast8_t kA = 0;
	uint_fast64_t frac64Z;

	uiA = pA;
	uiB = pB;

	//NaR or Zero
	if (uiA == 0x80000000 || uiB == 0x80000000) {
		uZ = 0x80000000;
		return uZ;
	}
	else if (uiA == 0 || uiB == 0) {
		uZ = 0;
		return uZ;
	}

	signA = signP32UI(uiA);
	signB = signP32UI(uiB);
	signZ = signA ^ signB;

	if (signA) uiA = (-uiA & 0xFFFFFFFF);
	if (signB) uiB = (-uiB & 0xFFFFFFFF);

	regSA = signregP32UI(uiA);
	regSB = signregP32UI(uiB);


	tmp = (uiA << 2) & 0xFFFFFFFF;
	if (regSA) {

		while (tmp >> 31) {

			kA++;
			tmp = (tmp << 1) & 0xFFFFFFFF;
		}
	}
	else {
		kA = -1;
		while (!(tmp >> 31)) {
			kA--;
			tmp = (tmp << 1) & 0xFFFFFFFF;
		}
		tmp &= 0x7FFFFFFF;
	}
	expA = tmp >> 29; //to get 2 bits
	fracA = ((tmp << 1) | 0x40000000) & 0x7FFFFFFF;

	tmp = (uiB << 2) & 0xFFFFFFFF;
	if (regSB) {
		while (tmp >> 31) {
			kA++;
			tmp = (tmp << 1) & 0xFFFFFFFF;
		}
	}
	else {
		kA--;
		while (!(tmp >> 31)) {
			kA--;
			tmp = (tmp << 1) & 0xFFFFFFFF;
		}
		tmp &= 0x7FFFFFFF;
	}
	expA += tmp >> 29;
	frac64Z = (uint_fast64_t)fracA * (((tmp << 1) | 0x40000000) & 0x7FFFFFFF);

	if (expA>3) {
		kA++;
		expA &= 0x3; // -=4
	}

	rcarry = frac64Z >> 61;//3rd bit of frac64Z
	if (rcarry) {
		expA++;
		if (expA>3) {
			kA++;
			expA &= 0x3;
		}
		frac64Z >>= 1;
	}

	if (kA<0) {
		regA = -kA;
		regSA = 0;
		regime = 0x40000000 >> regA;
	}
	else {
		regA = kA + 1;
		regSA = 1;
		regime = 0x7FFFFFFF - (0x7FFFFFFF >> regA);
	}



	if (regA>30) {
		//max or min pos. exp and frac does not matter.
		(regSA) ? (uZ = 0x7FFFFFFF) : (uZ = 0x1);
	}
	else {
		//remove carry and rcarry bits and shift to correct position (2 bits exp, so + 1 than 16 bits)
		frac64Z = (frac64Z & 0xFFFFFFFFFFFFFFF) >> regA;
		fracA = (uint_fast32_t)(frac64Z >> 32);

		if (regA <= 28) {
			bitNPlusOne |= (0x80000000 & frac64Z);
			expA <<= (28 - regA);
		}
		else {
			if (regA == 30) {
				bitNPlusOne = expA & 0x2;
				bitsMore = (expA & 0x1);
				expA = 0;
			}
			else if (regA == 29) {
				bitNPlusOne = expA & 0x1;
				expA >>= 1; //taken care of by the pack algo
			}
			if (fracA>0) {
				fracA = 0;
				bitsMore = 1;
			}

		}
		//sign is always zero
		uZ = packToP32UI(regime, expA, fracA);
		//n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
		if (bitNPlusOne) {
			(0x7FFFFFFF & frac64Z) ? (bitsMore = 1) : (bitsMore = 0);
			uZ += (uZ & 1) | bitsMore;
		}
	}

	if (signZ) uZ = -uZ & 0xFFFFFFFF;
	return uZ;

}

posit32_t p32_div(posit32_t pA, posit32_t pB) {
	posit32_t uZ;
	uint_fast32_t uiA, uiB, fracA, fracB, regA, regime, tmp;
	bool signA, signB, signZ, regSA, regSB, bitNPlusOne = 0, bitsMore = 0, rcarry;
	int_fast8_t kA = 0;
	int_fast32_t expA;
	uint_fast64_t frac64A, frac64Z, rem;
	lldiv_t divresult;

	uiA = pA;
	uiB = pB;

	//Zero or infinity
	if (uiA == 0x80000000 || uiB == 0x80000000 || uiB == 0) {
		uZ = 0x80000000;
		return uZ;
	}
	else if (uiA == 0) {
		uZ = 0;
		return uZ;
	}

	signA = signP32UI(uiA);
	signB = signP32UI(uiB);
	signZ = signA ^ signB;
	if (signA) uiA = (-uiA & 0xFFFFFFFF);
	if (signB) uiB = (-uiB & 0xFFFFFFFF);
	regSA = signregP32UI(uiA);
	regSB = signregP32UI(uiB);

	tmp = (uiA << 2) & 0xFFFFFFFF;
	if (regSA) {
		while (tmp >> 31) {
			kA++;
			tmp = (tmp << 1) & 0xFFFFFFFF;
		}
	}
	else {
		kA = -1;
		while (!(tmp >> 31)) {
			kA--;
			tmp = (tmp << 1) & 0xFFFFFFFF;
		}
		tmp &= 0x7FFFFFFF;
	}
	expA = tmp >> 29; //to get 2 bits
	fracA = ((tmp << 1) | 0x40000000) & 0x7FFFFFFF;
	frac64A = fracA << 30;

	tmp = (uiB << 2) & 0xFFFFFFFF;
	if (regSB) {
		while (tmp >> 31) {
			kA--;
			tmp = (tmp << 1) & 0xFFFFFFFF;
		}
	}
	else {
		kA++;
		while (!(tmp >> 31)) {
			kA++;
			tmp = (tmp << 1) & 0xFFFFFFFF;
		}
		tmp &= 0x7FFFFFFF;
	}
	expA -= tmp >> 29;
	fracB = ((tmp << 1) | 0x40000000) & 0x7FFFFFFF;

	divresult = lldiv(frac64A, (uint_fast64_t)fracB);
	frac64Z = divresult.quot;
	rem = divresult.rem;

	if (expA<0) {
		expA += 4;
		kA--;
	}
	if (frac64Z != 0) {
		rcarry = frac64Z >> 30; // this is the hidden bit (14th bit) , extreme right bit is bit 0
		if (!rcarry) {
			if (expA == 0) {
				kA--;
				expA = 3;
			}
			else
				expA--;
			frac64Z <<= 1;
		}
	}

	if (kA<0) {
		regA = -kA;
		regSA = 0;
		regime = 0x40000000 >> regA;
	}
	else {
		regA = kA + 1;
		regSA = 1;
		regime = 0x7FFFFFFF - (0x7FFFFFFF >> regA);
	}
	if (regA>30) {
		//max or min pos. exp and frac does not matter.
		(regSA) ? (uZ = 0x7FFFFFFF) : (uZ = 0x1);
	}
	else {
		//remove carry and rcarry bits and shift to correct position
		frac64Z &= 0x3FFFFFFF;

		fracA = (uint_fast32_t)frac64Z >> (regA + 2);

		if (regA <= 28) {
			bitNPlusOne = (frac64Z >> (regA + 1)) & 0x1;
			expA <<= (28 - regA);
			if (bitNPlusOne) (((1 << (regA + 1)) - 1) & frac64Z) ? (bitsMore = 1) : (bitsMore = 0);
		}
		else {
			if (regA == 30) {
				bitNPlusOne = expA & 0x2;
				bitsMore = (expA & 0x1);
				expA = 0;
			}
			else if (regA == 29) {
				bitNPlusOne = expA & 0x1;
				expA >>= 1; //taken care of by the pack algo
			}
			if (frac64Z>0) {
				fracA = 0;
				bitsMore = 1;
			}

		}
		if (rem) bitsMore = 1;

		uZ = packToP32UI(regime, expA, fracA);
		if (bitNPlusOne) uZ += (uZ & 1) | bitsMore;
	}

	if (signZ) uZ = -uZ & 0xFFFFFFFF;
	return uZ;
}

posit32_t p32_sqrt(posit32_t pA) {

	posit32_t uA;
	uint_fast32_t index, r0, shift, fracA, expZ, expA;
	uint_fast32_t mask, uiA, uiZ;
	uint_fast64_t eSqrR0, fracZ, negRem, recipSqrt, shiftedFracZ, sigma0, sqrSigma0;
	int_fast32_t eps, shiftZ;

	uiA = pA;

	// If NaR or a negative number, return NaR.
	if (uiA & 0x80000000) {
		uA = 0x80000000;
		return uA;
	}
	// If the argument is zero, return zero.
	else if (!uiA) {
		return pA;
	}
	// Compute the square root; shiftZ is the power-of-2 scaling of the result.
	// Decode regime and exponent; scale the input to be in the range 1 to 4:
	if (uiA & 0x40000000) {
		shiftZ = -2;
		while (uiA & 0x40000000) {
			shiftZ += 2;
			uiA = (uiA << 1) & 0xFFFFFFFF;
		}
	}
	else {
		shiftZ = 0;
		while (!(uiA & 0x40000000)) {
			shiftZ -= 2;
			uiA = (uiA << 1) & 0xFFFFFFFF;
		}
	}

	uiA &= 0x3FFFFFFF;
	expA = (uiA >> 28);
	shiftZ += (expA >> 1);
	expA = (0x1 ^ (expA & 0x1));
	uiA &= 0x0FFFFFFF;
	fracA = (uiA | 0x10000000);

	// Use table look-up of first 4 bits for piecewise linear approx. of 1/sqrt:
	index = ((fracA >> 24) & 0xE) + expA;
	eps = ((fracA >> 9) & 0xFFFF);
	r0 = softposit_approxRecipSqrt0[index]
		- (((uint_fast32_t)softposit_approxRecipSqrt1[index] * eps) >> 20);

	// Use Newton-Raphson refinement to get 33 bits of accuracy for 1/sqrt:
	eSqrR0 = (uint_fast64_t)r0 * r0;
	if (!expA) eSqrR0 <<= 1;
	sigma0 = 0xFFFFFFFF & (0xFFFFFFFF ^ ((eSqrR0 * (uint64_t)fracA) >> 20));
	recipSqrt = ((uint_fast64_t)r0 << 20) + (((uint_fast64_t)r0 * sigma0) >> 21);

	sqrSigma0 = ((sigma0 * sigma0) >> 35);
	recipSqrt += (((recipSqrt + (recipSqrt >> 2) - ((uint_fast64_t)r0 << 19)) * sqrSigma0) >> 46);


	fracZ = (((uint_fast64_t)fracA) * recipSqrt) >> 31;
	if (expA) fracZ = (fracZ >> 1);

	// Find the exponent of Z and encode the regime bits.
	expZ = shiftZ & 0x3;
	if (shiftZ < 0) {
		shift = (-1 - shiftZ) >> 2;
		uiZ = 0x20000000 >> shift;
	}
	else {
		shift = shiftZ >> 2;
		uiZ = 0x7FFFFFFF - (0x3FFFFFFF >> shift);
	}

	// Trick for eliminating off-by-one cases that only uses one multiply:
	fracZ++;
	if (!(fracZ & 0xF)) {
		shiftedFracZ = fracZ >> 1;
		negRem = (shiftedFracZ * shiftedFracZ) & 0x1FFFFFFFF;
		if (negRem & 0x100000000) {
			fracZ |= 1;
		}
		else {
			if (negRem) fracZ--;
		}
	}
	// Strip off the hidden bit and round-to-nearest using last shift+5 bits.
	fracZ &= 0xFFFFFFFF;
	mask = (1 << (4 + shift));
	if (mask & fracZ) {
		if (((mask - 1) & fracZ) | ((mask << 1) & fracZ)) fracZ += (mask << 1);
	}
	// Assemble the result and return it.
	uA = uiZ | (expZ << (27 - shift)) | (fracZ >> (5 + shift));
	return uA;
}
