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
#define signP8UI( a ) ((bool) ((uint8_t) (a)>>7))
#define signregP8UI( a ) ((bool) (((uint8_t) (a)>>6) & 0x1))
#define packToP8UI( regime, fracA) ((uint8_t) regime + ((uint8_t)(fracA)) )

using posit8_t = uint8_t;

// add magnitudes
posit8_t softposit_addMagsP8(uint_fast8_t uiA, uint_fast8_t uiB) {
	uint_fast8_t regA;
	uint_fast16_t frac16A, frac16B;
	uint_fast8_t fracA = 0, regime, tmp;
	bool sign, regSA, regSB, rcarry = 0, bitNPlusOne = 0, bitsMore = 0;
	int_fast8_t kA = 0;
	int_fast16_t shiftRight;
	posit8_t uZ;

	sign = signP8UI(uiA); //sign is always positive.. actually don't have to do this.
	if (sign) {
		uiA = -uiA & 0xFF;
		uiB = -uiB & 0xFF;
	}

	if ((int_fast8_t)uiA < (int_fast8_t)uiB) {
		uiA ^= uiB;
		uiB ^= uiA;
		uiA ^= uiB;
	}
	regSA = signregP8UI(uiA);
	regSB = signregP8UI(uiB);

	tmp = (uiA << 2) & 0xFF;
	if (regSA) {
		while (tmp >> 7) {
			kA++;
			tmp = (tmp << 1) & 0xFF;
		}
	}
	else {
		kA = -1;
		while (!(tmp >> 7)) {
			kA--;
			tmp = (tmp << 1) & 0xFF;
		}
		tmp &= 0x7F;
	}
	frac16A = (0x80 | tmp) << 7;
	shiftRight = kA;

	tmp = (uiB << 2) & 0xFF;
	if (regSB) {
		while (tmp >> 7) {
			shiftRight--;
			tmp = (tmp << 1) & 0xFF;
		}
	}
	else {
		shiftRight++;
		while (!(tmp >> 7)) {
			shiftRight++;
			tmp = (tmp << 1) & 0xFF;
		}
		tmp &= 0x7F;
	}
	frac16B = (0x80 | tmp) << 7;

	//Manage CLANG (LLVM) compiler when shifting right more than number of bits
	(shiftRight>7) ? (frac16B = 0) : (frac16B >>= shiftRight); //frac32B >>= shiftRight

	frac16A += frac16B;

	rcarry = 0x8000 & frac16A; //first left bit
	if (rcarry) {
		kA++;
		frac16A >>= 1;
	}

	if (kA<0) {
		regA = (-kA & 0xFF);
		regSA = 0;
		regime = 0x40 >> regA;
	}
	else {
		regA = kA + 1;
		regSA = 1;
		regime = 0x7F - (0x7F >> regA);
	}

	if (regA>6) {
		//max or min pos. exp and frac does not matter.
		(regSA) ? (uZ = 0x7F) : (uZ = 0x1);
	}
	else {
		frac16A = (frac16A & 0x3FFF) >> regA;
		fracA = (uint_fast8_t)(frac16A >> 8);
		bitNPlusOne = (0x80 & frac16A);
		uZ = packToP8UI(regime, fracA);

		//n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
		if (bitNPlusOne) {
			(0x7F & frac16A) ? (bitsMore = 1) : (bitsMore = 0);
			uZ += (uZ & 1) | bitsMore;
		}
	}
	if (sign) uZ = -uZ & 0xFF;
	return uZ;
}

