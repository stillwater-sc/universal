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

// example: #pragma warning( disable : 4507 34; once : 4385; error : 164 )
#pragma warning( disable : 4146)

#define signP16UI( a ) ( (bool) ( ( uint16_t ) (a)>>15 ) )
#define signregP16UI( a ) ( (bool) (((uint16_t) (a)>>14) & 0x1) )
#define expP16UI( a, regA ) ((int_fast8_t) ((a)>>(13-regA) & 0x0001))
#define packToP16UI( regime, regA, expA, fracA) ((uint16_t) regime + ((uint16_t) (expA)<< (13-regA)) + ((uint16_t)(fracA)) )

using posit16_t = uint16_t;

posit16_t softposit_addMagsP16(uint_fast16_t, uint_fast16_t);
posit16_t softposit_subMagsP16(uint_fast16_t, uint_fast16_t);
posit16_t softposit_mulAddP16(uint_fast16_t, uint_fast16_t, uint_fast16_t, uint_fast16_t);

posit16_t i32_to_p16(int32_t a) {
	int_fast8_t k, log2 = 25;
	posit16_t uZ;
	uint_fast16_t uiA;
	uint_fast32_t expA, mask = 0x0200'0000, fracA;
	bool sign;

	sign = a >> 31;
	if (sign) {
		a = -a & 0xFFFFFFFF;
	}
	if (a == 0x80000000) {
		uZ = 0x8000;
		return uZ;
	}
	else if (a > 0x08000000) {
		uiA = 0x7FFF;
	}
	else if (a > 0x02FFFFFF) {
		uiA = 0x7FFE;
	}
	else if (a < 2) {
		uiA = (a << 14);
	}
	else {
		fracA = a;
		while (!(fracA & mask)) {
			log2--;
			fracA <<= 1;
		}
		k = log2 >> 1;
		expA = (log2 & 0x1) << (12 - k);
		fracA = (fracA ^ mask);

		uiA = (0x7FFF ^ (0x3FFF >> k)) | expA | (fracA >> (k + 13));
		mask = 0x1000 << k; //bitNPlusOne
		if (mask & fracA) {
			if (((mask - 1) & fracA) | ((mask << 1) & fracA)) uiA++;
		}
	}
	(sign) ? (uZ = -uiA & 0xFFFF) : (uZ = uiA);
	return uZ;
}

void checkExtraTwoBitsP16(double f16, double temp, bool * bitsNPlusOne, bool * bitsMore) {
	temp /= 2;
	if (temp <= f16) {
		*bitsNPlusOne = 1;
		f16 -= temp;
	}
	if (f16>0)
		*bitsMore = 1;
}

uint_fast16_t convertFractionP16(double f16, uint_fast8_t fracLength, bool * bitsNPlusOne, bool * bitsMore) {

	uint_fast16_t frac = 0;

	if (f16 == 0) return 0;
	else if (f16 == INFINITY) return 0x8000;

	f16 -= 1; //remove hidden bit
	if (fracLength == 0)
		checkExtraTwoBitsP16(f16, 1.0, bitsNPlusOne, bitsMore);
	else {
		double temp = 1;
		while (true) {
			temp /= 2;
			if (temp <= f16) {
				f16 -= temp;
				fracLength--;
				frac = (frac << 1) + 1; //shift in one
				if (f16 == 0) {
					//put in the rest of the bits
					frac <<= (uint_fast8_t)fracLength;
					break;
				}

				if (fracLength == 0) {
					checkExtraTwoBitsP16(f16, temp, bitsNPlusOne, bitsMore);

					break;
				}
			}
			else {
				frac <<= 1; //shift in a zero
				fracLength--;
				if (fracLength == 0) {
					checkExtraTwoBitsP16(f16, temp, bitsNPlusOne, bitsMore);
					break;
				}
			}
		}
	}

	return frac;
}

posit16_t convertFloatToP16(float a) {
	return convertDoubleToP16((double)a);
}

