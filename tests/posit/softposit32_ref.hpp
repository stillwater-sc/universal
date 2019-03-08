
#define signP32UI( a ) ((bool) ((uint32_t) (a)>>31))
#define signregP32UI( a ) ((bool) (((uint32_t) (a)>>30) & 0x1))
#define packToP32UI(regime, expA, fracA) ( (uint32_t) regime + (uint32_t) expA + ((uint32_t)(fracA)) )



using posit32_t = uint32_t;


// add reference from SoftPosit
posit32_t softposit_addMagsP32(uint_fast32_t uiA, uint_fast32_t uiB) {
	uint_fast16_t regA, regB;
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

	uint_fast16_t regA, regB;
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
	posit32_t uA, uB, uZ;
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