// subtract magnitudes
posit8_t softposit_subMagsP8(uint_fast8_t uiA, uint_fast8_t uiB) {
	uint_fast8_t regA;
	uint_fast16_t frac16A, frac16B;
	uint_fast8_t fracA = 0, regime, tmp;
	bool sign = 0, regSA, regSB, ecarry = 0, bitNPlusOne = 0, bitsMore = 0;
	int_fast16_t shiftRight;
	int_fast8_t kA = 0;
	posit8_t uZ;


	//Both uiA and uiB are actually the same signs if uiB inherits sign of sub
	//Make both positive
	sign = signP8UI(uiA);
	(sign) ? (uiA = (-uiA & 0xFF)) : (uiB = (-uiB & 0xFF));

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

	regSA = signregP8UI(uiA);
	regSB = signregP8UI(uiB);

	tmp = (uiA << 2) & 0xFF;
	if (regSA) {
		while (tmp >> 7) {
			kA++;
			tmp = (tmp << 1) & 0xFF;
		}
	}
	else {
		kA = -1;
		while (!(tmp >> 7)) {
			kA--;
			tmp = (tmp << 1) & 0xFF;
		}
		tmp &= 0x7F;
	}
	frac16A = (0x80 | tmp) << 7;
	shiftRight = kA;

	tmp = (uiB << 2) & 0xFF;
	if (regSB) {
		while (tmp >> 7) {
			shiftRight--;
			tmp = (tmp << 1) & 0xFF;
		}
	}
	else {
		shiftRight++;
		while (!(tmp >> 7)) {
			shiftRight++;
			tmp = (tmp << 1) & 0xFF;
		}
		tmp &= 0x7F;
	}
	frac16B = (0x80 | tmp) << 7;


	if (shiftRight >= 14) {
		uZ = uiA;
		if (sign) uZ = -uZ & 0xFFFF;
		return uZ;
	}
	else
		frac16B >>= shiftRight;

	frac16A -= frac16B;

	while ((frac16A >> 14) == 0) {
		kA--;
		frac16A <<= 1;
	}
	ecarry = (0x4000 & frac16A) >> 14;
	if (!ecarry) {
		kA--;
		frac16A <<= 1;
	}

	if (kA<0) {
		regA = (-kA & 0xFF);
		regSA = 0;
		regime = 0x40 >> regA;
	}
	else {
		regA = kA + 1;
		regSA = 1;
		regime = 0x7F - (0x7F >> regA);
	}

	if (regA>6) {
		//max or min pos. exp and frac does not matter.
		(regSA) ? (uZ = 0x7F) : (uZ = 0x1);
	}
	else {
		frac16A = (frac16A & 0x3FFF) >> regA;
		fracA = (uint_fast8_t)(frac16A >> 8);
		bitNPlusOne = (0x80 & frac16A);
		uZ = packToP8UI(regime, fracA);

		if (bitNPlusOne) {
			(0x7F & frac16A) ? (bitsMore = 1) : (bitsMore = 0);
			uZ += (uZ & 1) | bitsMore;
		}
	}
	if (sign) uZ = -uZ & 0xFF;
	return uZ;
}

// add reference from SoftPosit
posit8_t p8_add(posit8_t a, posit8_t b)
{
	uint_fast8_t uiA, uiB;
	posit8_t uZ;

	uiA = a;
	uiB = b;

	//Zero or infinity
	if (uiA == 0 || uiB == 0) { // Not required but put here for speed
		uZ = uiA | uiB;
		return uZ;
	}
	else if (uiA == 0x80 || uiB == 0x80) {
		uZ = 0x80;
		return uZ;
	}

	//different signs
	if ((uiA^uiB) >> 7)
		return softposit_subMagsP8(uiA, uiB);
	else
		return softposit_addMagsP8(uiA, uiB);

}

// sub reference from SoftPosit
posit8_t p8_sub(posit8_t a, posit8_t b) {
	uint_fast8_t uiA, uiB;
	posit8_t uZ;

	uiA = a;
	uiB = b;

	//infinity
	if (uiA == 0x80 || uiB == 0x80) {
		uZ = 0x80;
		return uZ;
	}
	//Zero
	else if (uiA == 0 || uiB == 0) {
		uZ = (uiA | -uiB);
		return uZ;
	}

	//different signs
	if (signP8UI(uiA^uiB))
		return softposit_addMagsP8(uiA, (-uiB & 0xFF));
	else
		return softposit_subMagsP8(uiA, (-uiB & 0xFF));
}