posit16_t convertDoubleToP16(double f16) {
	posit16_t uZ;
	bool sign, regS;
	uint_fast16_t reg, frac = 0;
	int_fast8_t exp = 0;
	bool bitNPlusOne = 0, bitsMore = 0;

	(f16 >= 0) ? (sign = 0) : (sign = 1);

	if (f16 == 0) {
		uZ = 0;
		return uZ;
	}
	else if (f16 == INFINITY || f16 == -INFINITY || f16 == NAN) {
		uZ = 0x8000;
		return uZ;
	}
	else if (f16 == 1) {
		uZ = 16384;
		return uZ;
	}
	else if (f16 == -1) {
		uZ = 49152;
		return uZ;
	}
	else if (f16 >= 268435456) {
		//maxpos
		uZ = 32767;
		return uZ;
	}
	else if (f16 <= -268435456) {
		// -maxpos
		uZ = 32769;
		return uZ;
	}
	else if (f16 <= 3.725290298461914e-9 && !sign) {
		//minpos
		uZ = 1;
		return uZ;
	}
	else if (f16 >= -3.725290298461914e-9 && sign) {
		//-minpos
		uZ = 65535;
		return uZ;
	}
	else if (f16>1 || f16<-1) {
		if (sign) {
			//Make negative numbers positive for easier computation
			f16 = -f16;
		}

		regS = 1;
		reg = 1; //because k = m-1; so need to add back 1
				 // minpos
		if (f16 <= 3.725290298461914e-9) {
			uZ = 1;
		}
		else {
			//regime
			while (f16 >= 4) {
				f16 *= 0.25;
				reg++;
			}
			if (f16 >= 2) {
				f16 *= 0.5;
				exp++;
			}

			int fracLength = 13 - reg;

			if (fracLength<0) {
				//reg == 14, means rounding bits is exp and just the rest.
				if (f16>1) 	bitsMore = 1;

			}
			else
				frac = convertFractionP16(f16, fracLength, &bitNPlusOne, &bitsMore);


			if (reg == 14 && frac>0) {
				bitsMore = 1;
				frac = 0;
			}
			if (reg>14)
				(regS) ? (uZ = 32767) : (uZ = 0x1);
			else {
				uint_fast16_t regime = 1;
				if (regS) regime = ((1 << reg) - 1) << 1;
				uZ = ((uint16_t)(regime) << (14 - reg)) + ((uint16_t)(exp) << (13 - reg)) + ((uint16_t)(frac));
				//n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
				if (reg == 14 && exp) bitNPlusOne = 1;
				uZ += (bitNPlusOne & (uZ & 1)) | (bitNPlusOne & bitsMore);
			}
			if (sign) uZ = -uZ & 0xFFFF;
		}
	}
	else if (f16 < 1 || f16 > -1) {

		if (sign) {
			//Make negative numbers positive for easier computation
			f16 = -f16;
		}
		regS = 0;
		reg = 0;

		//regime
		while (f16<1) {
			f16 *= 4;
			reg++;
		}
		if (f16 >= 2) {
			f16 /= 2;
			exp++;
		}
		if (reg == 14) {
			bitNPlusOne = exp;
			if (frac>1) bitsMore = 1;
		}
		else {
			//only possible combination for reg=15 to reach here is 7FFF (maxpos) and FFFF (-minpos)
			//but since it should be caught on top, so no need to handle
			int_fast8_t fracLength = 13 - reg;
			frac = convertFractionP16(f16, fracLength, &bitNPlusOne, &bitsMore);
		}

		if (reg == 14 && frac>0) {
			bitsMore = 1;
			frac = 0;
		}
		if (reg>14)
			(regS) ? (uZ.ui = 32767) : (uZ.ui = 0x1);
		else {
			uint_fast16_t regime = 1;
			if (regS) regime = ((1 << reg) - 1) << 1;
			uZ = ((uint16_t)(regime) << (14 - reg)) + ((uint16_t)(exp) << (13 - reg)) + ((uint16_t)(frac));
			//n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
			if (reg == 14 && exp) bitNPlusOne = 1;
			uZ += (bitNPlusOne & (uZ & 1)) | (bitNPlusOne & bitsMore);
		}
		if (sign) uZ = -uZ & 0xFFFF;
	}
	else {
		//NaR - for NaN, INF and all other combinations
		uZ = 0x8000;
	}
	return uZ;
}

