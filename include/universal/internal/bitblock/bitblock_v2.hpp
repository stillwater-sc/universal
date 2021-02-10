#pragma once
//  bitblock.hpp : bitblock class
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <sstream>

// this should be removed when we have made the transition away from std::bitset to sw::unum::bitblock
#include <cassert>
#include "ubb.h"


namespace sw { namespace universal {

		/**
		 @brief templated class implementing efficient multi-precision binary arithmetic and logic
		 */
		template<size_t nbits>
		class bitblock : public universal_bitset::bitset<nbits> {
		public:
			bitblock() {
				setzero();
			}

			bitblock(const bitblock&) = default;
			bitblock(bitblock&&) = default;
			bitblock& operator=(const bitblock&) = default;
			bitblock& operator=(bitblock&&) = default;

			// assignment operators for native types
			bitblock& operator=(signed char rhs) { universal_bitset::bitset<nbits>::operator=(rhs); return *this; }
			bitblock& operator=(short rhs) { universal_bitset::bitset<nbits>::operator=(rhs); return *this;	}
			bitblock& operator=(int rhs) { universal_bitset::bitset<nbits>::operator=(rhs); return *this; }
			bitblock& operator=(long rhs) { universal_bitset::bitset<nbits>::operator=(rhs); return *this; }
			bitblock& operator=(long long rhs) { universal_bitset::bitset<nbits>::operator=(rhs); return *this;	}
			bitblock& operator=(char rhs) { universal_bitset::bitset<nbits>::operator=(rhs); return *this; }
			bitblock& operator=(unsigned short rhs) {universal_bitset::bitset<nbits>::operator=(rhs); return *this; }
			bitblock& operator=(unsigned int rhs) { universal_bitset::bitset<nbits>::operator=(rhs); return *this; }
			bitblock& operator=(unsigned long rhs) { universal_bitset::bitset<nbits>::operator=(rhs); return *this;	}
			bitblock& operator=(unsigned long long rhs) { universal_bitset::bitset<nbits>::operator=(rhs); return *this; }

			unsigned long long to_ullong() const {
				return this->M_do_to_ullong();
			}

			void setzero() {
				universal_bitset::bitset<nbits>::reset();
			}

			bool load_bits(const std::string& string_of_bits) {
				if (string_of_bits.length() != nbits)
					return false;

				setzero();
				int msb = nbits - 1;

				for (std::string::const_iterator it = string_of_bits.begin(); it != string_of_bits.end(); ++it) {

					if (*it == '0') {
						this->reset(msb--);
					}
					else if (*it == '1') {
						this->set(msb--);
					}
					else {
						return false;
					}
				}
				return true;
			}
		};

		/**
		   @param val is a bitblock value.

		   @return value of sign bit.
		 */
		template<size_t nbits>
		unsigned int getSignBit(const bitblock<nbits>& val) {
			return val[nbits - 1];
		}

		// logic operators:  unsigned (in)equality inherited.

		// this comparison is for a two's complement number only
		template<size_t nbits>
		bool lessThan(const bitblock<nbits>& lhs,
			const bitblock<nbits>& rhs) {
			// Short-circuits on disparate signs:
			if (getSignBit(lhs) == 0 && getSignBit(rhs) == 1) {
				return false;
			}
			if (getSignBit(lhs) == 1 && getSignBit(rhs) == 0) {
				return true;
			}

			// Sign bits agree, can compare bit patterns directly:
			return lhs < rhs;
		}

		/**
		  @brief Vestigial alias for '<' operator:  unsigned.
		*/
		template<size_t nbits>
		bool lessThan_unsigned(const bitblock<nbits> &lhs,
			const bitblock<nbits> &rhs) {
			return lhs < rhs;
		}

		////////////////////////////// ARITHMETIC functions
		//////////////////////////////////////////////////////////////////////////////////////
		// increment and decrement

		/**
		   @brief Increments value in place.

		   @param[in, out] number is the value to decrement.

		   @return true iff the sign has changed from nonnegative to negative.
		 */
		template<size_t nbits>
		bool increment_bitset(bitblock<nbits>& number) {
			unsigned int signPre = getSignBit(number);
			number.increment();
			unsigned int signPost = getSignBit(number);
			return signPost > signPre;
		}

