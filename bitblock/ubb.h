// Copyright (C) 2001-2018 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

// Adapted from SGI implementation, the copyright for which is include:
/*
 * Copyright (c) 1998
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */

/** @file bitblock/ubb.h
 *  Block-based modification of stdlib bitset.
 */

#ifndef UBB_H /* Universal bitblock */
#define UBB_H 1

using namespace std;

namespace universal_bitset {

        // Ideally, word-sized for target.
//      typedef unsigned char WordT; 
//      typedef unsigned int WordT;
        typedef unsigned long WordT;
//      typedef unsigned long long WordT;

#ifndef __CHAR_BIT__
#define __CHAR_BIT__ 8
#endif

#define UBB_BITS_PER_WORD  (__CHAR_BIT__ * sizeof(WordT))

#define UBB_BITS_PER_ULL (__CHAR_BIT__ * sizeof(unsigned long long))

#define UBB_WORDS(bits) ((bits + UBB_BITS_PER_WORD - 1) / UBB_BITS_PER_WORD)

#if defined(__clang__)
 /* Clang/LLVM. ---------------------------------------------- */
        typedef __128bitdd double_double;

#elif defined(__ICC) || defined(__INTEL_COMPILER)
 /* Intel ICC/ICPC. ------------------------------------------ */
        typedef __128bitdd double_double;

#elif defined(__GNUC__) || defined(__GNUG__)
 /* GNU GCC/G++. --------------------------------------------- */

#define ctz(x) __builtin_ctz(x)
#define ctzl(x) __builtin_ctzl(x)
#define popcnt(x) __builtin_popcount(x)
#define popcntl(x) __builtin_popcountl(x)
#define clearmemory(dst,val,size) __builtin_memset(dst,val,size)

#elif defined(__HP_cc) || defined(__HP_aCC)
 /* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
 /* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
 /* Microsoft Visual Studio. --------------------------------- */

#include <intrin.h>
#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanForward64)
#define clearmemory(dst,val,size) memset(dst,val,size)

#pragma warning( push )
#pragma warning( disable : 4146)
// disable this warning: as the logic is correct
// warning C4146 : unary minus operator applied to unsigned type, result still unsigned

#define ctz(x)  __lzcnt(x)
#define ctzl(x) __lzcnt64(x)
#define popcnt(x)  __popcnt(x)
#define popcntl(x) __popcnt64(x)

  /* Exit?
        static inline uint32_t clz(uint32_t x) {
                x |= (x >> 1);
                x |= (x >> 2);
                x |= (x >> 4);
                x |= (x >> 8);
                x |= (x >> 16);
                return 32 - popcnt(x);
        }
        static inline uint32_t clzl(uint64_t x) {
                x |= (x >> 1);
                x |= (x >> 2);
                x |= (x >> 4);
                x |= (x >> 8);
                x |= (x >> 16);
                x |= (x >> 32);
                return 64 - popcntl(x);
        }


  static inline uint32_t ctz(uint32_t x) {
    return popcnt((x & -x) - 1);
  }

  static inline uint32_t ctzl(uint64_t x) {
    return popcntl((x & -x) - 1);
  }
  */

#pragma warning( pop ) 

#elif defined(__PGI)
 /* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
 /* Oracle Solaris Studio. ----------------------------------- */

#endif

#define leadzeroes(x) (sizeof(WordT) > 4 ? ctzl(x) : ctz(x))
#define popcount(x) (sizeof(WordT) > 4 ? popcntl(x) : popcnt(x)) 


  /**
   *  Base class, general case.  It is a class invariant that Nw will be
   *  nonnegative.
   *
   *  See documentation for bitset.
  */
        template<size_t Nw>
        struct BaseBitset
        {

                /// 0 is the least significant word.
                WordT           M_w[Nw];

                constexpr BaseBitset() noexcept
                        : M_w() { }

                constexpr BaseBitset(unsigned long long val) noexcept
                {
                        for (size_t i = 0; i < Nw; ++i) {
                                M_w[i] = WordT(val);
                                val >>= 8 * sizeof(WordT);
                        }
                }


                /**
                   @brief Finds leading bit of value passed.

                   @param val is said value.

                   @return leading bit position, if any, else -1.
                 */
                static int
                        S_getMSB(const WordT& val) {
                        WordT onesTail = val | val >> 1;
                        onesTail |= onesTail >> 2;
                        onesTail |= onesTail >> 4;
                        onesTail |= onesTail >> 8;
                        onesTail |= onesTail >> (sizeof(WordT) > 2 ? 16 : 0);
                        onesTail |= onesTail >> (sizeof(WordT) > 4 ? 32 : 0);
                        return popcntl(onesTail) - 1;
                }

                /**
                   @brief increments the value passed.

                   @param[in, out] value to be incremented.

                   @return true iff (unsigned) increment precipitates a carry.
                 */
                static bool
                        S_increment(WordT &val) {
                        return ++val == 0;
                }

                /**
                   @brief decrements the value passed.

                   @param[in, out] value to be decremented.

                   @return true iff (unsigned) decrement precipitates a borrow.
                 */
                static bool
                        S_decrement(WordT &val) {
                        return val-- == 0;
                }


                /**
                   @brief Performs a two-operand addition on a single slot.

                   @param[in, out] accum accumulates the sum.

                   @param a is an addend.

                   @param b is an addend.

                   @param carry is true iff a carry was generated by the previous slot.

                   @return true iff a \it{full slot} carry precipitated.
                 */
                static bool
                        S_add(WordT &accum, const WordT& a, const WordT& b, bool carry) {
                        accum = a + b + (carry ? 1 : 0);
                        return accum < a;
                }


                /**
                   @brief Performs a two-operand addition on a single slot.

                   @param[in, out] accum accumulates the sum.

                   @param a is an addend.

                   @param carry is true iff a carry was generated by the previous slot.

                   @return true iff a \it{full slot} carry precipitated.
                 */
                static bool
                        S_add(WordT &accum, const WordT &a, bool carry) {
                        accum += a + (carry ? 1 : 0);
                        return accum < a;
                }


                /**
                   @brief Performs a two-operand subtraction on a single slot.

                   @param[in, out] accum accumulates the sum.

                   @param a is the minuend.

                   @param b is the (pre-borrow) subtrahend.

                   @param borrow inidcates whether a borrow applies from the previous slot.

                   @return true iff a \it{full slot} borrow precipitated.
                 */
                static bool
                        S_sub(WordT &accum,
                                const WordT& a,
                                const WordT& b,
                                const bool borrow) {
                        WordT subtrahend = b + (borrow ? 1 : 0);
                        accum = a - subtrahend;
                        return a < subtrahend;
                }

                /**
                   @brief Performs a single-operand subtraction.

                   @param[in, out] accum accumulates the sum.

                   @param a is the (pre-borrow) subtrahend.

                   @param borrow inidcates whether a borrow applies from the previous slot.

                   @return true iff a \it{full slot} borrow precipitated.
                 */
                static bool
                        S_sub(WordT &accum,
                                const WordT& a,
                                bool borrow) {
                        WordT subtrahend = a + (borrow ? 1 : 0);
                        borrow = accum < subtrahend;
                        accum -= subtrahend;
                        return borrow;
                }

                static constexpr size_t
                        S_whichword(size_t pos) noexcept
                {
                        return pos / UBB_BITS_PER_WORD;
                }

                static constexpr size_t
                        S_whichbyte(size_t pos) noexcept
                {
                        return (pos % UBB_BITS_PER_WORD) / __CHAR_BIT__;
                }

                static constexpr size_t
                        S_whichbit(size_t pos) noexcept
                {
                        return pos % UBB_BITS_PER_WORD;
                }

                static constexpr WordT
                        S_maskbit(size_t pos) noexcept
                {
                        return (static_cast<WordT>(1)) << S_whichbit(pos);
                }

                constexpr bool
                        M_testBit(size_t pos) const noexcept
                {
                        return ((this->M_getword(pos) & S_maskbit(pos))
                                != static_cast<WordT>(0));
                }

                WordT&
                        M_getword(size_t pos) noexcept
                {
                        return M_w[S_whichword(pos)];
                }