posit16_t softposit_addMagsP16(uint_fast16_t uiA, uint_fast16_t uiB) {
	uint_fast16_t regA, uiX, uiY;
	uint_fast32_t frac32A, frac32B;
	uint_fast16_t fracA = 0, regime, tmp;
	bool sign, regSA, regSB, rcarry = 0, bitNPlusOne = 0, bitsMore = 0;
	int_fast8_t kA = 0, expA;
	int_fast16_t shiftRight;
	posit16_t uZ;

	sign = signP16UI(uiA); //sign is always positive.. actually don't have to do this.
	if (sign) {
		uiA = -uiA & 0xFFFF;
		uiB = -uiB & 0xFFFF;
	}

	if ((int_fast16_t)uiA < (int_fast16_t)uiB) {
		uiX = uiA;
		uiY = uiB;
		uiA = uiY;
		uiB = uiX;
	}
	regSA = signregP16UI(uiA);
	regSB = signregP16UI(uiB);

	tmp = (uiA << 2) & 0xFFFF;
	if (regSA) {
		while (tmp >> 15) {
			kA++;
			tmp = (tmp << 1) & 0xFFFF;
		}
	}
	else {
		kA = -1;
		while (!(tmp >> 15)) {
			kA--;
			tmp = (tmp << 1) & 0xFFFF;
		}
		tmp &= 0x7FFF;
	}
	expA = tmp >> 14;
	frac32A = (0x4000 | tmp) << 16;
	shiftRight = kA;

	tmp = (uiB << 2) & 0xFFFF;
	if (regSB) {
		while (tmp >> 15) {
			shiftRight--;
			tmp = (tmp << 1) & 0xFFFF;
		}
		frac32B = (0x4000 | tmp) << 16;
	}
	else {
		shiftRight++;
		while (!(tmp >> 15)) {
			shiftRight++;
			tmp = (tmp << 1) & 0xFFFF;
		}
		tmp &= 0x7FFF;
		frac32B = ((0x4000 | tmp) << 16) & 0x7FFFFFFF;
	}

	//This is 2kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)
	shiftRight = (shiftRight << 1) + expA - (tmp >> 14);

	if (shiftRight == 0) {
		frac32A += frac32B;
		//rcarry is one
		if (expA) kA++;
		expA ^= 1;
		frac32A >>= 1;
	}
	else {
		//Manage CLANG (LLVM) compiler when shifting right more than number of bits
		(shiftRight>31) ? (frac32B = 0) : (frac32B >>= shiftRight); //frac32B >>= shiftRight

		frac32A += frac32B;
		rcarry = 0x80000000 & frac32A; //first left bit
		if (rcarry) {
			if (expA) kA++;
			expA ^= 1;
			frac32A >>= 1;
		}
	}
	if (kA<0) {
		regA = (-kA & 0xFFFF);
		regSA = 0;
		regime = 0x4000 >> regA;
	}
	else {
		regA = kA + 1;
		regSA = 1;
		regime = 0x7FFF - (0x7FFF >> regA);
	}
	if (regA>14) {
		//max or min pos. exp and frac does not matter.
		(regSA) ? (uZ = 0x7FFF) : (uZ = 0x1);
	}
	else {
		//remove hidden bits
		frac32A = (frac32A & 0x3FFFFFFF) >> (regA + 1);
		fracA = frac32A >> 16;
		if (regA != 14) bitNPlusOne = (frac32A >> 15) & 0x1;
		else if (frac32A>0) {
			fracA = 0;
			bitsMore = 1;
		}
		if (regA == 14 && expA) bitNPlusOne = 1;
		uZ = packToP16UI(regime, regA, expA, fracA);
		if (bitNPlusOne) {
			(frac32A & 0x7FFF) ? (bitsMore = 1) : (bitsMore = 0);
			//n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
			uZ += (uZ & 1) | bitsMore;
		}
	}

	if (sign) uZ = -uZ & 0xFFFF;
	return uZ;
}