		// increment the input bitset in place, and return true if there is a carry generated.
		// The input number is assumed to be right adjusted starting at nbits-nrBits
		// [1 0 0 0] nrBits = 0 is a noop as there is no word to increment
		// [1 0 0 0] nrBits = 1 is the word [1]
		// [1 0 0 0] nrBits = 2 is the word [1 0]
		// [1 1 0 0] nrBits = 3 is the word [1 1 0], etc.

		template<size_t nbits>
		bool increment_unsigned(bitblock<nbits>& number, int nrBits = nbits - 1) {
			bool carry = 1;  // ripple carry
			int lsb = nbits - nrBits;
			for (size_t i = lsb; i < nbits; i++) {
				bool _a = number[i];
				number[i] = _a ^ carry;
				carry = (_a & false) | (carry & (_a ^ false));
			}
			return carry;
		}


		/**
		   @brief Decrements value in place.

		   @param[in, out] number is the value to decrement.

		   @return true iff the sign has changed from negative to nonnegative.
		 */
		template<size_t nbits>
		bool decrement_bitset(bitblock<nbits>& number) {
			unsigned int signPre = getSignBit(number);
			(void)number.decrement();
			unsigned int signPost = getSignBit(number);
			return signPost < signPre;
		}


		//////////////////////////////////////////////////////////////////////////////////////

		/**
		   @brief Adds two nbit summands into nbit+1 result.

		   @param a is an addend.

		   @param b is an addend.

		   @param[out] sum contains the sum, with carry bit set.

		   @return true iff carry precipitated.
		*/
		template<size_t nbits>
		bool add_unsigned(const bitblock<nbits> &a,
			const bitblock<nbits> &b,
			bitblock<nbits + 1>& sum) {
			return sum.add(a, b);
		}

		// subtract bitsets a and b and return result in bitset dif. Return true if there is a borrow generated.
		template<size_t nbits>
		bool subtract_unsigned(bitblock<nbits> a,
			bitblock<nbits> b,
			bitblock<nbits + 1>& dif) {
			return dif.sub(a, b);
		}

		template<size_t nbits>
		bool add_signed_magnitude(bitblock<nbits> a,
			bitblock<nbits> b,
			bitblock<nbits>& sum) {
			uint8_t carry = 0;
			bool sign_a = a.test(nbits - 1);
			if (sign_a) {
				a = a.flip();
				carry += 1;
			}
			bool sign_b = b.test(nbits - 1);
			if (sign_b) {
				b = b.flip();
				carry += 1;
			}
			for (int i = 0; i < nbits - 2; i++) {
				bool _a = a[i];
				bool _b = b[i];
				sum[i] = _a ^ _b ^ carry;
				carry = (_a & _b) | (carry & (_a ^ _b));
			}

			return carry;
		}


		template<size_t nbits>
		bool subtract_signed_magnitude(bitblock<nbits> a,
			bitblock<nbits> b,
			bitblock<nbits>& diff) {
			bool sign_a = a.test(nbits - 1);
			bool sign_b = b.test(nbits - 1);
			std::cerr << "subtract_signed_magnitude not implemented yet" << std::endl;
			return false;
		}


		// integral type to bitblock transformations
		// we are using a full nbits sized bitset even though nbits-3 is the maximum fraction
		// a posit would contain. However, we need an extra bit after the cut-off to make the
		// round up/down decision. The <nbits-something> size created a lot of sw complexity
		// that isn't worth the trouble, so we are simplifying and simply manage a full nbits
		// of fraction bits.

		template<size_t nbits>
		bitblock<nbits> extract_23b_fraction(uint32_t _23b_fraction_without_hidden_bit) {
			bitblock<nbits> _fraction;
			uint32_t mask = uint32_t(0x00400000ul);
			unsigned int ub = (nbits < 23 ? nbits : 23);
			for (unsigned int i = 0; i < ub; i++) {
				_fraction[nbits - 1 - i] = _23b_fraction_without_hidden_bit & mask;
				mask >>= 1;
			}
			return _fraction;
		}

		template<size_t nbits>
		bitblock<nbits> extract_52b_fraction(uint64_t _52b_fraction_without_hidden_bit) {
			bitblock<nbits> _fraction;
			uint64_t mask = uint64_t(0x0008000000000000ull);
			unsigned int ub = (nbits < 52 ? nbits : 52);
			for (unsigned int i = 0; i < ub; i++) {
				_fraction[nbits - 1 - i] = _52b_fraction_without_hidden_bit & mask;
				mask >>= 1;
			}
			return _fraction;
		}