                constexpr WordT
                        M_getword(size_t pos) const noexcept
                {
                        return M_w[S_whichword(pos)];
                }

                //#if __cplusplus >= 201103L
                const WordT*
                        M_getdata() const noexcept
                {
                        return M_w;
                }
                //#endif

                WordT&
                        M_hiword() noexcept
                {
                        return M_w[Nw - 1];
                }

                constexpr WordT
                        M_hiword() const noexcept
                {
                        return M_w[Nw - 1];
                }

                void
                        M_do_and(const BaseBitset<Nw>& x) noexcept
                {
                        for (size_t i = 0; i < Nw; i++)
                                M_w[i] &= x.M_w[i];
                }

                void
                        M_do_or(const BaseBitset<Nw>& x) noexcept
                {
                        for (size_t i = 0; i < Nw; i++)
                                M_w[i] |= x.M_w[i];
                }

                void
                        M_do_xor(const BaseBitset<Nw>& x) noexcept
                {
                        for (size_t i = 0; i < Nw; i++)
                                M_w[i] ^= x.M_w[i];
                }

                void
                        M_do_left_shift(size_t shift) noexcept;

                void
                        M_do_right_shift(size_t shift) noexcept;

                void
                        M_do_flip() noexcept
                {
                        for (size_t i = 0; i < Nw; i++)
                                M_w[i] = ~M_w[i];
                }

                void
                        M_do_set() noexcept
                {
                        for (size_t i = 0; i < Nw; i++)
                                M_w[i] = ~static_cast<WordT>(0);
                }

                void
                        M_do_reset() noexcept
                {
                        clearmemory(M_w, 0, Nw * sizeof(WordT));
                }
                //{ __builtin_memset(M_w, 0, Nw * sizeof(WordT)); }

                bool
                        M_is_equal(const BaseBitset<Nw>& x) const noexcept
                {
                        for (size_t i = 0; i < Nw; ++i)
                                if (M_w[i] != x.M_w[i])
                                        return false;
                        return true;
                }

                /**
                   @brief Blocked unsigned less-than operation.

                   @param x is the comparison value.

                   @return true iff this is less than passed argument.
                 */
                bool M_is_less_than(const BaseBitset<Nw>& x) const noexcept
                {
                        for (int i = static_cast<int>(Nw) - 1; i >= 0; i--) {
                                if (M_w[i] > x.M_w[i])
                                        return false;
                                else if (M_w[i] < x.M_w[i])
                                        return true;
                        } // wraps around on equality.

                        return false;
                }


                /**
                   @brief Blocked unsigned greater-than operation.

                   @param x is the comparison value.

                   @return true iff this is greater than passed value.
                 */
                bool M_is_greater_than(const BaseBitset<Nw>& x) const noexcept
                {
                        for (int i = static_cast<int>(Nw) - 1; i >= 0; i--) {
                                if (M_w[i] < x.M_w[i])
                                        return false;
                                else if (M_w[i] > x.M_w[i])
                                        return true;
                        } // wraps around on equality.

                        return false;
                }


                /**
                   @brief Finds the position of the highest set bit.

                   @return leading bit position, if any, else -1.
                 */
                int M_getMSB() const noexcept {
                        int msb = -1;
                        for (int i = static_cast<int>(Nw) - 1; i >= 0; i--) {
                                msb = BaseBitset::S_getMSB(M_w[i]);
                                if (msb >= 0) {
                                        msb += i * UBB_BITS_PER_WORD;
                                        break;
                                }
                        }
                        return msb;
                }


                /**
                   @brief Increments underlying value by unity.

                   @return true iff carry in most-significant word.
                 */
                bool M_increment() noexcept {
                        for (size_t i = 0; i < Nw; i++) {
                                if (!BaseBitset::S_increment(M_w[i]))
                                        return false;
                        }

                        return true;
                }


                /**
                   @brief Decrements underlying value by unity.

                   @return true iff borrow generated in most-significant word.
                 */
                bool M_decrement() noexcept {
                        for (size_t i = 0; i < Nw; i++) {
                                if (!BaseBitset::S_decrement(M_w[i]))
                                        return false;
                        }

                        return true;
                }


                /**
                   @brief Slotwise addtion to this value.

                   @param a is an addend.

                   @param b is an addend.

                   @return true iff carry generated.
                 */
                template<size_t Nb>
                bool
                        M_do_add(const BaseBitset<UBB_WORDS(Nb)>& a,
                                const BaseBitset<UBB_WORDS(Nb)>& b)
                        noexcept {
                        bool carry = false;
                        for (size_t i = 0; i < UBB_WORDS(Nb); i++) {
                                carry = BaseBitset::S_add(M_w[i], a.M_w[i], b.M_w[i], carry);
                        }
                        return this->M_testBit(Nb);
                }


                /**
                   @brief Slotwise increment of this value.

                   @param a is an addend.

                   @return true iff carry generated.
                 */
                template<size_t Nb>
                bool
                        M_do_add(const BaseBitset<UBB_WORDS(Nb)>& a)
                        noexcept {
                        bool carry = false;
                        for (size_t i = 0; i < UBB_WORDS(Nb); i++) {
                                carry = BaseBitset::S_add(M_w[i], a.M_w[i], carry);
                        }
                        return this->M_testBit(Nb);
                }


                /**
                   @brief Slotwise subtraction into this value.

                   @param a is the minuend.

                   @param b is the subtrahend.
                 */
                template<size_t Nb>
                bool
                        M_do_sub(const BaseBitset<UBB_WORDS(Nb)>& a,
                                const BaseBitset<UBB_WORDS(Nb)>& b)
                        noexcept {
                        bool borrow = false;
                        for (size_t i = 0; i < UBB_WORDS(Nb); i++) {
                                borrow = BaseBitset::S_sub(M_w[i], a.M_w[i], b.M_w[i], borrow);
                        }

                        return borrow;
                }


                /**
                   @brief Slotwise decrement of this value.

                   @param a is the subtrahend.

                   @return true iff borrow generated.
                 */
                template<size_t Nb>
                bool
                        M_do_sub(const BaseBitset<UBB_WORDS(Nb)>& a)
                        noexcept {
                        bool borrow = false;
                        for (size_t i = 0; i < UBB_WORDS(Nb); i++) {
                                borrow = BaseBitset::S_sub(M_w[i], a.M_w[i], borrow);
                        }

                        return borrow;
                }


                template<size_t Nb>
                bool
                        M_are_all() const noexcept
                {
                        for (size_t i = 0; i < Nw - 1; i++)
                                if (M_w[i] != ~static_cast<WordT>(0))
                                        return false;
                        return M_hiword() == (~static_cast<WordT>(0)
                                >> (Nw * UBB_BITS_PER_WORD - Nb));
                }

                bool
                        M_is_any() const noexcept
                {
                        for (size_t i = 0; i < Nw; i++)
                                if (M_w[i] != static_cast<WordT>(0))
                                        return true;
                        return false;
                }

                size_t
                        M_do_count() const noexcept
                {
                        size_t result = 0;
                        for (size_t i = 0; i < Nw; i++)
                                result += popcntl(M_w[i]);
                        return result;
                }

                unsigned long
                        M_do_to_ulong() const;

                //#if __cplusplus >= 201103L
                unsigned long long
                        M_do_to_ullong() const;
                //#endif

                          // find first "on" bit
                size_t
                        M_do_find_first(size_t) const noexcept;

                // find the next "on" bit that follows "prev"
                size_t
                        M_do_find_next(size_t, size_t) const noexcept;
        };

        // Definitions of non-inline functions from BaseBitset.
        template<size_t Nw>
        void
                BaseBitset<Nw>::M_do_left_shift(size_t shift) noexcept
        {
                //      if (__builtin_expect(shift != 0, 1))
                if (shift != 0)
                {
                        const size_t wshift = shift / UBB_BITS_PER_WORD;
                        const size_t offset = shift % UBB_BITS_PER_WORD;

                        if (offset == 0)
                                for (size_t n = Nw - 1; n >= wshift; --n)
                                        M_w[n] = M_w[n - wshift];
                        else
                        {
                                const size_t sub_offset = (UBB_BITS_PER_WORD
                                        - offset);
                                for (size_t n = Nw - 1; n > wshift; --n)
                                        M_w[n] = ((M_w[n - wshift] << offset)
                                                | (M_w[n - wshift - 1] >> sub_offset));
                                M_w[wshift] = M_w[0] << offset;
                        }

                        std::fill(M_w + 0, M_w + wshift, static_cast<WordT>(0));
                }
        }

