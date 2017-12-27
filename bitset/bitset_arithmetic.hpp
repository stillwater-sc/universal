//  arithmetic.cpp : bitset-based arithmetic operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <bitset>

#include "exceptions.hpp"

namespace sw {
	namespace unum {

		//////////////////////////////////////////////////////////////////////////////////////
		// increment and decrement

		// increment the input bitset in place, and return true if there is a carry generated.
		template<size_t nbits>
		bool increment_bitset(std::bitset<nbits>& number) {
			bool carry = true;  // ripple carry
			for (int i = 0; i < nbits; i++) {
				bool _a = number[i];
				number[i] = _a ^ carry;
				carry = carry & (_a ^ false);
			}
			return carry;
		}

		// increment the input bitset in place, and return true if there is a carry generated.
		// The input number is assumed to be right adjusted starting at nbits-nrBits
		// [1 0 0 0] nrBits = 0 is a noop as there is no word to increment
		// [1 0 0 0] nrBits = 1 is the word [1]
		// [1 0 0 0] nrBits = 2 is the word [1 0]
		// [1 1 0 0] nrBits = 3 is the word [1 1 0], etc.
		template<size_t nbits>
		bool increment_unsigned(std::bitset<nbits>& number, int nrBits = nbits - 1) {
			bool carry = 1;  // ripple carry
			int lsb = nbits - nrBits;
			for (int i = lsb; i < nbits; i++) {
				bool _a = number[i];
				number[i] = _a ^ carry;
				carry = (_a & false) | carry & (_a ^ false);
			}
			return carry;
		}

		// decrement the input bitset in place, and return true if there is a borrow generated.
		template<size_t nbits>
		bool decrement_bitset(std::bitset<nbits>& number) {
			bool borrow = true;
			for (int i = 0; i < nbits; i++) {
				bool _a = number[i];
				number[i] = _a ^ borrow;
				borrow = (!(!_a ^ true) & borrow);
			}
			return borrow;
		}

		//////////////////////////////////////////////////////////////////////////////////////
		// add and subtract

		// add bitsets a and b and return result in bitset sum. Return true if there is a carry generated.
		template<size_t nbits>
		bool add_unsigned(std::bitset<nbits> a, std::bitset<nbits> b, std::bitset<nbits + 1>& sum) {
			bool carry = false;  // ripple carry
			for (int i = 0; i < nbits; i++) {
				bool _a = a[i];
				bool _b = b[i];
				sum[i] = _a ^ _b ^ carry;
				carry = (_a & _b) | carry & (_a ^ _b);
			}
			sum.set(nbits, carry);
			return carry;
		}

		// subtract bitsets a and b and return result in bitset dif. Return true if there is a borrow generated.
		template<size_t nbits>
		bool subtract_unsigned(std::bitset<nbits> a, std::bitset<nbits> b, std::bitset<nbits + 1>& dif) {
			bool borrow = false;  // ripple borrow
			for (int i = 0; i < nbits; i++) {
				bool _a = a[i];
				bool _b = b[i];
				dif[i] = _a ^ _b ^ borrow;
				borrow = (!_a & _b) | (!(!_a ^ !_b) & borrow);
			}
			dif.set(nbits, borrow);
			return borrow;
		}

		template<size_t nbits>
		bool add_signed_magnitude(std::bitset<nbits> a, std::bitset<nbits> b, std::bitset<nbits>& sum) {
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
				carry = (_a & _b) | carry & (_a ^ _b);
			}

			return carry;
		}

		template<size_t nbits>
		bool subtract_signed_magnitude(std::bitset<nbits> a, std::bitset<nbits> b, std::bitset<nbits>& diff) {
			bool sign_a = a.test(nbits - 1);
			bool sign_b = b.test(nbits - 1);
			std::cerr << "subtract_signed_magnitude not implemented yet" << std::endl;
			return false;
		}

		////////////////////////////////////////////////////////////////////////////////////////
		// bitset copy and slice operators

		// copy a bitset into a bigger bitset starting at position indicated by the shift value
		template<size_t src_size, size_t tgt_size>
		void copy_into(const std::bitset<src_size>& src, size_t shift, std::bitset<tgt_size>& tgt) {
			tgt.reset();
			for (size_t i = 0; i < src_size; i++)
				tgt.set(i + shift, src[i]);
		}

		// copy a slice of a bitset into a bigger bitset starting at position indicated by the shift value
		template<size_t src_size, size_t tgt_size>
		void copy_slice_into(std::bitset<src_size>& src, std::bitset<tgt_size>& tgt, size_t begin = 0, size_t end = src_size, size_t shift = 0) {
			// do NOT reset the target!!!
			if (end <= src_size) throw iteration_bound_too_large{};
			if (end + shift < tgt_size) throw iteration_bound_too_large{};
			for (size_t i = begin; i < end; i++)
				tgt.set(i + shift, src[i]);
		}

		template<size_t from, size_t to, size_t src_size>
		std::bitset<to - from> fixed_subset(const std::bitset<src_size>& src) {
			static_assert(from <= to, "from cannot be larger than to");
			static_assert(to <= src_size, "to is larger than src_size");

			std::bitset<to - from> result;
			for (size_t i = 0, end = to - from; i < end; ++i)
				result[i] = src[i + from];
			return result;
		}

		//////////////////////////////////////////////////////////////////////////////////////
		// multiply and divide