		template<size_t nbits>
		bitblock<nbits> extract_63b_fraction(uint64_t _63b_fraction_without_hidden_bit) {
			bitblock<nbits> _fraction;
			uint64_t mask = uint64_t(0x4000000000000000ull);
			unsigned int ub = (nbits < 63 ? nbits : 63);
			for (unsigned int i = 0; i < ub; i++) {
				_fraction[nbits - 1 - i] = _63b_fraction_without_hidden_bit & mask;
				mask >>= 1;
			}
			return _fraction;
		}

		// 128 bit unsigned int mapped to two uint64_t elements
		typedef struct __uint128 {
			uint64_t lower;
			uint64_t upper;
		} uint128;


		// take in a long double mapped to two uint64_t elements
		template<size_t nbits>
		bitblock<nbits> extract_long_double_fraction(uint128* _112b_fraction_without_hidden_bit) {
			bitblock<nbits> _fraction;
			int msb = nbits - 1;
			uint64_t mask = uint64_t(0x0000800000000000ull);
			unsigned int ub = (nbits < 48 ? nbits : 48); // 48 bits in the upper half
			for (unsigned int i = 0; i < ub; i++) {
				_fraction[msb--] = _112b_fraction_without_hidden_bit->upper & mask;
				mask >>= 1;
			}
			mask = uint64_t(0x8000000000000000ull);
			ub = (nbits < 112 - 48 ? nbits : 112 - 48); // max 64 bits in the lower half
			for (unsigned int i = 0; i < ub; i++) {
				_fraction[msb--] = _112b_fraction_without_hidden_bit->lower & mask;
				mask >>= 1;
			}
			return _fraction;
		}

		template<size_t nbits>
		bitblock<nbits> copy_integer_fraction(unsigned long long _fraction_without_hidden_bit) {
			bitblock<nbits> _fraction;
			uint64_t mask = uint64_t(0x8000000000000000ull);
			unsigned int ub = (nbits < 64 ? nbits : 64);
			for (unsigned int i = 0; i < ub; i++) {
				_fraction[nbits - 1 - i] = _fraction_without_hidden_bit & mask;
				mask >>= 1;
			}
			return _fraction;
		}


		////////////////////////////////////////////////////////////////////////////////////////
		// bitset copy and slice operators

		// copy a bitset into a bigger bitset starting at position indicated by the shift value
		template<size_t src_size, size_t tgt_size>
		void copy_into(const bitblock<src_size>& src, size_t shift, bitblock<tgt_size>& tgt) {
			tgt.reset();
			for (size_t i = 0; i < src_size; i++)
				tgt.set(i + shift, src[i]);
		}


		// copy a slice of a bitset into a bigger bitset starting at position indicated by the shift value
		template<size_t src_size, size_t tgt_size>
		void copy_slice_into(bitblock<src_size>& src, bitblock<tgt_size>& tgt, size_t begin = 0, size_t end = src_size, size_t shift = 0) {
			// do NOT reset the target!!!
			if (end <= src_size) throw iteration_bound_too_large{};
			if (end + shift < tgt_size) throw iteration_bound_too_large{};
			for (size_t i = begin; i < end; i++)
				tgt.set(i + shift, src[i]);
		}

		template<size_t from, size_t to, size_t src_size>
		bitblock<to - from> fixed_subset(const bitblock<src_size>& src) {
			static_assert(from <= to, "from cannot be larger than to");
			static_assert(to <= src_size, "to is larger than src_size");

			bitblock<to - from> result;
			for (size_t i = 0, end = to - from; i < end; ++i)
				result[i] = src[i + from];

			return result;
		}

		//////////////////////////////////////////////////////////////////////////////////////
		// multiply and divide


		// multiply bitsets a and b and return result in bitset result.
		template<size_t operand_size>
		void multiply_unsigned(const bitblock<operand_size>& a, const bitblock<operand_size>& b, bitblock<2 * operand_size>& result) {
			constexpr size_t result_size = 2 * operand_size;
			bitblock<result_size> addend;
			result.reset();
			if (a.test(0)) {
				copy_into<operand_size, result_size>(b, 0, result);
			}
			for (size_t i = 1; i < operand_size; i++) {
				if (a.test(i)) {
					copy_into<operand_size, result_size>(b, i, addend);
					(void)result.add(addend);
					// we should never have a carry:  assert(carry == false);
				}
			}
		}


