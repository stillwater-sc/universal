#pragma once

// seed sqrt approximation
const uint_fast16_t softposit_approxRecipSqrt0[16] = {
	0xb4c9, 0xffab, 0xaa7d, 0xf11c, 0xa1c5, 0xe4c7, 0x9a43, 0xda29,
	0x93b5, 0xd0e5, 0x8ded, 0xc8b7, 0x88c6, 0xc16d, 0x8424, 0xbae1
};
const uint_fast16_t softposit_approxRecipSqrt1[16] = {
	0xa5a5, 0xea42, 0x8c21, 0xc62d, 0x788f, 0xaa7f, 0x6928, 0x94b6,
	0x5cc7, 0x8335, 0x52a6, 0x74e2, 0x4a3e, 0x68fe, 0x432b, 0x5efd
};

// incomplete dispatch for posit based FMA, default is opcode 0 mulAdd 
//softposit_mulAdd_subC => (uiA*uiB)-uiC
//softposit_mulAdd_subProd => uiC - (uiA*uiB)
//Default is always op==0 (uiA*uiB)+uiC
enum {
	softposit_mulAdd_subC = 1,
	softposit_mulAdd_subProd = 2
};

#include "softposit8_ref.hpp"
#include "softposit16_ref.hpp"
#include "softposit32_ref.hpp"
#include "softposit64_ref.hpp"

namespace sw {
	namespace unum {

		template<size_t nbits, size_t es>
		void reference(int opcode, double da, double db, const posit<nbits, es>& pa, const posit<nbits, es>& pb, posit<nbits, es>& preference, posit<nbits, es>& presult) {
			double reference = 0.0;
			switch (opcode) {
			default:
			case OPCODE_NOP:
				preference.setzero();
				presult.setzero();
				return;
			case OPCODE_ADD:
				presult = pa + pb;
				reference = da + db;
				break;
			case OPCODE_SUB:
				presult = pa - pb;
				reference = da - db;
				break;
			case OPCODE_MUL:
				presult = pa * pb;
				reference = da * db;
				break;
			case OPCODE_DIV:
				presult = pa / pb;
				reference = da / db;
				break;
			case OPCODE_SQRT:
				presult = sw::unum::sqrt(pa);
				reference = std::sqrt(da);
				break;
			}
			preference = reference;
		}

		template<>
		void reference(int opcode, double da, double db, const posit<32, 2>& _pa, const posit<32, 2>& _pb, posit<32, 2>& preference, posit<32, 2>& presult) {
			double reference = 0.0;
			posit32_t pa = posit32_t(_pa.encoding());
			posit32_t pb = posit32_t(_pb.encoding());
			switch (opcode) {
			default:
			case OPCODE_NOP:
				preference.setzero();
				presult.setzero();
				return;
			case OPCODE_ADD:
				presult.set_raw_bits(p32_add(pa, pb));
				reference = da + db;
				break;
			case OPCODE_SUB:
				presult.set_raw_bits(p32_sub(pa, pb));
				reference = da - db;
				break;
			case OPCODE_MUL:
				presult.set_raw_bits(p32_mul(pa, pb));
				reference = da * db;
				break;
			case OPCODE_DIV:
				presult.set_raw_bits(p32_div(pa, pb));
				reference = da / db;
				break;
			case OPCODE_SQRT:
				presult.set_raw_bits(p32_sqrt(pa));
				reference = std::sqrt(da);
				break;
			}
			preference = reference;
		}