        template<size_t Nw>
        void
                BaseBitset<Nw>::M_do_right_shift(size_t shift) noexcept
        {
                //if (__builtin_expect(shift != 0, 1))
                if (shift != 0)
                {
                        const size_t wshift = shift / UBB_BITS_PER_WORD;
                        const size_t offset = shift % UBB_BITS_PER_WORD;
                        const size_t limit = Nw - wshift - 1;

                        if (offset == 0)
                                for (size_t n = 0; n <= limit; ++n)
                                        M_w[n] = M_w[n + wshift];
                        else
                        {
                                const size_t sub_offset = (UBB_BITS_PER_WORD
                                        - offset);
                                for (size_t n = 0; n < limit; ++n)
                                        M_w[n] = ((M_w[n + wshift] >> offset)
                                                | (M_w[n + wshift + 1] << sub_offset));
                                M_w[limit] = M_w[Nw - 1] >> offset;
                        }

                        std::fill(M_w + limit + 1, M_w + Nw, static_cast<WordT>(0));
                }
        }

        template<size_t Nw>
        unsigned long
                BaseBitset<Nw>::M_do_to_ulong() const
        {
                for (size_t i = 1; i < Nw; ++i)
                        if (M_w[i])
                                __throw_overflow_error(__N("BaseBitset::M_do_to_ulong"));
                return M_w[0];
        }

        //#if __cplusplus >= 201103L
        template<size_t Nw>
        unsigned long long
                BaseBitset<Nw>::M_do_to_ullong() const
        {
                const bool dw = sizeof(unsigned long long) > sizeof(unsigned long);
                for (size_t i = 1 + dw; i < Nw; ++i)
                        if (M_w[i])
                                __throw_overflow_error(__N("BaseBitset::M_do_to_ullong"));

                if (dw)
                        return M_w[0] + (static_cast<unsigned long long>(M_w[1])
                                << UBB_BITS_PER_WORD);
                return M_w[0];
        }
        //#endif

        template<size_t Nw>
        size_t
                BaseBitset<Nw>::
                M_do_find_first(size_t not_found) const noexcept
        {
                for (size_t i = 0; i < Nw; i++)
                {
                        WordT thisword = M_w[i];
                        if (thisword != static_cast<WordT>(0))
                                return (i * UBB_BITS_PER_WORD
                                        + leadzeroes(thisword));//__builtin_ctzl(thisword));
                }
                // not found, so return an indication of failure.
                return not_found;
        }

        template<size_t Nw>
        size_t
                BaseBitset<Nw>::
                M_do_find_next(size_t prev, size_t not_found) const noexcept
        {
                // make bound inclusive
                ++prev;

                // check out of bounds
                if (prev >= Nw * UBB_BITS_PER_WORD)
                        return not_found;

                // search first word
                size_t i = S_whichword(prev);
                WordT thisword = M_w[i];

                // mask off bits below bound
                thisword &= (~static_cast<WordT>(0)) << S_whichbit(prev);

                if (thisword != static_cast<WordT>(0))
                        return (i * UBB_BITS_PER_WORD + leadzeroes(thisword));//__builtin_ctzl(thisword));

                          // check subsequent words
                i++;
                for (; i < Nw; i++)
                {
                        thisword = M_w[i];
                        if (thisword != static_cast<WordT>(0))
                                return (i * UBB_BITS_PER_WORD + leadzeroes(thisword));//__builtin_ctzl(thisword));
                }
                // not found, so return an indication of failure.
                return not_found;
        } // end M_do_find_next

  /**
   *  Base class, specialization for a single word.
   *
   *  See documentation for bitset.
  */
        template<>
        struct BaseBitset<1>
        {
                WordT M_w;

                constexpr BaseBitset() noexcept
                        : M_w(0)
                { }

                //#if __cplusplus >= 201103L
                constexpr BaseBitset(unsigned long long val) noexcept
                        //#else
                        //      BaseBitset(unsigned long val)
                        //#endif
                        : M_w(WordT(val))
                { }

                static constexpr size_t
                        S_whichword(size_t pos) noexcept
                {
                        return pos / UBB_BITS_PER_WORD;
                }

                static constexpr size_t
                        S_whichbyte(size_t pos) noexcept
                {
                        return (pos % UBB_BITS_PER_WORD) / __CHAR_BIT__;
                }

                static constexpr size_t
                        S_whichbit(size_t pos) noexcept
                {
                        return pos % UBB_BITS_PER_WORD;
                }

                static constexpr WordT
                        S_maskbit(size_t pos) noexcept
                {
                        return (static_cast<WordT>(1)) << S_whichbit(pos);
                }

                constexpr bool
                        M_testBit(size_t pos) const noexcept {
                        return (M_w & S_maskbit(pos)) != static_cast<WordT>(0);
                }

                WordT&
                        M_getword(size_t) noexcept
                {
                        return M_w;
                }

                constexpr WordT
                        M_getword(size_t) const noexcept
                {
                        return M_w;
                }

                //#if __cplusplus >= 201103L
                const WordT*
                        M_getdata() const noexcept
                {
                        return &M_w;
                }
                //#endif

                WordT&
                        M_hiword() noexcept
                {
                        return M_w;
                }

                constexpr WordT
                        M_hiword() const noexcept
                {
                        return M_w;
                }

                void
                        M_do_and(const BaseBitset<1>& x) noexcept
                {
                        M_w &= x.M_w;
                }

                void
                        M_do_or(const BaseBitset<1>& x) noexcept
                {
                        M_w |= x.M_w;
                }

                void
                        M_do_xor(const BaseBitset<1>& x) noexcept
                {
                        M_w ^= x.M_w;
                }

                void
                        M_do_left_shift(size_t shift) noexcept
                {
                        M_w <<= shift;
                }

                void
                        M_do_right_shift(size_t shift) noexcept
                {
                        M_w >>= shift;
                }

                void
                        M_do_flip() noexcept
                {
                        M_w = ~M_w;
                }

                void
                        M_do_set() noexcept
                {
                        M_w = ~static_cast<WordT>(0);
                }

                void
                        M_do_reset() noexcept
                {
                        M_w = 0;
                }

                bool
                        M_is_equal(const BaseBitset<1>& x) const noexcept
                {
                        return M_w == x.M_w;
                }

                bool
                        M_is_less_than(const BaseBitset<1>& x) const noexcept
                {
                        return M_w < x.M_w;
                }

                bool
                        M_is_greater_than(const BaseBitset<1>& x) const noexcept
                {
                        return M_w > x.M_w;
                }

                /**
                   @brief Finds leading high bit position, if any, by counting the bits
                   in a mask packed to the right with ones.

                   @return position of leftmost set bit, if any, otherwise -1.
                 */
                int
                        M_getMSB() const noexcept {
                        WordT onesTail = M_w | M_w >> 1;
                        onesTail |= onesTail >> 2;
                        onesTail |= onesTail >> 4;
                        onesTail |= onesTail >> 8;
                        onesTail |= onesTail >> (sizeof(WordT) > 2 ? 16 : 0);
                        onesTail |= onesTail >> (sizeof(WordT) > 4 ? 32 : 0);

                        return popcntl(onesTail) - 1;
                }

                /**
                   @brief increments value by unity.

                   @return TRUE iff (unsigned) increment results in a carry.
                 */
                bool M_increment() noexcept {
                        return ++M_w == 0;
                }

                /**
                   @brief decrements value by unity.

                   @return TRUE iff (unsigned) decrement results in a borrow.
                 */
                bool M_decrement() noexcept {
                        return M_w-- == 0;
                }