// mul reference from SoftPosit
posit8_t p8_mul(posit8_t pA, posit8_t pB) {
	using namespace std;
	posit8_t uZ;
	uint_fast8_t uiA, uiB;
	uint_fast8_t regA, fracA, regime, tmp;
	bool signA, signB, signZ, regSA, regSB, bitNPlusOne = 0, bitsMore = 0, rcarry;
	int_fast8_t kA = 0;
	uint_fast16_t frac16Z;

	uiA = pA;
	uiB = pB;

	cout << hex;
	cout << "a = 0x" << int(uiA) << endl;
	cout << "b = 0x" << int(uiB) << endl;
	cout << dec;

	//NaR or Zero
	if (uiA == 0x80 || uiB == 0x80) {
		uZ = 0x80;
		return uZ;
	}
	else if (uiA == 0 || uiB == 0) {
		uZ = 0;
		return uZ;
	}

	signA = signP8UI(uiA);
	signB = signP8UI(uiB);
	signZ = signA ^ signB;

	if (signA) uiA = (-uiA & 0xFF);
	if (signB) uiB = (-uiB & 0xFF);

	regSA = signregP8UI(uiA);
	regSB = signregP8UI(uiB);

	tmp = (uiA << 2) & 0xFF;
	if (regSA) {
		while (tmp >> 7) {
			kA++;
			tmp = (tmp << 1) & 0xFF;
		}
	}
	else {
		kA = -1;
		while (!(tmp >> 7)) {
			kA--;
			tmp = (tmp << 1) & 0xFF;
		}
		tmp &= 0x7F;
	}
	fracA = (0x80 | tmp);

	tmp = (uiB << 2) & 0xFF;
	if (regSB) {
		while (tmp >> 7) {
			kA++;
			tmp = (tmp << 1) & 0xFF;
		}
	}
	else {
		kA--;
		while (!(tmp >> 7)) {
			kA--;
			tmp = (tmp << 1) & 0xFF;
		}
		tmp &= 0x7F;
	}
	frac16Z = (uint_fast16_t)fracA * (0x80 | tmp);
	cout << hex << (frac16Z) << dec << endl;

	rcarry = frac16Z >> 15;//1st bit of frac32Z
	if (rcarry) {
		kA++;
		frac16Z >>= 1;
	}

	if (kA<0) {
		regA = (-kA & 0xFF);
		regSA = 0;
		regime = 0x40 >> regA;
	}
	else {
		regA = kA + 1;
		regSA = 1;
		regime = 0x7F - (0x7F >> regA);
	}

	if (regA>6) {
		//max or min pos. exp and frac does not matter.
		(regSA) ? (uZ = 0x7F) : (uZ = 0x1);
	}
	else {
		//remove carry and rcarry bits and shift to correct position
		frac16Z = (frac16Z & 0x3FFF) >> regA;
		fracA = (uint_fast8_t)(frac16Z >> 8);
		bitNPlusOne = (0x80 & frac16Z);
		uZ = packToP8UI(regime, fracA);

		//n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
		if (bitNPlusOne) {
			(0x7F & frac16Z) ? (bitsMore = 1) : (bitsMore = 0);
			uZ += (uZ & 1) | bitsMore;
		}
	}

	if (signZ) uZ = -uZ & 0xFF;
	return uZ;
}

// div reference from SoftPosit
posit8_t p8_div(posit8_t pA, posit8_t pB) {
	posit8_t uZ;
	uint_fast8_t uiA, uiB, fracA, fracB, regA, regime, tmp;
	bool signA, signB, signZ, regSA, regSB, bitNPlusOne = 0, bitsMore = 0, rcarry;
	int_fast8_t kA = 0;
	uint_fast16_t frac16A, frac16Z, rem;
	div_t divresult;

	uiA = pA;
	uiB = pB;

	//Zero or infinity
	if (uiA == 0x80 || uiB == 0x80 || uiB == 0) {
		uZ = 0x80;
		return uZ;
	}
	else if (uiA == 0) {
		uZ = 0;
		return uZ;
	}

	signA = signP8UI(uiA);
	signB = signP8UI(uiB);
	signZ = signA ^ signB;
	if (signA) uiA = (-uiA & 0xFF);
	if (signB) uiB = (-uiB & 0xFF);
	regSA = signregP8UI(uiA);
	regSB = signregP8UI(uiB);

	tmp = (uiA << 2) & 0xFF;
	if (regSA) {
		while (tmp >> 7) {
			kA++;
			tmp = (tmp << 1) & 0xFF;
		}
	}
	else {
		kA = -1;
		while (!(tmp >> 7)) {
			kA--;
			tmp = (tmp << 1) & 0xFF;
		}
		tmp &= 0x7F;
	}
	fracA = (0x80 | tmp);
	frac16A = fracA << 7; //hidden bit 2nd bit

	tmp = (uiB << 2) & 0xFF;
	if (regSB) {
		while (tmp >> 7) {
			kA--;
			tmp = (tmp << 1) & 0xFF;
		}
		fracB = (0x80 | tmp);
	}
	else {
		kA++;
		while (!(tmp >> 7)) {
			kA++;
			tmp = (tmp << 1) & 0xFF;
		}
		tmp &= 0x7F;
		fracB = (0x80 | (0x7F & tmp));
	}

	divresult = div(frac16A, fracB);
	frac16Z = divresult.quot;
	rem = divresult.rem;

	if (frac16Z != 0) {
		rcarry = frac16Z >> 7; // this is the hidden bit (7th bit) , extreme right bit is bit 0
		if (!rcarry) {
			kA--;
			frac16Z <<= 1;
		}
	}

	if (kA<0) {
		regA = (-kA & 0xFF);
		regSA = 0;
		regime = 0x40 >> regA;
	}
	else {
		regA = kA + 1;
		regSA = 1;
		regime = 0x7F - (0x7F >> regA);
	}
	if (regA>6) {
		//max or min pos. exp and frac does not matter.
		(regSA) ? (uZ = 0x7F) : (uZ = 0x1);
	}
	else {
		//remove carry and rcarry bits and shift to correct position
		frac16Z &= 0x7F;
		fracA = (uint_fast16_t)frac16Z >> (regA + 1);

		bitNPlusOne = (0x1 & (frac16Z >> regA));
		uZ = packToP8UI(regime, fracA);

		std::cout << std::hex;
		std::cout << "frac16z = " << int(frac16Z) << std::endl;
		std::cout << "fracA   = " << int(fracA) << std::endl;
		std::cout << "bits    = " << int(uZ) << std::endl;
		std::cout << std::dec;

		//uZ.ui = (uint16_t) (regime) + ((uint16_t) (expA)<< (13-regA)) + ((uint16_t)(fracA));
		if (bitNPlusOne) {
			(((1 << regA) - 1) & frac16Z) ? (bitsMore = 1) : (bitsMore = 0);
			if (rem) bitsMore = 1;
			std::cout << "bitsMore = " << (bitsMore ? "true" : "false");
			//n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
			uZ += (uZ & 1) | bitsMore;
		}
	}
	if (signZ) uZ = -uZ & 0xFF;

	return uZ;
}