posit16_t softposit_subMagsP16(uint_fast16_t uiA, uint_fast16_t uiB) {
	uint_fast16_t regA;
	uint_fast32_t frac32A, frac32B;
	uint_fast16_t fracA = 0, regime, tmp;
	bool sign = 0, regSA, regSB, ecarry = 0, bitNPlusOne = 0, bitsMore = 0;
	int_fast16_t shiftRight;
	int_fast8_t kA = 0, expA;
	posit16_t uZ;

	//Both uiA and uiB are actually the same signs if uiB inherits sign of sub
	//Make both positive
	sign = signP16UI(uiA);
	(sign) ? (uiA = (-uiA & 0xFFFF)) : (uiB = (-uiB & 0xFFFF));

	if (uiA == uiB) { //essential, if not need special handling
		uZ = 0;
		return uZ;
	}
	if (uiA<uiB) {
		uiA ^= uiB;
		uiB ^= uiA;
		uiA ^= uiB;
		(sign) ? (sign = 0) : (sign = 1); //A becomes B
	}

	regSA = signregP16UI(uiA);
	regSB = signregP16UI(uiB);

	tmp = (uiA << 2) & 0xFFFF;
	if (regSA) {
		while (tmp >> 15) {
			kA++;
			tmp = (tmp << 1) & 0xFFFF;
		}
	}
	else {
		kA = -1;
		while (!(tmp >> 15)) {
			kA--;
			tmp = (tmp << 1) & 0xFFFF;
		}
		tmp &= 0x7FFF;
	}
	expA = tmp >> 14;
	frac32A = (0x4000 | tmp) << 16;
	shiftRight = kA;

	tmp = (uiB << 2) & 0xFFFF;
	if (regSB) {
		while (tmp >> 15) {
			shiftRight--;
			tmp = (tmp << 1) & 0xFFFF;
		}
	}
	else {
		shiftRight++;
		while (!(tmp >> 15)) {
			shiftRight++;
			tmp = (tmp << 1) & 0xFFFF;
		}
		tmp &= 0x7FFF;
	}
	frac32B = (0x4000 | tmp) << 16;
	//This is 2kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)

	shiftRight = (shiftRight << 1) + expA - (tmp >> 14);

	if (shiftRight != 0) {
		if (shiftRight >= 29) {
			uZ = uiA;
			if (sign) uZ = -uZ & 0xFFFF;
			return uZ;
		}
		else
			frac32B >>= shiftRight;
	}

	frac32A -= frac32B;

	while ((frac32A >> 29) == 0) {
		kA--;
		frac32A <<= 2;
	}
	ecarry = (0x40000000 & frac32A) >> 30;
	if (!ecarry) {
		if (expA == 0) kA--;
		expA ^= 1;
		frac32A <<= 1;
	}

	if (kA<0) {
		regA = (-kA & 0xFFFF);
		regSA = 0;
		regime = 0x4000 >> regA;
	}
	else {
		regA = kA + 1;
		regSA = 1;
		regime = 0x7FFF - (0x7FFF >> regA);
	}

	if (regA>14) {
		//max or min pos. exp and frac does not matter.
		(regSA) ? (uZ = 0x7FFF) : (uZ = 0x1);
	}
	else {
		//remove hidden bits
		frac32A = (frac32A & 0x3FFFFFFF) >> (regA + 1);
		fracA = frac32A >> 16;
		if (regA != 14) bitNPlusOne = (frac32A >> 15) & 0x1;
		else if (frac32A>0) {
			fracA = 0;
			bitsMore = 1;
		}
		if (regA == 14 && expA) bitNPlusOne = 1;
		uZ = packToP16UI(regime, regA, expA, fracA);
		if (bitNPlusOne) {
			(frac32A & 0x7FFF) ? (bitsMore = 1) : (bitsMore = 0);
			//n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
			uZ += (uZ & 1) | bitsMore;
		}
	}
	if (sign) uZ = -uZ & 0xFFFF;
	return uZ;

}

posit16_t p16_add(posit16_t a, posit16_t b) {
	posit16_t uZ;
	uint_fast16_t uiA, uiB;

	uiA = a;
	uiB = b;

	//Zero or infinity
	if (uiA == 0 || uiB == 0) { // Not required but put here for speed
		uZ = uiA | uiB;
		return uZ;
	}
	else if (uiA == 0x8000 || uiB == 0x8000) {
		uZ = 0x8000;
		return uZ;
	}

	//different signs
	if ((uiA^uiB) >> 15)
		return softposit_subMagsP16(uiA, uiB);
	else
		return softposit_addMagsP16(uiA, uiB);
}

posit16_t p16_sub(posit16_t a, posit16_t b) {

	posit16_t uZ;
	uint_fast16_t uiA, uiB;

	uiA = a;
	uiB = b;

	//infinity
	if (uiA == 0x8000 || uiB == 0x8000) {
		uZ = 0x8000;
		return uZ;
	}
	//Zero
	else if (uiA == 0 || uiB == 0) {
		uZ = (uiA | -uiB);
		return uZ;
	}

	//different signs
	if ((uiA^uiB) >> 15)
		return softposit_addMagsP16(uiA, (-uiB & 0xFFFF));
	else
		return softposit_subMagsP16(uiA, (-uiB & 0xFFFF));
}