                /**
                   @brief Addtion to this value.

                   @param a is the first addend.

                   @param b is the other addend.

                   @return true iff carry generated.
                 */
                template<size_t Nb>
                bool
                        M_do_add(const BaseBitset<1>& a,
                                const BaseBitset<1>& b) noexcept {
                        M_w = a.M_w + b.M_w;
                        return this->M_testBit(Nb);
                }


                /**
                   @brief Incremental addtion of this value.

                   @param a is the increment value.

                   @return true iff carry generated.
                 */
                template<size_t Nb>
                bool
                        M_do_add(const BaseBitset<1>& a) noexcept {
                        M_w += a.M_w;
                        return this->M_testBit(Nb);
                }


                /**
                   @brief Subtraction into underlying value.

                   @param a is the minuend.

                   @param b is the subtrahend.

                   @return true iff a borrow has been precipitated.
                 */
                template<size_t Nb>
                bool
                        M_do_sub(const BaseBitset<1>& a,
                                const BaseBitset<1>& b) noexcept {
                        M_w = a.M_w - b.M_w;
                        return a.M_w < b.M_w;
                }

                /**
                   @brief Subtraction from underlying value.

                   @param a is the subtrahend.

                   @return true iff a borrow has been precipitated.
                 */
                template<size_t Nb>
                bool
                        M_do_sub(const BaseBitset<1>& a) noexcept {
                        bool borrow = M_w < a.M_w;
                        M_w -= a.M_w;
                        return borrow;
                }


                template<size_t Nb>
                bool
                        M_are_all() const noexcept
                {
                        return M_w == (~static_cast<WordT>(0)
                                >> (UBB_BITS_PER_WORD - Nb));
                }

                bool
                        M_is_any() const noexcept
                {
                        return M_w != 0;
                }

                size_t
                        M_do_count() const noexcept
                {
                        return popcntl(M_w);
                }

                unsigned long
                        M_do_to_ulong() const noexcept
                {
                        return M_w;
                }

                //#if __cplusplus >= 201103L
                unsigned long long
                        M_do_to_ullong() const noexcept
                {
                        return M_w;
                }
                //#endif

                size_t
                        M_do_find_first(size_t not_found) const noexcept
                {
                        if (M_w != 0)
                                //return __builtin_ctzl(M_w);
                                return leadzeroes(M_w);
                        else
                                return not_found;
                }

                // find the next "on" bit that follows "prev"
                size_t
                        M_do_find_next(size_t prev, size_t not_found) const
                        noexcept
                {
                        ++prev;
                        if (prev >= ((size_t)UBB_BITS_PER_WORD))
                                return not_found;

                        WordT x = M_w >> prev;
                        if (x != 0)
                                // return __builtin_ctzl(x) + prev;
                                return leadzeroes(x) + prev;
                        else
                                return not_found;
                }
        };

        /**
         *  Base class, specialization for no storage (zero-length %bitset).
         *
         *  See documentation for bitset.
        */
        template<>
        struct BaseBitset<0>
        {
                constexpr BaseBitset() noexcept
                { }

                //#if __cplusplus >= 201103L
                constexpr BaseBitset(unsigned long long) noexcept
                        //#else
                        //     BaseBitset(unsigned long)
                        //#endif
                { }

                static constexpr size_t
                        S_whichword(size_t pos) noexcept
                {
                        return pos / UBB_BITS_PER_WORD;
                }

                static constexpr size_t
                        S_whichbyte(size_t pos) noexcept
                {
                        return (pos % UBB_BITS_PER_WORD) / __CHAR_BIT__;
                }

                static constexpr size_t
                        S_whichbit(size_t pos) noexcept
                {
                        return pos % UBB_BITS_PER_WORD;
                }

                static constexpr WordT
                        S_maskbit(size_t pos) noexcept
                {
                        return (static_cast<WordT>(1)) << S_whichbit(pos);
                }

                // This would normally give access to the data.  The bounds-checking
                // in the bitset class will prevent the user from getting this far,
                // but (1) it must still return an lvalue to compile, and (2) the
                // user might call Unchecked_set directly, in which case this /needs/
                // to fail.  Let's not penalize zero-length users unless they actually
                // make an unchecked call; all the memory ugliness is therefore
                // localized to this single should-never-get-this-far function.
                WordT&
                        M_getword(size_t) noexcept
                {
                        //__throw_out_of_range(__N("BaseBitset::M_getword"));
                        //throw std::runtime_error("bitblock out of range");
                        return *new WordT;
                }

                constexpr WordT
                        M_getword(size_t) const noexcept
                {
                        return 0;
                }

                constexpr WordT
                        M_hiword() const noexcept
                {
                        return 0;
                }

                void
                        M_do_and(const BaseBitset<0>&) noexcept
                { }

                void
                        M_do_or(const BaseBitset<0>&) noexcept
                { }

                void
                        M_do_xor(const BaseBitset<0>&) noexcept
                { }

                void
                        M_do_left_shift(size_t) noexcept
                { }

                void
                        M_do_right_shift(size_t) noexcept
                { }

                void
                        M_do_flip() noexcept
                { }

                void
                        M_do_set() noexcept
                { }

                void
                        M_do_reset() noexcept
                { }

                // Are all empty bitsets equal to each other?  Are they equal to
                // themselves?  How to compare a thing which has no state?  What is
                // the sound of one zero-length bitset clapping?
                bool
                        M_is_equal(const BaseBitset<0>&) const noexcept
                {
                        return true;
                }

                bool
                        M_is_less_than(const BaseBitset<0>&) const noexcept
                {
                        return true;
                }

                bool
                        M_is_greater_than(const BaseBitset<0>&) const noexcept
                {
                        return true;
                }

                int
                        M_getMSB() const noexcept {
                        return -1;
                }

                bool
                        M_increment() const noexcept {
                        return false;
                }


                bool
                        M_decrement() const noexcept {
                        return false;
                }


                template<size_t Nb>
                bool
                        M_are_all() const noexcept
                {
                        return true;
                }

                bool
                        M_is_any() const noexcept
                {
                        return false;
                }

                size_t
                        M_do_count() const noexcept
                {
                        return 0;
                }

                unsigned long
                        M_do_to_ulong() const noexcept
                {
                        return 0;
                }

                //#if __cplusplus >= 201103L
                unsigned long long
                        M_do_to_ullong() const noexcept
                {
                        return 0;
                }
                //#endif

                          // Normally "not found" is the size, but that could also be
                          // misinterpreted as an index in this corner case.  Oh well.
                size_t
                        M_do_find_first(size_t) const noexcept
                {
                        return 0;
                }

                size_t
                        M_do_find_next(size_t, size_t) const noexcept
                {
                        return 0;
                }
        };


        // Helper class to zero out the unused high-order bits in the highest word.
        template<size_t _Extrabits>
        struct Sanitize
        {
                static void
                        S_do_sanitize(WordT& val) noexcept
                {
                        val &= ~((~static_cast<WordT>(0)) << _Extrabits);
                }
        };

        template<>
        struct Sanitize<0>
        {
                static void
                        S_do_sanitize(WordT) noexcept { }
        };

        //#if __cplusplus >= 201103L
        template<size_t Nb, bool = (Nb < UBB_BITS_PER_ULL)>
                struct Sanitize_val
                {
                        static constexpr unsigned long long
                                S_do_sanitize_val(unsigned long long val)
                        {
                                return val;
                        }
                };

                template<size_t Nb>
                struct Sanitize_val<Nb, true>
                {
                        static constexpr unsigned long long
                                S_do_sanitize_val(unsigned long long val)
                        {
                                return val & ~((~static_cast<unsigned long long>(0)) << Nb);
                        }
                };
                //#endif