posit8_t p8_sqrt(posit8_t pA) {
	posit8_t uA;
	uint_fast8_t uiA;

	static const uint8_t p8Sqrt[] =
	{ 0, 8, 11, 14, 16, 18, 20, 21, 23, 24, 25, 27, 28, 29, 30, 31, 32,
		33, 34, 35, 36, 37, 38, 38, 39, 40, 41, 42, 42, 43, 44, 45, 45, 46,
		47, 47, 48, 49, 49, 50, 51, 51, 52, 52, 53, 54, 54, 55, 55, 56, 57,
		57, 58, 58, 59, 59, 60, 60, 61, 61, 62, 62, 63, 63, 64, 64, 65, 65,
		66, 66, 67, 67, 68, 68, 69, 69, 70, 70, 70, 71, 71, 72, 72, 72, 73,
		73, 74, 74, 74, 75, 75, 75, 76, 76, 77, 77, 77, 79, 80, 81, 83, 84,
		85, 86, 87, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 100,
		101, 102, 103, 105, 108, 110, 112, 114, 115, 120 };
	uiA = pA;

	if (uiA >= 0x80) {
		uA = 0x80;
		return uA;
	}
	uA = p8Sqrt[uiA];

	return uA;
}

//softposit_mulAdd_subC => (uiA*uiB)-uiC
//softposit_mulAdd_subProd => uiC - (uiA*uiB)
//Default is always op==0
posit8_t softposit_mulAddP8(uint_fast8_t uiA, uint_fast8_t uiB, uint_fast8_t uiC, uint_fast8_t op) {
	posit8_t uZ;
	uint_fast8_t regZ, fracA, fracZ, regime, tmp;
	bool signA, signB, signC, signZ, regSA, regSB, regSC, regSZ, bitNPlusOne = 0, bitsMore = 0, rcarry;
	int_fast8_t kA = 0, kC = 0, kZ = 0, shiftRight;
	uint_fast16_t frac16C, frac16Z;

	//NaR
	if (uiA == 0x80 || uiB == 0x80 || uiC == 0x80) {
		uZ = 0x80;
		return uZ;
	}
	else if (uiA == 0 || uiB == 0) {
		if (op == softposit_mulAdd_subC)
			uZ = -uiC;
		else
			uZ = uiC;
		return uZ;
	}

	signA = signP8UI(uiA);
	signB = signP8UI(uiB);
	signC = signP8UI(uiC);//^ (op == softposit_mulAdd_subC);
	signZ = signA ^ signB;// ^ (op == softposit_mulAdd_subProd);

	if (signA) uiA = (-uiA & 0xFF);
	if (signB) uiB = (-uiB & 0xFF);
	if (signC) uiC = (-uiC & 0xFF);

	regSA = signregP8UI(uiA);
	regSB = signregP8UI(uiB);
	regSC = signregP8UI(uiC);

	tmp = (uiA << 2) & 0xFF;
	if (regSA) {
		while (tmp >> 7) {
			kA++;
			tmp = (tmp << 1) & 0xFF;
		}
	}
	else {
		kA = -1;
		while (!(tmp >> 7)) {
			kA--;
			tmp = (tmp << 1) & 0xFF;
		}
		tmp &= 0x7F;
	}
	fracA = (0x80 | tmp); //use first bit here for hidden bit to get more bits

	tmp = (uiB << 2) & 0xFF;
	if (regSB) {
		while (tmp >> 7) {
			kA++;
			tmp = (tmp << 1) & 0xFF;
		}
	}
	else {
		kA--;
		while (!(tmp >> 7)) {
			kA--;
			tmp = (tmp << 1) & 0xFF;
		}
		tmp &= 0x7F;
	}
	frac16Z = (uint_fast16_t)fracA * (0x80 | tmp);

	rcarry = frac16Z >> 15;//1st bit of frac16Z
	if (rcarry) {
		kA++;
		frac16Z >>= 1;
	}

	if (uiC != 0) {
		tmp = (uiC << 2) & 0xFF;
		if (regSC) {
			while (tmp >> 7) {
				kC++;
				tmp = (tmp << 1) & 0xFF;
			}
		}
		else {
			kC = -1;
			while (!(tmp >> 7)) {
				kC--;
				tmp = (tmp << 1) & 0xFF;
			}
			tmp &= 0x7F;
		}
		frac16C = (0x80 | tmp) << 7;
		shiftRight = (kA - kC);

		if (shiftRight<0) { // |uiC| > |Prod|
			if (shiftRight <= -15) {
				bitsMore = 1;
				frac16Z = 0;
			}
			else if (((frac16Z << (16 + shiftRight)) & 0xFFFF) != 0) bitsMore = 1;
			if (signZ == signC)
				frac16Z = frac16C + (frac16Z >> -shiftRight);
			else {//different signs
				frac16Z = frac16C - (frac16Z >> -shiftRight);
				signZ = signC;
				if (bitsMore) frac16Z -= 1;
			}
			kZ = kC;

		}
		else if (shiftRight>0) {// |uiC| < |Prod|

			if (shiftRight >= 15) {
				bitsMore = 1;
				frac16C = 0;
			}
			else if (((frac16C << (16 - shiftRight)) & 0xFFFF) != 0) bitsMore = 1;
			if (signZ == signC)
				frac16Z += (frac16C >> shiftRight);
			else {
				frac16Z -= (frac16C >> shiftRight);
				if (bitsMore) frac16Z -= 1;
			}
			kZ = kA;
		}
		else {
			if (frac16C == frac16Z && signZ != signC) { //check if same number
				uZ = 0;
				return uZ;
			}
			else {
				if (signZ == signC)
					frac16Z += frac16C;
				else {
					if (frac16Z<frac16C) {
						frac16Z = frac16C - frac16Z;
						signZ = signC;
					}
					else {
						frac16Z -= frac16C;
					}
				}
			}
			kZ = kA;// actually can be kC too, no diff
		}

		rcarry = 0x8000 & frac16Z; //first left bit
		if (rcarry) {
			kZ++;
			frac16Z = (frac16Z >> 1) & 0x7FFF;
		}
		else {

			//for subtract cases
			if (frac16Z != 0) {
				while ((frac16Z >> 14) == 0) {
					kZ--;
					frac16Z <<= 1;
				}
			}
		}

	}
	else {
		kZ = kA;
	}

	if (kZ<0) {
		regZ = (-kZ & 0xFF);
		regSZ = 0;
		regime = 0x40 >> regZ;
	}
	else {
		regZ = kZ + 1;
		regSZ = 1;
		regime = 0x7F - (0x7F >> regZ);
	}

	if (regZ>6) {
		//max or min pos. exp and frac does not matter.
		(regSZ) ? (uZ = 0x7F) : (uZ = 0x1);
	}
	else {
		//remove hidden bits
		frac16Z &= 0x3FFF;

		fracZ = (frac16Z >> regZ) >> 8;

		bitNPlusOne = ((frac16Z >> regZ) & 0x80);
		uZ = packToP8UI(regime, fracZ);

		if (bitNPlusOne) {
			if ((frac16Z << (9 - regZ)) & 0xFFFF) bitsMore = 1;
			uZ += (uZ & 1) | bitsMore;
		}
	}

	if (signZ) uZ = -uZ & 0xFF;
	return uZ;

}

posit8_t p8_mulAdd(posit8_t a, posit8_t b, posit8_t c)
{
	uint_fast8_t uiA, uiB, uiC;

	uiA = a;
	uiB = b;
	uiC = c;
	return softposit_mulAddP8(uiA, uiB, uiC, 0);

}