		template<size_t nbits, size_t es>
		int ValidateAgainstSoftPosit(std::string tag, bool bReportIndividualTestCases, int opcode, uint32_t nrOfRandoms) {
			const size_t SIZE_STATE_SPACE = nrOfRandoms;
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pb, presult, preference;

			std::string operation_string;
			switch (opcode) {
			default:
			case OPCODE_NOP:
				operation_string = "nop";
				break;
			case OPCODE_ADD:
				operation_string = "+";
				break;
			case OPCODE_SUB:
				operation_string = "-";
				break;
			case OPCODE_MUL:
				operation_string = "*";
				break;
			case OPCODE_DIV:
				operation_string = "/";
				break;
			case OPCODE_SQRT:
				operation_string = "sqrt";
				break;
			}
			// generate the full state space set of valid posit values
			std::random_device rd;     //Get a random seed from the OS entropy device, or whatever
			std::mt19937_64 eng(rd()); //Use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
									   //Define the distribution, by default it goes from 0 to MAX(unsigned long long)
			std::uniform_int_distribution<unsigned long long> distr;
#ifdef POSIT_USE_LONG_DOUBLE
			std::vector<long double> operand_values(SIZE_STATE_SPACE);
			for (uint32_t i = 0; i < SIZE_STATE_SPACE; i++) {
				presult.set_raw_bits(distr(eng));  // take the bottom nbits bits as posit encoding
				operand_values[i] = (long double)(presult);
			}
			long double da, db;
#else // USE DOUBLE
			std::vector<double> operand_values(SIZE_STATE_SPACE);
			for (uint32_t i = 0; i < SIZE_STATE_SPACE; i++) {
				presult.set_raw_bits(distr(eng));  // take the bottom nbits bits as posit encoding
				operand_values[i] = double(presult);
			}
			double da, db;
#endif // POSIT_USE_LONG_DOUBLE
			unsigned ia, ib;  // random indices for picking operands to test
			for (unsigned i = 1; i < nrOfRandoms; i++) {
				ia = std::rand() % SIZE_STATE_SPACE;
				da = operand_values[ia];
				pa = da;
				ib = std::rand() % SIZE_STATE_SPACE;
				db = operand_values[ib];
				pb = db;
				// in case you have numeric_limits<long double>::digits trouble... this will show that
				//std::cout << "sizeof da: " << sizeof(da) << " bits in significant " << (std::numeric_limits<long double>::digits - 1) << " value da " << da << " at index " << ia << " pa " << pa << std::endl;
				//std::cout << "sizeof db: " << sizeof(db) << " bits in significant " << (std::numeric_limits<long double>::digits - 1) << " value db " << db << " at index " << ia << " pa " << pb << std::endl;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					execute(opcode, da, db, pa, pb, preference, presult);
				}
				catch (const posit_arithmetic_exception& err) {
					if (pa.isnar() || pb.isnar() || (opcode == OPCODE_DIV && pb.iszero())) {
						std::cerr << "Correctly caught arithmetic exception: " << err.what() << std::endl;
					}
					else {
						throw err;
					}
				}
#else
				execute(opcode, da, db, pa, pb, preference, presult);
#endif
				posit<nbits, es> psoftposit;
				reference(opcode, da, db, pa, pb, preference, psoftposit);

				if (presult != psoftposit) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases) ReportBinaryArithmeticErrorInBinary("FAIL", operation_string, pa, pb, psoftposit, presult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccessInBinary("PASS", operation_string, pa, pb, psoftposit, presult);
				}
			}