                  /**
                   *  @brief The %bitset class represents a @e fixed-size sequence of bits.
                   *  @ingroup utilities
                   *
                   *  (Note that %bitset does @e not meet the formal requirements of a
                   *  <a href="tables.html#65">container</a>.  Mainly, it lacks iterators.)
                   *
                   *  The template argument, @a Nb, may be any non-negative number,
                   *  specifying the number of bits (e.g., "0", "12", "1024*1024").
                   *
                   *  In the general unoptimized case, storage is allocated in word-sized
                   *  blocks.  Let B be the number of bits in a word, then (Nb+(B-1))/B
                   *  words will be used for storage.  B - Nb%B bits are unused.  (They are
                   *  the high-order bits in the highest word.)  It is a class invariant
                   *  that those unused bits are always zero.
                   *
                   *  If you think of %bitset as <em>a simple array of bits</em>, be
                   *  aware that your mental picture is reversed: a %bitset behaves
                   *  the same way as bits in integers do, with the bit at index 0 in
                   *  the <em>least significant / right-hand</em> position, and the bit at
                   *  index Nb-1 in the <em>most significant / left-hand</em> position.
                   *  Thus, unlike other containers, a %bitset's index <em>counts from
                   *  right to left</em>, to put it very loosely.
                   *
                   *  This behavior is preserved when translating to and from strings.  For
                   *  example, the first line of the following program probably prints
                   *  <em>b(&apos;a&apos;) is 0001100001</em> on a modern ASCII system.
                   *
                   *  @code
                   *     #include <bitset>
                   *     #include <iostream>
                   *     #include <sstream>
                   *
                   *     using namespace std;
                   *
                   *     int main()
                   *     {
                   *         long         a = 'a';
                   *         bitset<10>   b(a);
                   *
                   *         cout << "b('a') is " << b << endl;
                   *
                   *         ostringstream s;
                   *         s << b;
                   *         string  str = s.str();
                   *         cout << "index 3 in the string is " << str[3] << " but\n"
                   *              << "index 3 in the bitset is " << b[3] << endl;
                   *     }
                   *  @endcode
                   *
                   *  Also see:
                   *  https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_containers.html
                   *  for a description of extensions.
                   *
                   *  Most of the actual code isn't contained in %bitset<> itself, but in the
                   *  base class BaseBitset.  The base class works with whole words, not with
                   *  individual bits.  This allows us to specialize BaseBitset for the
                   *  important special case where the %bitset is only a single word.
                   *
                   *  Extra confusion can result due to the fact that the storage for
                   *  BaseBitset @e is a regular array, and is indexed as such.  This is
                   *  carefully encapsulated.
                  */
                template<size_t Nb>
                class bitset
                        : public BaseBitset<UBB_WORDS(Nb)>
                {
                private:
                        typedef BaseBitset<UBB_WORDS(Nb)> Base;

                        template<class _CharT, class _Traits, class _Alloc>
                        void
                                M_check_initial_position(const std::basic_string<_CharT, _Traits, _Alloc>& s, size_t position) const
                        {
                                if (position > s.size()) {
#ifdef __GNUC__
                                        __throw_out_of_range_fmt(__N("bitset::bitset: position "
                                                "(which is %zu) > s.size() "
                                                "(which is %zu)"),
                                                position, s.size());
#elif defined(_MSC_VER)
                                        char msg[256];
                                        sprintf(msg, "bitset::bitset: position %zu > s.size() (%zu)", position, s.size());
                                        throw std::runtime_error(msg);
#endif
                                }

                        }

                        void M_check(size_t position, const char *s) const
                        {
                                if (position >= Nb) {
#ifdef __GNUC__
                                        __throw_out_of_range_fmt(__N("%s: position (which is %zu) "
                                                ">= Nb (which is %zu)"),
                                                s, position, Nb);
#elif defined(_MSC_VER)
                                        char msg[256];
                                        sprintf(msg, "%s: position (which is %zu) >= Nb (which is %zu)", s, position, Nb);
                                        throw std::runtime_error(msg);
#endif
                                }
                        }

                        void
                                M_do_sanitize() noexcept
                        {
                                typedef Sanitize<Nb % UBB_BITS_PER_WORD> sanitize_type;
                                sanitize_type::S_do_sanitize(this->M_hiword());
                        }

                        //#if __cplusplus >= 201103L
                        friend struct std::hash<bitset>;
                        //#endif

                public:
                        /**
                         *  This encapsulates the concept of a single bit.  An instance of this
                         *  class is a proxy for an actual bit; this way the individual bit
                         *  operations are done as faster word-size bitwise instructions.
                         *
                         *  Most users will never need to use this class directly; conversions
                         *  to and from bool are automatic and should be transparent.  Overloaded
                         *  operators help to preserve the illusion.
                         *
                         *  (On a typical system, this <em>bit %reference</em> is 64
                         *  times the size of an actual bit.  Ha.)
                         */
                        class reference
                        {
                                friend class bitset;

                                WordT*  M_wp;
                                size_t  M_bpos;

                                // left undefined
                                reference();

                        public:
                                reference(bitset& b, size_t pos) noexcept
                                {
                                        M_wp = &b.M_getword(pos);
                                        M_bpos = Base::S_whichbit(pos);
                                }

                                ~reference() noexcept
                                { }

                                // For b[i] = x;
                                reference&
                                        operator=(bool x) noexcept
                                {
                                        if (x)
                                                *M_wp |= Base::S_maskbit(M_bpos);
                                        else
                                                *M_wp &= ~Base::S_maskbit(M_bpos);
                                        return *this;
                                }

                                // For b[i] = b[__j];
                                reference&
                                        operator=(const reference& j) noexcept
                                {
                                        if ((*(j.M_wp) & Base::S_maskbit(j.M_bpos)))
                                                *M_wp |= Base::S_maskbit(M_bpos);
                                        else
                                                *M_wp &= ~Base::S_maskbit(M_bpos);
                                        return *this;
                                }

                                // Flips the bit
                                bool
                                        operator~() const noexcept
                                {
                                        return (*(M_wp)& Base::S_maskbit(M_bpos)) == 0;
                                }

                                // For x = b[i];
                                operator bool() const noexcept
                                {
                                        return (*(M_wp)& Base::S_maskbit(M_bpos)) != 0;
                                }

                                // For b[i].flip();
                                reference&
                                        flip() noexcept
                                {
                                        *M_wp ^= Base::S_maskbit(M_bpos);
                                        return *this;
                                }
                        };
                        friend class reference;

                        // 23.3.5.1 constructors:
                        /// All bits set to zero.
                        constexpr bitset() noexcept
                        { }

                        /// Initial bits bitwise-copied from a single word (others set to zero).
          //#if __cplusplus >= 201103L
                        constexpr bitset(unsigned long long val) noexcept
                                : Base(Sanitize_val<Nb>::S_do_sanitize_val(val)) { }
                        //#else
                        //      bitset(unsigned long val)
                        //      : Base(val)
                        //      { M_do_sanitize(); }
                        //#endif

                                  /**
                                   *  Use a subset of a string.
                                   *  @param  s  A string of @a 0 and @a 1 characters.
                                   *  @param  position  Index of the first character in @a s to use;
                                   *                    defaults to zero.
                                   *  @throw  std::out_of_range  If @a pos is bigger the size of @a s.
                                   *  @throw  std::invalid_argument  If a character appears in the string
                                   *                                 which is neither @a 0 nor @a 1.
                                   */
                        template<class _CharT, class _Traits, class _Alloc>
                        explicit
                                bitset(const std::basic_string<_CharT, _Traits, _Alloc>& s,
                                        size_t position = 0)
                                : Base()
                        {
                                M_check_initial_position(s, position);
                                M_copy_from_string(s, position,
                                        std::basic_string<_CharT, _Traits, _Alloc>::npos,
                                        _CharT('0'), _CharT('1'));
                        }

                        /**
                         *  Use a subset of a string.
                         *  @param  s  A string of @a 0 and @a 1 characters.
                         *  @param  position  Index of the first character in @a s to use.
                         *  @param  n    The number of characters to copy.
                         *  @throw std::out_of_range If @a position is bigger the size
                         *  of @a s.
                         *  @throw  std::invalid_argument  If a character appears in the string
                         *                                 which is neither @a 0 nor @a 1.
                         */
                        template<class _CharT, class _Traits, class _Alloc>
                        bitset(const std::basic_string<_CharT, _Traits, _Alloc>& s,
                                size_t position, size_t n)
                                : Base()
                        {
                                M_check_initial_position(s, position);
                                M_copy_from_string(s, position, n, _CharT('0'), _CharT('1'));
                        }

                        // _GLIBCXX_RESOLVE_LIB_DEFECTS
                        // 396. what are characters zero and one.
                        template<class _CharT, class _Traits, class _Alloc>
                        bitset(const std::basic_string<_CharT, _Traits, _Alloc>& s,
                                size_t position, size_t n,
                                _CharT zero, _CharT one = _CharT('1'))
                                : Base()
                        {
                                M_check_initial_position(s, position);
                                M_copy_from_string(s, position, n, zero, one);
                        }

                        //#if __cplusplus >= 201103L
                                  /**
                                   *  Construct from a character %array.
                                   *  @param  str  An %array of characters @a zero and @a one.
                                   *  @param  n    The number of characters to use.
                                   *  @param  zero The character corresponding to the value 0.
                                   *  @param  one  The character corresponding to the value 1.
                                   *  @throw  std::invalid_argument If a character appears in the string
                                   *                                which is neither @a zero nor @a one.
                                   */
                        template<typename _CharT>
                        explicit
                                bitset(const _CharT* str,
                                        typename std::basic_string<_CharT>::size_type n
                                        = std::basic_string<_CharT>::npos,
                                        _CharT zero = _CharT('0'), _CharT one = _CharT('1'))
                                : Base()
                        {
                                if (!str)
                                        __throw_logic_error(__N("bitset::bitset(const _CharT*, ...)"));

                                if (n == std::basic_string<_CharT>::npos)
                                        n = std::char_traits<_CharT>::length(str);
                                M_copy_from_ptr<_CharT, std::char_traits<_CharT>>(str, n, 0,
                                        n, zero,
                                        one);
                        }
                        //#endif

                                  // 23.3.5.2 bitset operations:
                                  //@{
                                  /**
                                   *  Operations on bitsets.
                                   *  @param  rhs  A same-sized bitset.
                                   *
                                   *  These should be self-explanatory.
                                   */
                        bitset<Nb>&
                                operator&=(const bitset<Nb>& rhs) noexcept
                        {
                                this->M_do_and(rhs);
                                return *this;
                        }

                        bitset<Nb>&
                                operator|=(const bitset<Nb>& rhs) noexcept
                        {
                                this->M_do_or(rhs);
                                return *this;
                        }

                        bitset<Nb>&
                                operator^=(const bitset<Nb>& rhs) noexcept
                        {
                                this->M_do_xor(rhs);
                                return *this;
                        }
                        //@}

                        //@{
                        /**
                         *  Operations on bitsets.
                         *  @param  position  The number of places to shift.
                         *
                         *  These should be self-explanatory.
                         */
                        bitset<Nb>&
                                operator<<=(size_t position) noexcept
                        {
                                //      if (__builtin_expect(position < Nb, 1))
                                if (position < Nb)
                                {
                                        this->M_do_left_shift(position);
                                        this->M_do_sanitize();
                                }
                                else
                                        this->M_do_reset();
                                return *this;
                        }

                        bitset<Nb>&
                                operator>>=(size_t position) noexcept
                        {
                                //      if (__builtin_expect(position < Nb, 1))
                                if (position < Nb)
                                {
                                        this->M_do_right_shift(position);
                                        this->M_do_sanitize();
                                }
                                else
                                        this->M_do_reset();
                                return *this;
                        }
                        //@}

                        //@{
                        /**
                         *  These versions of single-bit set, reset, flip, and test are
                         *  extensions from the SGI version.  They do no range checking.
                         *  @ingroup SGIextensions
                         */
                        bitset<Nb>&
                                Unchecked_set(size_t pos) noexcept
                        {
                                this->M_getword(pos) |= Base::S_maskbit(pos);
                                return *this;
                        }

                        bitset<Nb>&
                                Unchecked_set(size_t pos, int val) noexcept
                        {
                                if (val)
                                        this->M_getword(pos) |= Base::S_maskbit(pos);
                                else
                                        this->M_getword(pos) &= ~Base::S_maskbit(pos);
                                return *this;
                        }

                        bitset<Nb>&
                                Unchecked_reset(size_t pos) noexcept
                        {
                                this->M_getword(pos) &= ~Base::S_maskbit(pos);
                                return *this;
                        }

                        bitset<Nb>&
                                Unchecked_flip(size_t pos) noexcept
                        {
                                this->M_getword(pos) ^= Base::S_maskbit(pos);
                                return *this;
                        }

                        constexpr bool
                                Unchecked_test(size_t pos) const noexcept
                        {
                                return ((this->M_getword(pos) & Base::S_maskbit(pos))
                                        != static_cast<WordT>(0));
                        }
                        //@}

                        // Set, reset, and flip.
                        /**
                         *  @brief Sets every bit to true.
                         */
                        bitset<Nb>&
                                set() noexcept
                        {
                                this->M_do_set();
                                this->M_do_sanitize();
                                return *this;
                        }

                        /**
                         *  @brief Sets a given bit to a particular value.
                         *  @param  position  The index of the bit.
                         *  @param  val  Either true or false, defaults to true.
                         *  @throw  std::out_of_range  If @a pos is bigger the size of the %set.
                         */
                        bitset<Nb>&
                                set(size_t position, bool val = true)
                        {
                                //              this->M_check(position, __N("bitset::set"));
                                this->M_check(position, "bitset::set");

                                return Unchecked_set(position, val);
                        }

                        /**
                         *  @brief Sets every bit to false.
                         */
                        bitset<Nb>&
                                reset() noexcept
                        {
                                this->M_do_reset();
                                return *this;
                        }

                        /**
                         *  @brief Sets a given bit to false.
                         *  @param  position  The index of the bit.
                         *  @throw  std::out_of_range  If @a pos is bigger the size of the %set.
                         *
                         *  Same as writing @c set(pos,false).
                         */
                        bitset<Nb>&
                                reset(size_t position)
                        {
                                //                this->M_check(position, __N("bitset::reset"));
                                this->M_check(position, "bitset::reset");
                                return Unchecked_reset(position);
                        }

                        /**
                         *  @brief Toggles every bit to its opposite value.
                         */
                        bitset<Nb>&
                                flip() noexcept
                        {
                                this->M_do_flip();
                                this->M_do_sanitize();
                                return *this;
                        }

                        /**
                         *  @brief Toggles a given bit to its opposite value.
                         *  @param  position  The index of the bit.
                         *  @throw  std::out_of_range  If @a pos is bigger the size of the %set.
                         */
                        bitset<Nb>&
                                flip(size_t position)
                        {
                                //                this->M_check(position, __N("bitset::flip"));
                                this->M_check(position, "bitset::flip");
                                return Unchecked_flip(position);
                        }

                        /// See the no-argument flip().
                        bitset<Nb>
                                operator~() const noexcept
                        {
                                return bitset<Nb>(*this).flip();
                        }

                        //@{
                        /**
                         *  @brief  Array-indexing support.
                         *  @param  position  Index into the %bitset.
                         *  @return A bool for a <em>const %bitset</em>.  For non-const
                         *           bitsets, an instance of the reference proxy class.
                         *  @note  These operators do no range checking and throw no exceptions,
                         *         as required by DR 11 to the standard.
                         *
                         *  _GLIBCXX_RESOLVE_LIB_DEFECTS Note that this implementation already
                         *  resolves DR 11 (items 1 and 2), but does not do the range-checking
                         *  required by that DR's resolution.  -pme
                         *  The DR has since been changed:  range-checking is a precondition
                         *  (users' responsibility), and these functions must not throw.  -pme
                         */
                        reference
                                operator[](size_t position)
                        {
                                return reference(*this, position);
                        }

                        constexpr bool
                                operator[](size_t position) const
                        {
                                return Unchecked_test(position);
                        }
                        //@}

                        /**
                         *  @brief Returns a numerical interpretation of the %bitset.
                         *  @return  The integral equivalent of the bits.
                         *  @throw  std::overflow_error  If there are too many bits to be
                         *                               represented in an @c unsigned @c long.
                         */
                        unsigned long
                                to_ulong() const
                        {
                                return this->M_do_to_ulong();
                        }

                        //#if __cplusplus >= 201103L
                        unsigned long long
                                to_ullong() const
                        {
                                return this->M_do_to_ullong();
                        }
                        //#endif

                                  /**
                                   *  @brief Returns a character interpretation of the %bitset.
                                   *  @return  The string equivalent of the bits.
                                   *
                                   *  Note the ordering of the bits:  decreasing character positions
                                   *  correspond to increasing bit positions (see the main class notes for
                                   *  an example).
                                   */
                        template<class _CharT, class _Traits, class _Alloc>
                        std::basic_string<_CharT, _Traits, _Alloc>
                                to_string() const
                        {
                                std::basic_string<_CharT, _Traits, _Alloc> result;
                                M_copy_to_string(result, _CharT('0'), _CharT('1'));
                                return result;
                        }

                        // _GLIBCXX_RESOLVE_LIB_DEFECTS
                        // 396. what are characters zero and one.
                        template<class _CharT, class _Traits, class _Alloc>
                        std::basic_string<_CharT, _Traits, _Alloc>
                                to_string(_CharT zero, _CharT one = _CharT('1')) const
                        {
                                std::basic_string<_CharT, _Traits, _Alloc> result;
                                M_copy_to_string(result, zero, one);
                                return result;
                        }

                        // _GLIBCXX_RESOLVE_LIB_DEFECTS
                        // 434. bitset::to_string() hard to use.
                        template<class _CharT, class _Traits>
                        std::basic_string<_CharT, _Traits, std::allocator<_CharT> >
                                to_string() const
                        {
                                return to_string<_CharT, _Traits, std::allocator<_CharT> >();
                        }

                        // _GLIBCXX_RESOLVE_LIB_DEFECTS
                        // 853. to_string needs updating with zero and one.
                        template<class _CharT, class _Traits>
                        std::basic_string<_CharT, _Traits, std::allocator<_CharT> >
                                to_string(_CharT zero, _CharT one = _CharT('1')) const
                        {
                                return to_string<_CharT, _Traits,
                                        std::allocator<_CharT> >(zero, one);
                        }

                        template<class _CharT>
                        std::basic_string<_CharT, std::char_traits<_CharT>,
                                std::allocator<_CharT> >
                                to_string() const
                        {
                                return to_string<_CharT, std::char_traits<_CharT>,
                                        std::allocator<_CharT> >();
                        }

                        template<class _CharT>
                        std::basic_string<_CharT, std::char_traits<_CharT>,
                                std::allocator<_CharT> >
                                to_string(_CharT zero, _CharT one = _CharT('1')) const
                        {
                                return to_string<_CharT, std::char_traits<_CharT>,
                                        std::allocator<_CharT> >(zero, one);
                        }

                        std::basic_string<char, std::char_traits<char>, std::allocator<char> >
                                to_string() const
                        {
                                return to_string<char, std::char_traits<char>,
                                        std::allocator<char> >();
                        }

                        std::basic_string<char, std::char_traits<char>, std::allocator<char> >
                                to_string(char zero, char one = '1') const
                        {
                                return to_string<char, std::char_traits<char>,
                                        std::allocator<char> >(zero, one);
                        }

                        // Helper functions for string operations.
                        template<class _CharT, class _Traits>
                        void
                                M_copy_from_ptr(const _CharT*, size_t, size_t, size_t,
                                        _CharT, _CharT);

                        template<class _CharT, class _Traits, class _Alloc>
                        void
                                M_copy_from_string(const std::basic_string<_CharT,
                                        _Traits, _Alloc>& s, size_t pos, size_t n,
                                        _CharT zero, _CharT one)
                        {
                                M_copy_from_ptr<_CharT, _Traits>(s.data(), s.size(), pos, n,
                                        zero, one);
                        }

                        template<class _CharT, class _Traits, class _Alloc>
                        void
                                M_copy_to_string(std::basic_string<_CharT, _Traits, _Alloc>&,
                                        _CharT, _CharT) const;

                        // NB: Backward compat.
                        template<class _CharT, class _Traits, class _Alloc>
                        void
                                M_copy_from_string(const std::basic_string<_CharT,
                                        _Traits, _Alloc>& s, size_t pos, size_t n)
                        {
                                M_copy_from_string(s, pos, n, _CharT('0'), _CharT('1'));
                        }

                        template<class _CharT, class _Traits, class _Alloc>
                        void
                                M_copy_to_string(std::basic_string<_CharT, _Traits, _Alloc>& s) const
                        {
                                M_copy_to_string(s, _CharT('0'), _CharT('1'));
                        }

                        /// Returns the number of bits which are set.
                        size_t
                                count() const noexcept
                        {
                                return this->M_do_count();
                        }

                        /// Returns the total number of bits.
                        constexpr size_t
                                size() const noexcept
                        {
                                return Nb;
                        }

                        //@{
                        /// These comparisons for equality/inequality are, well, @e bitwise.
                        bool
                                operator==(const bitset<Nb>& rhs) const noexcept
                        {
                                return this->M_is_equal(rhs);
                        }

                        bool
                                operator!=(const bitset<Nb>& rhs) const noexcept
                        {
                                return !this->M_is_equal(rhs);
                        }
                        //@}

                        bool
                                operator<(const bitset<Nb>& rhs) const noexcept
                        {
                                return this->M_is_less_than(rhs);
                        }

                        bool
                                operator<=(const bitset<Nb>& rhs) const noexcept
                        {
                                return !this->M_is_greater_than(rhs);
                        }

                        bool
                                operator>(const bitset<Nb>& rhs) const noexcept
                        {
                                return this->M_is_greater_than(rhs);
                        }

                        bool
                                operator>=(const bitset<Nb>& rhs) const noexcept
                        {
                                return !this->M_is_less_than(rhs);
                        }


                        int
                                getMSB() const noexcept {
                                return this->M_getMSB();
                        }

                        bool
                                increment() noexcept {
                                bool carry = this->M_increment();
                                this->M_do_sanitize();
                                return carry;
                        }


                        bool
                                decrement() noexcept {
                                bool borrow = this->M_decrement();
                                this->M_do_sanitize();
                                return borrow;
                        }


                        template<size_t NbOpnd>
                        bool
                                add(const bitset<NbOpnd>& a,
                                        const bitset<NbOpnd>& b) noexcept {
                                return this->template M_do_add<NbOpnd>(a, b);
                        }


                        /**
                           @brief Incremental add.
                         */
                        template<size_t NbOpnd>
                        bool
                                add(const bitset<NbOpnd>& a) noexcept {
                                return this->template M_do_add<NbOpnd>(a);
                        }


                        /**
                         */
                        template<size_t NbOpnd>
                        bool
                                sub(const bitset<NbOpnd>& a,
                                        const bitset<NbOpnd>& b) noexcept {
                                bool borrow = this->template M_do_sub<NbOpnd>(a, b);
                                Unchecked_set(NbOpnd, borrow);
                                M_do_sanitize();

                                return borrow;
                        }

                        template<size_t NbOpnd>
                        bool
                                sub(const bitset<NbOpnd>& a) noexcept {
                                bool borrow = this->template M_do_sub<NbOpnd>(a);
                                Unchecked_set(NbOpnd, borrow);
                                M_do_sanitize();

                                return borrow;
                        }


                        /**
                         *  @brief Tests the value of a bit.
                         *  @param  position  The index of a bit.
                         *  @return  The value at @a pos.
                         *  @throw  std::out_of_range  If @a pos is bigger the size of the %set.
                         */
                        bool
                                test(size_t position) const
                        {
                                //                this->M_check(position, __N("bitset::test"));
                                this->M_check(position, "bitset::test");
                                return Unchecked_test(position);
                        }

                        // _GLIBCXX_RESOLVE_LIB_DEFECTS
                        // DR 693. std::bitset::all() missing.
                        /**
                         *  @brief Tests whether all the bits are on.
                         *  @return  True if all the bits are set.
                         */
                        bool
                                all() const noexcept
                        {
                                return this->template M_are_all<Nb>();
                        }

                        /**
                         *  @brief Tests whether any of the bits are on.
                         *  @return  True if at least one bit is set.
                         */
                        bool
                                any() const noexcept
                        {
                                return this->M_is_any();
                        }

                        /**
                         *  @brief Tests whether any of the bits are on.
                         *  @return  True if none of the bits are set.
                         */
                        bool
                                none() const noexcept
                        {
                                return !this->M_is_any();
                        }

                        //@{
                        /// Self-explanatory.
                        bitset<Nb>
                                operator<<(size_t position) const noexcept
                        {
                                return bitset<Nb>(*this) <<= position;
                        }

                        bitset<Nb>
                                operator>>(size_t position) const noexcept
                        {
                                return bitset<Nb>(*this) >>= position;
                        }
                        //@}

                        /**
                         *  @brief  Finds the index of the first "on" bit.
                         *  @return  The index of the first bit set, or size() if not found.
                         *  @ingroup SGIextensions
                         *  @sa  _Find_next
                         */
                        size_t
                                _Find_first() const noexcept
                        {
                                return this->M_do_find_first(Nb);
                        }

                        /**
                         *  @brief  Finds the index of the next "on" bit after prev.
                         *  @return  The index of the next bit set, or size() if not found.
                         *  @param  prev  Where to start searching.
                         *  @ingroup SGIextensions
                         *  @sa  _Find_first
                         */
                        size_t
                                _Find_next(size_t prev) const noexcept
                        {
                                return this->M_do_find_next(prev, Nb);
                        }
                };

