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

// incomplete dispatch for posit based FMA
enum {
	softposit_mulAdd_subC = 1,
	softposit_mulAdd_subProd = 2
};

#include "softposit8_ref.hpp"
#include "softposit16_ref.hpp"
#include "softposit32_ref.hpp"

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
				presult = p32_add(pa, pb);
				reference = da + db;
				break;
			case OPCODE_SUB:
				presult = p32_sub(pa, pb);
				reference = da - db;
				break;
			case OPCODE_MUL:
				presult = p32_mul(pa, pb);
				reference = da * db;
				break;
			case OPCODE_DIV:
				presult = p32_div(pa, pb);
				reference = da / db;
				break;
			case OPCODE_SQRT:
				presult = p32_sqrt(pa);
				reference = std::sqrt(da);
				break;
			}
			preference = reference;
		}

		/*
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
				switch (nbits) {
				case 8:
					presult = p8_add(pa, pb);
					break;
				case 16:
					presult = p16_add(pa, pb);
					break;
				case 32:
					presult = p32_add(pa, pb);
					break;
				case 64:
					//presult = p64_add(pa, pb);
					break;
				default:
					break;
				}
				reference = da + db;
				break;
			case OPCODE_SUB:
				switch (nbits) {
				case 8:
					presult = p8_sub(pa, pb);
					break;
				case 16:
					presult = p16_sub(pa, pb);
					break;
				case 32:
					presult = p32_sub(pa, pb);
					break;
				case 64:
					//presult = p64_sub(pa, pb);
					break;
				default:
					break;
				}
				reference = da - db;
				break;
			case OPCODE_MUL:
				switch (nbits) {
				case 8:
					presult = p8_mul(pa, pb);
					break;
				case 16:
					presult = p16_mul(pa, pb);
					break;
				case 32:
					presult = p32_mul(pa, pb);
					break;
				case 64:
					//presult = p64_mul(pa, pb);
					break;
				default:
					break;
				}
				reference = da * db;
				break;
			case OPCODE_DIV:
				switch (nbits) {
				case 8:
					presult = p8_div(pa, pb);
					break;
				case 16:
					presult = p16_div(pa, pb);
					break;
				case 32:
					presult = p32_div(pa, pb);
					break;
				case 64:
					//presult = p64_div(pa, pb);
					break;
				default:
					break;
				}
				reference = da / db;
				break;
			case OPCODE_SQRT:
				switch (nbits) {
				case 8:
					break;
				case 16:
					presult = p16_sqrt(pa);
					break;
				case 32:
					presult = p32_sqrt(pa);
					break;
				case 64:
					//presult = p64_sqrt(pa);
					break;
				default:
					break;
				}
				reference = std::sqrt(da);
				break;
			}
			preference = reference;
		}
		*/
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

				if (presult != preference) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases) ReportBinaryArithmeticErrorInBinary("FAIL", operation_string, pa, pb, preference, presult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccessInBinary("PASS", operation_string, pa, pb, preference, presult);
				}

				posit<nbits, es> psoftposit;
				reference(opcode, da, db, pa, pb, preference, psoftposit);
				std::cout << "softposit = " << posit_format(psoftposit) << std::endl;
				std::cout << "universal = " << posit_format(presult) << std::endl;
			}

			return nrOfFailedTests;
		}

	} // namespace unum
} // namespace sw