		// divide bitsets a and b and return result in bitset result.
		template<size_t operand_size>
		void integer_divide_unsigned(const bitblock<operand_size>& a,
			const bitblock<operand_size>& b,
			bitblock<2 * operand_size>& result) {
			bitblock<operand_size> subtractand, accumulator;
			result.reset();
			accumulator = a;
			int msb = findMostSignificantBit(b);
			if (msb < 0) {
				throw integer_divide_by_zero{};
			}

			const unsigned int shift = operand_size - msb - 1;
			// prepare the subtractand
			subtractand = b;
			subtractand <<= shift;
			for (int i = shift; i >= 0; --i) {
				if (subtractand <= accumulator) {
					(void)accumulator.sub(subtractand);
					result.set(i);
				}
				else {
					result.reset(i);
				}
				subtractand >>= 1;
			}
		}


		// divide bitsets a and b and return result in bitset result. 
		// By providing more bits in the result, the algorithm will fill these with fraction bits if available.
		// Radix point must be maintained by calling function.
		template<size_t operand_size, size_t result_size>
		void divide_with_fraction(const bitblock<operand_size>& a,
			const bitblock<operand_size>& b,
			bitblock<result_size>& result) {
			bitblock<result_size> subtractand, accumulator;
			result.reset();
			copy_into<operand_size, result_size>(a, result_size - operand_size, accumulator);
			int msb = findMostSignificantBit(b);
			if (msb < 0) {
				throw integer_divide_by_zero{};
			}

			int shift = operand_size - msb - 1;
			// prepare the subtractand
			copy_into<operand_size, result_size>(b, result_size - operand_size, subtractand);
			subtractand <<= shift;
			for (int i = result_size - msb - 1; i >= 0; --i) {
				//std::cout << "accumulator " << accumulator << std::endl;
				//std::cout << "subtractand " << subtractand << std::endl;
				if (subtractand <= accumulator) {
					(void)accumulator.sub(subtractand);
					//assert(borrow == false);
					result.set(i);
				}
				else {
					result.reset(i);
				}
				//std::cout << "result      " << result << std::endl;
				subtractand >>= 1;
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////
		// truncating and rounding

		// truncate right-side
		template<size_t src_size, size_t tgt_size>
		void truncate(bitblock<src_size>& src, bitblock<tgt_size>& tgt) {
			tgt.reset();
			for (size_t i = 0; i < tgt_size; i++)
				tgt.set(tgt_size - 1 - i, src[src_size - 1 - i]);
		}


		// round
		template<size_t tgt_size, size_t src_size>
		struct round_t
		{
			static bitblock<tgt_size> eval(const bitblock<src_size>& src, size_t n)
			{
				static_assert(src_size > 0 && tgt_size > 0, "We don't bother with empty sets.");
				if (n >= src_size)
					throw round_off_all{};

				// look for cut-off leading bits
				for (size_t leading = tgt_size + n; leading < src_size; ++leading)
					if (src[leading])
						throw cut_off_leading_bit{};

				bitblock<tgt_size> result((src >> n).to_ullong()); // convert to size_t to deal with different sizes
				if (n > 0 && src[n - 1]) {                                // round up potentially if first cut-off bit is true
#         ifdef POSIT_ROUND_TIES_AWAY_FROM_ZERO             // TODO: Evil hack to be consistent with assign_fraction, for testing only
					result = result.to_ullong() + 1;
#         else            

					bool more_bits = false;
					for (long i = 0; i + 1 < n && !more_bits; ++i)
						more_bits |= src[i];
					if (more_bits) {
						result = result.to_ullong() + 1;                // increment_unsigned is ambiguous 
					}
					else {                                            // tie: round up odd number
#             ifndef POSIT_ROUND_TIES_TO_ZERO               // TODO: evil hack to be removed later
						if (result[0])
							result = result.to_ullong() + 1;
#             endif
					}
#         endif

				}
				return result;
			}
		};

		template<size_t src_size>
		struct round_t<0, src_size>
		{
			static bitblock<0> eval(const bitblock<src_size>&, size_t)
			{
				return {};
			}
		};



		/** Round off \p n last bits of bitset \p src. Round to nearest resulting in potentially smaller bitset.
		 *  Doesn't return carry bit in case of overflow while rounding up! TODO: Check whether we need carry or we require an extra bit for this case.
		 */
		template<size_t tgt_size, size_t src_size>
		bitblock<tgt_size> round(const bitblock<src_size>& src, size_t n)
		{
			return round_t<tgt_size, src_size>::eval(src, n);
		}


		////////////////////////////// HELPER functions

		// Attention!
		// Achtung!
		// Regardez!
		// Waarschuwing!
		// DANGER: this depends on the implicit type conversion of number
		// to a uint64_t for sign-extending a 2's complement number system.
		// If nbits > 64 then this code breaks.
		template<size_t nbits, class Type>
		bitblock<nbits> convert_to_bitblock(Type number) {
			bitblock<nbits> bBits;
			const uint64_t one = uint64_t(1);
			for (std::size_t i = 0; i < nbits; i++) {
				bBits[i] = (one << i) & number;
			}

			return bBits;
		}

		/**
		   @brief Constructs a string of 1's and 0's representing the
		   value passed.

		   @param bit contains the value to display.

		   @return string representing the value.
		 */
		template<size_t nbits>
		std::string to_binary(bitblock<nbits> const &bits) {
			char str[nbits + 1];
			str[nbits] = 0;
			for (size_t i = 0; i < nbits; i++) {
				str[nbits - 1 - i] = bits[i] ? '1' : '0';
			}
			return std::string(str);
		}

		template<size_t nbits>
		std::string to_hex(bitblock<nbits> bits) {
			char str[(nbits >> 2) + 2];   // plenty of room
			for (size_t i = 0; i < (nbits >> 2) + 2; ++i) str[i] = 0;
			const char* hexits = "0123456789ABCDEF";
			unsigned int maxHexDigits = (nbits >> 2) + ((nbits % 4) ? 1 : 0);
			for (unsigned int i = 0; i < maxHexDigits; i++) {
				unsigned int hexit;
				switch (nbits) {
				case 1:
					hexit = bits[0];
					break;
				case 2:
					hexit = (bits[1] << 1) + bits[0];
					break;
				case 3:
					hexit = (bits[2] << 2) + (bits[1] << 1) + bits[0];
					break;
				default:
					hexit = (bits[3] << 3) + (bits[2] << 2) + (bits[1] << 1) + bits[0];
					break;
				}
				str[maxHexDigits - 1 - i] = hexits[hexit];
				bits >>= 4;
			}
			str[maxHexDigits] = 0;  // null terminated string
			return std::string(str);
		}

		// convert a sign/magnitude number to a string
		template<size_t nbits>
		std::string sign_magnitude_to_string(bitblock<nbits> bits) {
			std::stringstream ss;
			ss << (bits[nbits - 1] ? "n-" : "p-");
			if (nbits < 2) return ss.str();
			for (int i = nbits - 2; i >= 0; --i) {
				ss << (bits[i] ? "1" : "0");
			}
			return ss.str();
		}


		// find the MSB, return position if found, return -1 if no bits are set
		template<size_t nbits>
		int findMostSignificantBit(const bitblock<nbits>& bits) {
			return bits.getMSB();
		}

		// calculate the 1's complement of a sign-magnitude encoded number

		template<size_t nbits>
		bitblock<nbits> ones_complement(const bitblock<nbits> &number) {
			bitblock<nbits> complement = number;
			complement.flip();
			return complement;
		}


		// calculate the 2's complement of a 2's complement encoded number
		template<size_t nbits>
		bitblock<nbits> twos_complement(const bitblock<nbits> &number) {
			bitblock<nbits> complement;
			uint8_t carry = 1;
			for (size_t i = 0; i < nbits; i++) {
				uint8_t slice = uint8_t(!number[i]) + carry;
				carry = slice >> 1;
				complement[i] = (0x1 & slice);
			}

			return complement;
		}


		/**
		   @brief Flips the sign bit of value passed.

		   @param number is a value for which the sign is to be flipped.

		   @return new bitset with the sign flipped as compared to number
		*/
		template<size_t nbits>
		bitblock<nbits> flip_sign_bit(const bitblock<nbits> &number) {
			bitblock<nbits> negate = number;
			negate.flip(nbits - 1);
			return negate;
		}

		/**
		   @return true iff any bit at or right of msb is set.
		 */
		template<size_t nbits>
		bool anyAfter(const bitblock<nbits>& bits, unsigned msb) {
			bitblock<nbits> shBits = bits;
			shBits <<= (nbits - 1 - msb);
			return shBits.count() > 0;
		}

}} // namespace sw::universal