                // Definitions of non-inline member functions.
                template<size_t Nb>
                template<class _CharT, class _Traits>
                void
                        bitset<Nb>::
                        M_copy_from_ptr(const _CharT* s, size_t len,
                                size_t pos, size_t n, _CharT zero, _CharT one)
                {
                        reset();
                        const size_t nbits = std::min(Nb, std::min(n, size_t(len - pos)));
                        for (size_t i = nbits; i > 0; --i)
                        {
                                const _CharT c = s[pos + nbits - i];
                                if (_Traits::eq(c, zero))
                                        ;
                                else if (_Traits::eq(c, one))
                                        Unchecked_set(i - 1);
                                else
                                        __throw_invalid_argument(__N("bitset::M_copy_from_ptr"));
                        }
                }

                template<size_t Nb>
                template<class _CharT, class _Traits, class _Alloc>
                void
                        bitset<Nb>::
                        M_copy_to_string(std::basic_string<_CharT, _Traits, _Alloc>& s,
                                _CharT zero, _CharT one) const
                {
                        s.assign(Nb, zero);
                        for (size_t i = Nb; i > 0; --i)
                                if (Unchecked_test(i - 1))
                                        _Traits::assign(s[Nb - i], one);
                }

                // 23.3.5.3 bitset operations:
                //@{
                /**
                 *  @brief  Global bitwise operations on bitsets.
                 *  @param  x  A bitset.
                 *  @param  y  A bitset of the same size as @a x.
                 *  @return  A new bitset.
                 *
                 *  These should be self-explanatory.
                */
                template<size_t Nb>
                inline bitset<Nb>
                        operator&(const bitset<Nb>& x, const bitset<Nb>& y) noexcept
                {
                        bitset<Nb> result(x);
                        result &= y;
                        return result;
                }