posit16_t p16_mul(posit16_t pA, posit16_t pB) {

	posit16_t uZ;
	uint_fast16_t uiA, uiB;
	uint_fast16_t regA, fracA, regime, tmp;
	bool signA, signB, signZ, regSA, regSB, bitNPlusOne = 0, bitsMore = 0, rcarry;
	int_fast8_t expA;
	int_fast8_t kA = 0;
	uint_fast32_t frac32Z;

	uiA = pA;
	uiB = pB;

	//NaR or Zero
	if (uiA == 0x8000 || uiB == 0x8000) {
		uZ = 0x8000;
		return uZ;
	}
	else if (uiA == 0 || uiB == 0) {
		uZ = 0;
		return uZ;
	}

	signA = signP16UI(uiA);
	signB = signP16UI(uiB);
	signZ = signA ^ signB;

	if (signA) uiA = (-uiA & 0xFFFF);
	if (signB) uiB = (-uiB & 0xFFFF);

	regSA = signregP16UI(uiA);
	regSB = signregP16UI(uiB);

	tmp = (uiA << 2) & 0xFFFF;
	if (regSA) {
		while (tmp >> 15) {
			kA++;
			tmp = (tmp << 1) & 0xFFFF;
		}
	}
	else {
		kA = -1;
		while (!(tmp >> 15)) {
			kA--;
			tmp = (tmp << 1) & 0xFFFF;
		}
		tmp &= 0x7FFF;
	}
	expA = tmp >> 14;
	fracA = (0x4000 | tmp);

	tmp = (uiB << 2) & 0xFFFF;
	if (regSB) {
		while (tmp >> 15) {
			kA++;
			tmp = (tmp << 1) & 0xFFFF;
		}
	}
	else {
		kA--;
		while (!(tmp >> 15)) {
			kA--;
			tmp = (tmp << 1) & 0xFFFF;
		}
		tmp &= 0x7FFF;
	}
	expA += tmp >> 14;
	frac32Z = (uint_fast32_t)fracA * (0x4000 | tmp);

	if (expA>1) {
		kA++;
		expA ^= 0x2;
	}

	rcarry = frac32Z >> 29;//3rd bit of frac32Z
	if (rcarry) {
		if (expA) kA++;
		expA ^= 1;
		frac32Z >>= 1;
	}

	if (kA<0) {
		regA = (-kA & 0xFFFF);
		regSA = 0;
		regime = 0x4000 >> regA;
	}
	else {
		regA = kA + 1;
		regSA = 1;
		regime = 0x7FFF - (0x7FFF >> regA);
	}

	if (regA>14) {
		//max or min pos. exp and frac does not matter.
		(regSA) ? (uZ = 0x7FFF) : (uZ = 0x1);
	}
	else {
		//remove carry and rcarry bits and shift to correct position
		frac32Z = (frac32Z & 0xFFFFFFF) >> (regA - 1);
		fracA = (uint_fast16_t)(frac32Z >> 16);

		if (regA != 14) bitNPlusOne |= bool(0x8000 & frac32Z);
		else if (fracA>0) {
			fracA = 0;
			bitsMore = 1;
		}
		if (regA == 14 && expA) bitNPlusOne = 1;

		//sign is always zero
		uZ = packToP16UI(regime, regA, expA, fracA);
		//n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
		if (bitNPlusOne) {
			(0x7FFF & frac32Z) ? (bitsMore = 1) : (bitsMore = 0);
			uZ += (uZ & 1) | bitsMore;
		}
	}

	if (signZ) uZ = -uZ & 0xFFFF;
	return uZ;
}