			return nrOfFailedTests;
		}

		void GenerateP16Test(int opcode, uint16_t _a, uint16_t _b, uint16_t _c) {
			using namespace std;
			using namespace sw::unum;

			posit16_t a, b, c;
			a = _a;
			b = _b;
			switch (opcode) {
			case OPCODE_ADD:
				c = p16_add(a, b);
				break;
			case OPCODE_SUB:
				c = p16_sub(a, b);
				break;
			case OPCODE_MUL:
				c = p16_mul(a, b);
				break;
			case OPCODE_DIV:
				c = p16_div(a, b);
				break;
			case OPCODE_SQRT:
				c = p16_sqrt(a);
				break;
			}
			cout << hex;
			cout << "a = 16.1x" << a << "p" << endl;
			cout << "b = 16.1x" << b << "p" << endl;
			cout << "c = 16.1x" << c << "p" << endl;
			cout << dec;

			posit<16, 1> x, y, z, r;
			x.set_raw_bits(_a);
			y.set_raw_bits(_b);
			r.set_raw_bits(_c);
			switch (opcode) {
			case OPCODE_ADD:
				z = x + y;
				break;
			case OPCODE_SUB:
				z = a - b;
				break;
			case OPCODE_MUL:
				z = x * y;
				break;
			case OPCODE_DIV:
				z = x / y;
				break;
			case OPCODE_SQRT:
				z = sw::unum::sqrt(x);
				break;
			}
			cout << "x = " << posit_format(x) << endl;
			cout << "y = " << posit_format(y) << endl;
			cout << "z = " << posit_format(z) << endl;
			cout << "r = " << posit_format(r) << endl;
		}

		void GenerateP32Test(int opcode, uint32_t _a, uint32_t _b, uint32_t _c) {
			using namespace std;
			using namespace sw::unum;

			posit32_t a, b, c;
			a = _a;
			b = _b;
			switch (opcode) {
			case OPCODE_ADD:
				c = p32_add(a, b);
				break;
			case OPCODE_SUB:
				c = p32_sub(a, b);
				break;
			case OPCODE_MUL:
				c = p32_mul(a, b);
				break;
			case OPCODE_DIV:
				c = p32_div(a, b);
				break;
			case OPCODE_SQRT:
				c = p32_sqrt(a);
				break;
			}
			cout << hex;
			if (opcode == OPCODE_SQRT) {
				cout << "a    = 32.2x" << a << "p" << endl;
				cout << "sqrt = 32.2x" << c << "p" << endl;
			}
			else {
				cout << "a = 32.2x" << a << "p" << endl;
				cout << "b = 32.2x" << b << "p" << endl;
				cout << "c = 32.2x" << c << "p" << endl;
			}
			cout << dec;

			posit<32, 2> x, y, z, r;
			x.set_raw_bits(_a);
			y.set_raw_bits(_b);
			r.set_raw_bits(_c);
			switch (opcode) {
			case OPCODE_ADD:
				z = x + y;
				break;
			case OPCODE_SUB:
				z = a - b;
				break;
			case OPCODE_MUL:
				z = x * y;
				break;
			case OPCODE_DIV:
				z = x / y;
				break;
			case OPCODE_SQRT:
				z = sw::unum::sqrt(x);
				break;
			}
			if (opcode == OPCODE_SQRT) {
				cout << "x    = " << posit_format(x) << endl;
				cout << "sqrt = " << posit_format(z) << endl;
			}
			else {
				cout << "x = " << posit_format(x) << endl;
				cout << "y = " << posit_format(y) << endl;
				cout << "z = " << posit_format(z) << endl;
				cout << "r = " << posit_format(r) << endl;
			}
		}

		// general posit decode
		inline void decode_posit(uint16_t bits, bool& sign, int8_t& scale, int16_t& exp, uint32_t& fraction) {
			if (bits == 0x8000) {
				sign = true;
				scale = 28;
				exp = 0;
				fraction = 0;
			}
			else if (bits == 0) {
				sign = false;
				scale = 0;
				exp = 0;
				fraction = 0;
			}
			else {
				sign = bool(bits & 0x8000);
				uint16_t tmp = (bits << 2) & 0xFFFF;
				if (bits & 0x4000) {  // positive regimes
					scale = 0;
					while (tmp >> 15) {
						++scale;
						tmp = (tmp << 1) & 0xFFFF;
					}
				}
				else {              // negative regimes
					scale = -1;
					while (!(tmp >> 15)) {
						--scale;
						tmp = (tmp << 1) & 0xFFFF;
					}
					tmp &= 0x7FFF;
				}
				exp = tmp >> 14;  // extract the exponent
				fraction = (0x0000'4000 | tmp) << 16;  // shift to prepare for arithmetic use
			}
		}

	} // namespace unum
} // namespace sw



template<size_t nbits, size_t es>
void BulkCmpArithmeticOps(int nrOfRandoms = 10) {
	bool bReportIndividualTestCases = true;
	ReportTestResult(ValidateAgainstSoftPosit<nbits, es>("test", bReportIndividualTestCases, OPCODE_ADD, nrOfRandoms), tag, " add ");
	ReportTestResult(ValidateAgainstSoftPosit<nbits, es>("test", bReportIndividualTestCases, OPCODE_SUB, nrOfRandoms), tag, " sub ");
	ReportTestResult(ValidateAgainstSoftPosit<nbits, es>("test", bReportIndividualTestCases, OPCODE_MUL, nrOfRandoms), tag, " mul ");
	ReportTestResult(ValidateAgainstSoftPosit<nbits, es>("test", bReportIndividualTestCases, OPCODE_DIV, nrOfRandoms), tag, " div ");
	ReportTestResult(ValidateAgainstSoftPosit<nbits, es>("test", bReportIndividualTestCases, OPCODE_SQRT, nrOfRandoms), tag, " sqrt ");
}



void DecodePosit(sw::unum::posit<16, 1> p) {
	using namespace std;
	using namespace sw::unum;
	bool sign;
	int8_t scale;
	int16_t exp;
	uint32_t fraction;
	decode_posit(uint16_t(p.encoding()), sign, scale, exp, fraction);
	cout << "raw      0b" << p.get() << dec << endl;
	cout << "sign       " << (sign ? "-1" : "+1") << endl;
	cout << "scale      " << int(scale) << endl;
	cout << "exponent 0x" << hex << exp << dec << endl;
	cout << "fraction 0x" << hex << fraction << dec << endl;
}