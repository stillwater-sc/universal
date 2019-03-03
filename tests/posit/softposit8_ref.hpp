
#define signP8UI( a ) ((bool) ((uint8_t) (a)>>7))
#define signregP8UI( a ) ((bool) (((uint8_t) (a)>>6) & 0x1))
#define packToP8UI( regime, fracA) ((uint8_t) regime + ((uint8_t)(fracA)) )

// mul reference from SoftPosit
using posit8_t = uint8_t;

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