posit16_t p16_div(posit16_t pA, posit16_t pB) {
	posit16_t uZ;
	uint_fast16_t uiA, uiB, fracA, fracB, regA, regime, tmp;
	bool signA, signB, signZ, regSA, regSB, bitNPlusOne = 0, bitsMore = 0, rcarry;
	int_fast8_t expA, kA = 0;
	uint_fast32_t frac32A, frac32Z, rem;
	div_t divresult;

	uiA = pA;
	uiB = pB;

	//Zero or infinity
	if (uiA == 0x8000 || uiB == 0x8000 || uiB == 0) {
		uZ = 0x8000;
		return uZ;
	}
	else if (uiA == 0) {
		uZ = 0;
		return uZ;
	}

	signA = signP16UI(uiA);
	signB = signP16UI(uiB);
	signZ = signA ^ signB;
	if (signA) uiA = (-uiA & 0xFFFF);
	if (signB) uiB = (-uiB & 0xFFFF);
	regSA = signregP16UI(uiA);
	regSB = signregP16UI(uiB);

	tmp = (uiA << 2) & 0xFFFF;
	if (regSA) {
		while (tmp >> 15) {
			kA++;
			tmp = (tmp << 1) & 0xFFFF;
		}
	}
	else {
		kA = -1;
		while (!(tmp >> 15)) {
			kA--;
			tmp = (tmp << 1) & 0xFFFF;
		}
		tmp &= 0x7FFF;
	}
	expA = tmp >> 14;
	fracA = (0x4000 | tmp);
	frac32A = fracA << 14;

	tmp = (uiB << 2) & 0xFFFF;
	if (regSB) {
		while (tmp >> 15) {
			kA--;
			tmp = (tmp << 1) & 0xFFFF;
		}
		fracB = (0x4000 | tmp);
	}
	else {
		kA++;
		while (!(tmp >> 15)) {
			kA++;
			tmp = (tmp << 1) & 0xFFFF;
		}
		tmp &= 0x7FFF;
		fracB = (0x4000 | (0x7FFF & tmp));
	}
	expA -= tmp >> 14;

	divresult = div((int)frac32A, (int)fracB);
	frac32Z = divresult.quot;
	rem = divresult.rem;

	if (expA<0) {
		expA = 1;
		kA--;
	}
	if (frac32Z != 0) {
		rcarry = frac32Z >> 14; // this is the hidden bit (14th bit) , extreme right bit is bit 0
		if (!rcarry) {
			if (expA == 0) kA--;
			expA ^= 1;
			frac32Z <<= 1;
		}
	}
	if (kA<0) {
		regA = (-kA & 0xFFFF);
		regSA = 0;
		regime = 0x4000 >> regA;
	}
	else {
		regA = kA + 1;
		regSA = 1;
		regime = 0x7FFF - (0x7FFF >> regA);
	}

	if (regA>14) {
		//max or min pos. exp and frac does not matter.
		(regSA) ? (uZ = 0x7FFF) : (uZ = 0x1);
	}
	else {
		//remove carry and rcarry bits and shift to correct position
		frac32Z &= 0x3FFF;
		fracA = (uint_fast16_t)frac32Z >> (regA + 1);

		if (regA != 14) bitNPlusOne = (frac32Z >> regA) & 0x1;
		else if (fracA>0) {
			fracA = 0;
			bitsMore = 1;
		}
		if (regA == 14 && expA) bitNPlusOne = 1;

		//sign is always zero
		uZ = packToP16UI(regime, regA, expA, fracA);

		if (bitNPlusOne) {
			(((1 << regA) - 1) & frac32Z) ? (bitsMore = 1) : (bitsMore = 0);
			if (rem) bitsMore = 1;
			//n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
			uZ += (uZ & 1) | bitsMore;
		}
	}
	if (signZ) uZ = -uZ & 0xFFFF;

	return uZ;
}

posit16_t p16_sqrt(posit16_t pA) {

	posit16_t uA;
	uint_fast16_t expA, fracA, index, r0, shift, sigma0, uiA, uiZ;
	uint_fast32_t eSqrR0, fracZ, negRem, recipSqrt, shiftedFracZ;
	int_fast16_t kZ;
	bool bitNPlusOne;

	uiA = pA;

	// If sign bit is set, return NaR.
	if (uiA >> 15) {
		uA = 0x8000;
		return uA;
	}
	// If the argument is zero, return zero.
	if (uiA == 0) {
		uA = 0;
		return uA;
	}
	// Compute the square root. Here, kZ is the net power-of-2 scaling of the result.
	// Decode the regime and exponent bit; scale the input to be in the range 1 to 4:
	if (uiA >> 14) {
		kZ = -1;
		while (uiA & 0x4000) {
			kZ++;
			uiA = (uiA << 1) & 0xFFFF;
		}
	}
	else {
		kZ = 0;
		while (!(uiA & 0x4000)) {
			kZ--;
			uiA = (uiA << 1) & 0xFFFF;
		}

	}
	uiA &= 0x3fff;
	expA = 1 - (uiA >> 13);
	fracA = (uiA | 0x2000) >> 1;

	// Use table look-up of first four bits for piecewise linear approx. of 1/sqrt:
	index = ((fracA >> 8) & 0xE) + expA;

	r0 = softposit_approxRecipSqrt0[index]
		- (((uint_fast32_t)softposit_approxRecipSqrt1[index]
			* (fracA & 0x1FF)) >> 13);
	// Use Newton-Raphson refinement to get more accuracy for 1/sqrt:
	eSqrR0 = ((uint_fast32_t)r0 * r0) >> 1;

	if (expA) eSqrR0 >>= 1;
	sigma0 = 0xFFFF ^ (0xFFFF & (((uint64_t)eSqrR0 * (uint64_t)fracA) >> 18));//~(uint_fast16_t) ((eSqrR0 * fracA) >> 18);
	recipSqrt = ((uint_fast32_t)r0 << 2) + (((uint_fast32_t)r0 * sigma0) >> 23);

	// We need 17 bits of accuracy for posit16 square root approximation.
	// Multiplying 16 bits and 18 bits needs 64-bit scratch before the right shift:
	fracZ = (((uint_fast64_t)fracA) * recipSqrt) >> 13;

	// Figure out the regime and the resulting right shift of the fraction:
	if (kZ < 0) {
		shift = (-1 - kZ) >> 1;
		uiZ = 0x2000 >> shift;
	}
	else {
		shift = kZ >> 1;
		uiZ = 0x7fff - (0x7FFF >> (shift + 1));
	}
	// Set the exponent bit in the answer, if it is nonzero:
	if (kZ & 1) uiZ |= (0x1000 >> shift);

	// Right-shift fraction bits, accounting for 1 <= a < 2 versus 2 <= a < 4:
	fracZ = fracZ >> (expA + shift);

	// Trick for eliminating off-by-one cases that only uses one multiply:
	fracZ++;
	if (!(fracZ & 7)) {
		shiftedFracZ = fracZ >> 1;
		negRem = (shiftedFracZ * shiftedFracZ) & 0x3FFFF;
		if (negRem & 0x20000) {
			fracZ |= 1;
		}
		else {
			if (negRem) fracZ--;
		}
	}
	// Strip off the hidden bit and round-to-nearest using last 4 bits.
	fracZ -= (0x10000 >> shift);
	bitNPlusOne = (fracZ >> 3) & 1;
	if (bitNPlusOne) {
		if (((fracZ >> 4) & 1) | (fracZ & 7)) fracZ += 0x10;
	}
	// Assemble the result and return it.
	uA = uiZ | (fracZ >> 4);
	return uA;

}