                template<size_t Nb>
                inline bitset<Nb>
                        operator|(const bitset<Nb>& x, const bitset<Nb>& y) noexcept
                {
                        bitset<Nb> result(x);
                        result |= y;
                        return result;
                }

                template <size_t Nb>
                inline bitset<Nb>
                        operator^(const bitset<Nb>& x, const bitset<Nb>& y) noexcept
                {
                        bitset<Nb> result(x);
                        result ^= y;
                        return result;
                }
                //@}

                //@{
                /**
                 *  @brief Global I/O operators for bitsets.
                 *
                 *  Direct I/O between streams and bitsets is supported.  Output is
                 *  straightforward.  Input will skip whitespace, only accept @a 0 and @a 1
                 *  characters, and will only extract as many digits as the %bitset will
                 *  hold.
                */
                template<class _CharT, class _Traits, size_t Nb>
                std::basic_istream<_CharT, _Traits>&
                        operator>>(std::basic_istream<_CharT, _Traits>& __is, bitset<Nb>& x)
                {
                        typedef typename _Traits::char_type          char_type;
                        typedef std::basic_istream<_CharT, _Traits>  __istream_type;
                        typedef typename __istream_type::ios_base    __ios_base;

                        std::basic_string<_CharT, _Traits> tmp;
                        tmp.reserve(Nb);

                        // _GLIBCXX_RESOLVE_LIB_DEFECTS
                        // 303. Bitset input operator underspecified
                        const char_type zero = __is.widen('0');
                        const char_type one = __is.widen('1');

                        typename __ios_base::iostate state = __ios_base::goodbit;
                        typename __istream_type::sentry sentry(__is);
                        if (sentry)
                        {
                                __try
                                {
                                        for (size_t i = Nb; i > 0; --i)
                                        {
                                                static typename _Traits::int_type eof = _Traits::eof();

                                                typename _Traits::int_type c1 = __is.rdbuf()->sbumpc();
                                                if (_Traits::eq_int_type(c1, eof))
                                                {
                                                        state |= __ios_base::eofbit;
                                                        break;
                                                }
                                                else
                                                {
                                                        const char_type c2 = _Traits::to_char_type(c1);
                                                        if (_Traits::eq(c2, zero))
                                                                tmp.push_back(zero);
                                                        else if (_Traits::eq(c2, one))
                                                                tmp.push_back(one);
                                                        else if (_Traits::
                                                                eq_int_type(__is.rdbuf()->sputbackc(c2),
                                                                        eof))
                                                        {
                                                                state |= __ios_base::failbit;
                                                                break;
                                                        }
                                                }
                                        }
                                }
                                __catch(__cxxabiv1::__forced_unwind&)
                                {
                                        __is.M_setstate(__ios_base::badbit);
                                        __throw_exception_again;
                                }
                                __catch(...)
                                {
                                        __is.M_setstate(__ios_base::badbit);
                                }
                        }

                        if (tmp.empty() && Nb)
                                state |= __ios_base::failbit;
                        else
                                x.M_copy_from_string(tmp, static_cast<size_t>(0), Nb,
                                        zero, one);
                        if (state)
                                __is.setstate(state);
                        return __is;
                }