		// accumulate the addend to a running accumulator
		template<size_t src_size, size_t tgt_size>
		bool accumulate(const std::bitset<src_size>& addend, std::bitset<tgt_size>& accumulator) {
			bool carry = 0;  // ripple carry
			for (int i = 0; i < src_size; i++) {
				bool _a = addend[i];
				bool _b = accumulator[i];
				accumulator[i] = _a ^ _b ^ carry;
				carry = (_a & _b) | carry & (_a ^ _b);
			}
			return carry;
		}

		// multiply bitsets a and b and return result in bitset result.
		template<size_t operand_size>
		void multiply_unsigned(const std::bitset<operand_size>& a, const std::bitset<operand_size>& b, std::bitset<2 * operand_size>& result) {
			constexpr size_t result_size = 2 * operand_size;
			std::bitset<result_size> addend;
			result.reset();
			if (a.test(0)) {
				copy_into<operand_size, result_size>(b, 0, result);
			}
			for (int i = 1; i < operand_size; i++) {
				if (a.test(i)) {
					copy_into<operand_size, result_size>(b, i, addend);
					bool carry = accumulate(addend, result);   // we should never have a carry
					assert(carry == false);
				}
			}
		}


		// subtract a subtractand from a running accumulator
		template<size_t src_size, size_t tgt_size>
		bool subtract(std::bitset<tgt_size>& accumulator, const std::bitset<src_size>& subtractand) {
			bool borrow = 0;  // ripple carry
			for (int i = 0; i < src_size; i++) {
				bool _a = accumulator[i];
				bool _b = subtractand[i];
				accumulator[i] = _a ^ _b ^ borrow;
				borrow = (!_a & _b) | (!(!_a ^ !_b) & borrow);
			}
			return borrow;
		}

		// divide bitsets a and b and return result in bitset result.
		template<size_t operand_size>
		void integer_divide_unsigned(const std::bitset<operand_size>& a, const std::bitset<operand_size>& b, std::bitset<operand_size>& result) {
			std::bitset<operand_size> subtractand, accumulator;
			result.reset();
			accumulator = a;
			int msb = findMostSignificantBit(b);
			if (msb < 0) {
				throw integer_divide_by_zero{};
			}
			else {
				int shift = operand_size - msb - 1;
				// prepare the subtractand
				subtractand = b;
				subtractand <<= shift;
				for (int i = operand_size-msb-1; i >= 0; --i) {
					if (subtractand <= accumulator) {
						bool borrow = subtract(accumulator, subtractand);
						result.set(i);
					}
					else {
						result.reset(i);
					}
					subtractand >>= 1;
				}
			}
		}

		// divide bitsets a and b and return result in bitset result. 
		// By providing more bits in the result, the algorithm will fill these with fraction bits if available.
		// Radix point must be maintained by calling function.
		template<size_t operand_size, size_t result_size>
		void divide_with_fraction(const std::bitset<operand_size>& a, const std::bitset<operand_size>& b, std::bitset<result_size>& result) {
			std::bitset<result_size> subtractand, accumulator;
			result.reset();
			copy_into<operand_size, result_size>(a, result_size - operand_size, accumulator);
			int msb = findMostSignificantBit(b);
			if (msb < 0) {
				throw integer_divide_by_zero{};
			}
			else {
				int shift = operand_size - msb - 1;
				// prepare the subtractand
				copy_into<operand_size, result_size>(b, result_size - operand_size, subtractand);
				subtractand <<= shift;
				for (int i = result_size - msb - 1; i >= 0; --i) {
					std::cout << "accumulator " << accumulator << std::endl;
					std::cout << "subtractand " << subtractand << std::endl;
					if (subtractand <= accumulator) {
						bool borrow = subtract(accumulator, subtractand);
						//assert(borrow == false);
						result.set(i);
					}
					else {
						result.reset(i);
					}
					std::cout << "result      " << result << std::endl;
					subtractand >>= 1;
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////
		// truncating and rounding

		// truncate right-side
		template<size_t src_size, size_t tgt_size>
		void truncate(std::bitset<src_size>& src, std::bitset<tgt_size>& tgt) {
			tgt.reset();
			for (size_t i = 0; i < tgt_size; i++)
				tgt.set(tgt_size - 1 - i, src[src_size - 1 - i]);
		}

		// round
		template<size_t tgt_size, size_t src_size>
		struct round_t
		{
			static std::bitset<tgt_size> eval(const std::bitset<src_size>& src, size_t n)
			{
				static_assert(src_size > 0 && tgt_size > 0, "We don't bother with empty sets.");
				if (n >= src_size)
					throw round_off_all{};

				// look for cut-off leading bits
				for (size_t leading = tgt_size + n; leading < src_size; ++leading)
					if (src[leading])
						throw cut_off_leading_bit{};

				std::bitset<tgt_size> result((src >> n).to_ullong()); // convert to size_t to deal with different sizes

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
			static std::bitset<0> eval(const std::bitset<src_size>&, size_t)
			{
				return {};
			}
		};



		/** Round off \p n last bits of bitset \p src. Round to nearest resulting in potentially smaller bitset.
		*  Doesn't return carry bit in case of overflow while rounding up! TODO: Check whether we need carry or we require an extra bit for this case.
		*/
		template<size_t tgt_size, size_t src_size>
		std::bitset<tgt_size> round(const std::bitset<src_size>& src, size_t n)
		{
			return round_t<tgt_size, src_size>::eval(src, n);
		};




	} // namespace unum

} // namespace sw