//softposit_mulAdd_subC => (uiA*uiB)-uiC
//softposit_mulAdd_subProd => uiC - (uiA*uiB)
//Default is always op==0
posit16_t softposit_mulAddP16(uint_fast16_t uiA, uint_fast16_t uiB, uint_fast16_t uiC, uint_fast16_t op) {
	posit16_t uZ;
	uint_fast16_t regZ, fracA, fracZ, regime, tmp;
	bool signA, signB, signC, signZ, regSA, regSB, regSC, regSZ, bitNPlusOne = 0, bitsMore = 0, rcarry;
	int_fast8_t expA, expC, expZ;
	int_fast16_t kA = 0, kC = 0, kZ = 0, shiftRight;
	uint_fast32_t frac32C = 0, frac32Z = 0;

	//NaR
	if (uiA == 0x8000 || uiB == 0x8000 || uiC == 0x8000) {
		uZ = 0x8000;
		return uZ;
	}
	else if (uiA == 0 || uiB == 0) {
		if (op == softposit_mulAdd_subC)
			uZ = -uiC;
		else
			uZ = uiC;
		return uZ;
	}

	signA = signP16UI(uiA);
	signB = signP16UI(uiB);
	signC = signP16UI(uiC);//^ (op == softposit_mulAdd_subC);
	signZ = signA ^ signB;// ^ (op == softposit_mulAdd_subProd);

	if (signA) uiA = (-uiA & 0xFFFF);
	if (signB) uiB = (-uiB & 0xFFFF);
	if (signC) uiC = (-uiC & 0xFFFF);

	regSA = signregP16UI(uiA);
	regSB = signregP16UI(uiB);
	regSC = signregP16UI(uiC);

	tmp = (uiA << 2) & 0xFFFF;
	if (regSA) {
		while (tmp >> 15) {
			kA++;
			tmp = (tmp << 1) & 0xFFFF;
		}
	}
	else {
		kA = -1;
		while (!(tmp >> 15)) {
			kA--;
			tmp = (tmp << 1) & 0xFFFF;
		}
		tmp &= 0x7FFF;
	}
	expA = tmp >> 14;
	fracA = (0x8000 | (tmp << 1)); //use first bit here for hidden bit to get more bits

	tmp = (uiB << 2) & 0xFFFF;
	if (regSB) {
		while (tmp >> 15) {
			kA++;
			tmp = (tmp << 1) & 0xFFFF;
		}
	}
	else {
		kA--;
		while (!(tmp >> 15)) {
			kA--;
			tmp = (tmp << 1) & 0xFFFF;
		}
		tmp &= 0x7FFF;
	}
	expA += tmp >> 14;
	frac32Z = (uint_fast32_t)fracA * (0x8000 | (tmp << 1)); // first bit hidden bit

	if (expA>1) {
		kA++;
		expA ^= 0x2;
	}

	rcarry = frac32Z >> 31;//1st bit of frac32Z
	if (rcarry) {
		if (expA) kA++;
		expA ^= 1;
		frac32Z >>= 1;
	}

	//Add
	if (uiC != 0) {
		tmp = (uiC << 2) & 0xFFFF;
		if (regSC) {
			while (tmp >> 15) {
				kC++;
				tmp = (tmp << 1) & 0xFFFF;
			}
		}
		else {
			kC = -1;
			while (!(tmp >> 15)) {
				kC--;
				tmp = (tmp << 1) & 0xFFFF;
			}
			tmp &= 0x7FFF;
		}
		expC = tmp >> 14;
		frac32C = (0x4000 | tmp) << 16;
		shiftRight = ((kA - kC) << 1) + (expA - expC); //actually this is the scale

		if (shiftRight<0) { // |uiC| > |Prod Z|
			if (shiftRight <= -31) {
				bitsMore = 1;
				frac32Z = 0;
			}
			else if (((frac32Z << (32 + shiftRight)) & 0xFFFFFFFF) != 0) bitsMore = 1;
			if (signZ == signC)
				frac32Z = frac32C + (frac32Z >> -shiftRight);
			else {//different signs
				frac32Z = frac32C - (frac32Z >> -shiftRight);
				signZ = signC;
				if (bitsMore) frac32Z -= 1;
			}
			kZ = kC;
			expZ = expC;

		}
		else if (shiftRight>0) {// |uiC| < |Prod|
								//if (frac32C&((1<<shiftRight)-1)) bitsMore = 1;
			if (shiftRight >= 31) {
				bitsMore = 1;
				frac32C = 0;
			}
			else if (((frac32C << (32 - shiftRight)) & 0xFFFFFFFF) != 0) bitsMore = 1;
			if (signZ == signC)
				frac32Z = frac32Z + (frac32C >> shiftRight);
			else {
				frac32Z = frac32Z - (frac32C >> shiftRight);
				if (bitsMore) frac32Z -= 1;
			}
			kZ = kA;
			expZ = expA;

		}
		else {
			if (frac32C == frac32Z && signZ != signC) { //check if same number
				uZ = 0;
				return uZ;
			}
			else {
				if (signZ == signC)
					frac32Z += frac32C;
				else {
					if (frac32Z<frac32C) {
						frac32Z = frac32C - frac32Z;
						signZ = signC;
					}
					else {
						frac32Z -= frac32C;
					}
				}
			}
			kZ = kA;// actually can be kC too, no diff
			expZ = expA; //same here
		}

		rcarry = 0x80000000 & frac32Z; //first left bit
		if (rcarry) {
			if (expZ) kZ++;
			expZ ^= 1;
			if (frac32Z & 0x1) bitsMore = 1;
			frac32Z = (frac32Z >> 1) & 0x7FFFFFFF;
		}
		else {
			//for subtract cases
			if (frac32Z != 0) {
				while ((frac32Z >> 29) == 0) {
					kZ--;
					frac32Z <<= 2;
				}
			}
			bool ecarry = (0x40000000 & frac32Z) >> 30;

			if (!ecarry) {
				if (expZ == 0) kZ--;
				expZ ^= 1;
				frac32Z <<= 1;
			}
		}
	}
	else {
		kZ = kA;
		expZ = expA;
	}

	if (kZ<0) {
		regZ = (-kZ & 0xFFFF);
		regSZ = 0;
		regime = 0x4000 >> regZ;
	}
	else {
		regZ = kZ + 1;
		regSZ = 1;
		regime = 0x7FFF - (0x7FFF >> regZ);
	}

	if (regZ>14) {
		//max or min pos. exp and frac does not matter.
		(regSZ) ? (uZ = 0x7FFF) : (uZ = 0x1);
	}
	else {
		//remove hidden bits
		frac32Z &= 0x3FFFFFFF;
		fracZ = frac32Z >> (regZ + 17);

		if (regZ != 14) bitNPlusOne = (frac32Z >> regZ) & 0x10000;
		else if (frac32Z>0) {
			fracZ = 0;
			bitsMore = 1;
		}
		if (regZ == 14 && expZ) bitNPlusOne = 1;
		uZ = packToP16UI(regime, regZ, expZ, fracZ);
		if (bitNPlusOne) {
			if ((frac32Z << (16 - regZ)) & 0xFFFFFFFF) bitsMore = 1;
			uZ += (uZ & 1) | bitsMore;
		}
	}

	if (signZ) uZ = -uZ & 0xFFFF;
	return uZ;

}