                template <class _CharT, class _Traits, size_t Nb>
                std::basic_ostream<_CharT, _Traits>&
                        operator<<(std::basic_ostream<_CharT, _Traits>& os,
                                const bitset<Nb>& x)
                {
                        std::basic_string<_CharT, _Traits> tmp;

                        // _GLIBCXX_RESOLVE_LIB_DEFECTS
                        // 396. what are characters zero and one.
                        const ctype<_CharT>& ct = use_facet<ctype<_CharT> >(os.getloc());
                        x.M_copy_to_string(tmp, ct.widen('0'), ct.widen('1'));
                        return os << tmp;
                }
                //@}

#undef UBB_WORDS
#undef UBB_BITS_PER_WORD
#undef UBB_BITS_PER_ULL

} // namespace bitset

//#if __cplusplus >= 201103L

#ifdef VERSION_SPECIALIZATION
  //  _GLIBCXX_BEGIN_NAMESPACE_VERSION
namespace bitset2 {
        // DR 1182.
        /// std::hash specialization for bitset.
        template<size_t Nb>
        struct hash<universal_bitblock::bitset<Nb>>
                : public __hash_base<size_t, universal_bitblock::bitset<Nb>>
        {
                size_t
                        operator()(const universal_bitblock::bitset<Nb>& b) const noexcept
                {
                        const size_t clength = (Nb + __CHAR_BIT__ - 1) / __CHAR_BIT__;
                        return std::_Hash_impl::hash(b.M_getdata(), clength);
                }
        };

        template<>
        struct hash<universal_bitblock::bitset<0>>
                : public __hash_base<size_t, universal_bitblock::bitset<0>>
        {
                size_t
                        operator()(const universal_bitblock::bitset<0>&) const noexcept
                {
                        return 0;
                }
        };
}
#endif // VERSION_SPECIALIZATION

//#endif // C++11

#endif /* UBB_H */
