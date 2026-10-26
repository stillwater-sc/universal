#pragma once
// limb_kernels.hpp: products, quotients, square and cube roots of natural numbers on 64-bit limbs
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Natural numbers as 64-bit limbs, least significant first: schoolbook products with a
// 64 x 64 -> 128 bit partial product, Karatsuba and Toom-3 above measured thresholds (#1762,
// #1814), short products (#1807), division a 64-bit quotient limb per step with
// divide-and-conquer above a threshold, Zimmermann's square root on the division and the
// squaring (#1763), and the same recursion carried over to the cube (#1800). efloat holds its
// significand in this format (#1820) and is the first user; nothing here is efloat's own, and
// the kernels lived in efloat/efloat_mpn.hpp as namespace efloat_mpn until #1823.
// Every operand is a pointer and a limb count, least significant limb first, and nothing
// allocates; the caller provides the scratch space. The operation names follow GMP's mpn layer
// (add_n, addmul_1, div_qr, sqrtrem), so the two read side by side, but the code is written
// from the published algorithms cited at each kernel, not from GMP's sources (LGPL/GPL); see
// docs/licensing/mpfr-gmp-lgpl.md.
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>   // std::is_constant_evaluated: the kernels run in constant evaluation (#1819)
#include <universal/internal/blocktype/carry.hpp>   // mul128, uint128_t

// x86-64 kernels in GNU inline assembly (gcc, clang), chosen at run time when the processor has
// BMI2 and ADX (#1816); define UNIVERSAL_LIMB_KERNELS_NO_ASM to compile the portable loops alone.
// EFLOAT_MPN_NO_ASM, its name before #1823, is accepted too: deprecated, and to be removed after
// one release.
#if defined(EFLOAT_MPN_NO_ASM) && !defined(UNIVERSAL_LIMB_KERNELS_NO_ASM)
#define UNIVERSAL_LIMB_KERNELS_NO_ASM 1
#endif
#if !defined(UNIVERSAL_LIMB_KERNELS_NO_ASM) && defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#define UNIVERSAL_LIMB_KERNELS_X86_64_ASM 1
#include <cpuid.h>
#else
#define UNIVERSAL_LIMB_KERNELS_X86_64_ASM 0
#endif

namespace sw { namespace universal { namespace limb_kernels {

using limb_t = std::uint64_t;

// operands of at least this many limbs multiply by Karatsuba, below it by schoolbook. The
// thresholds here and below were measured with the basecase each build runs: the x86-64
// assembly (#1816), or the portable loops (#1762, #1807, #1769)
#if UNIVERSAL_LIMB_KERNELS_X86_64_ASM
inline constexpr std::size_t karatsuba_threshold = 24;
inline constexpr std::size_t karatsuba_sqr_threshold = 64;
#else
inline constexpr std::size_t karatsuba_threshold = 16;
inline constexpr std::size_t karatsuba_sqr_threshold = 48;
#endif

// r[0, n) = a[0, n) * b, returns the high limb
inline constexpr limb_t mul_1_portable(limb_t* r, const limb_t* a, std::size_t n, limb_t b) noexcept {
	limb_t carry = 0;
	for (std::size_t i = 0; i < n; ++i) {
#if defined(__SIZEOF_INT128__)
		const uint128_t t = static_cast<uint128_t>(a[i]) * b + carry;
		r[i]  = static_cast<limb_t>(t);
		carry = static_cast<limb_t>(t >> 64);
#else
		limb_t lo, hi;
		mul128(a[i], b, lo, hi);
		lo += carry;
		hi += (lo < carry) ? 1u : 0u;
		r[i]  = lo;
		carry = hi;
#endif
	}
	return carry;
}

// r[0, n) += a[0, n) * b, returns the carry out of the top: a * b + r + carry < 2^128
inline constexpr limb_t addmul_1_portable(limb_t* r, const limb_t* a, std::size_t n, limb_t b) noexcept {
	limb_t carry = 0;
	for (std::size_t i = 0; i < n; ++i) {
#if defined(__SIZEOF_INT128__)
		const uint128_t t = static_cast<uint128_t>(a[i]) * b + r[i] + carry;
		r[i]  = static_cast<limb_t>(t);
		carry = static_cast<limb_t>(t >> 64);
#else
		limb_t lo, hi;
		mul128(a[i], b, lo, hi);
		lo += carry;
		hi += (lo < carry) ? 1u : 0u;
		lo += r[i];
		hi += (lo < r[i]) ? 1u : 0u;
		r[i]  = lo;
		carry = hi;
#endif
	}
	return carry;
}

// r[0, n) = a[0, n) + b[0, n), returns the carry; r may be a or b
inline constexpr limb_t add_n_portable(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n) noexcept {
	limb_t carry = 0;
	for (std::size_t i = 0; i < n; ++i) {
		const limb_t s = a[i] + carry;
		const limb_t c = (s < carry) ? 1u : 0u;
		r[i]  = s + b[i];
		carry = c + ((r[i] < s) ? 1u : 0u);
	}
	return carry;
}

// r[0, n) = a[0, n) - b[0, n), returns the borrow; r may be a or b
inline constexpr limb_t sub_n_portable(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n) noexcept {
	limb_t borrow = 0;
	for (std::size_t i = 0; i < n; ++i) {
		const limb_t bi = b[i] + borrow;
		const limb_t c  = (bi < borrow) ? 1u : 0u;
		const limb_t ai = a[i];
		r[i]   = ai - bi;
		borrow = c + ((ai < bi) ? 1u : 0u);
	}
	return borrow;
}

// r[0, n) = 2 a[0, n) - b[0, n) mod B^n as a + (a + ~b + 1), the two sums' carries in c1 and c2,
// which a longer run continues; r may be a or b
inline constexpr void twice_sub_n_portable(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n, limb_t& c1, limb_t& c2) noexcept {
	for (std::size_t i = 0; i < n; ++i) {
		const limb_t ai = a[i];
		const limb_t nb = ~b[i];
		const limb_t s  = ai + c1;
		const limb_t k  = (s < c1) ? 1u : 0u;
		const limb_t d  = s + nb;   // a - b, its borrow the complement of the carry
		c1 = k + ((d < s) ? 1u : 0u);
		const limb_t u = ai + c2;
		const limb_t h = (u < c2) ? 1u : 0u;
		r[i] = u + d;
		c2 = h + ((r[i] < u) ? 1u : 0u);
	}
}

#if UNIVERSAL_LIMB_KERNELS_X86_64_ASM
// The loops below are efloat's own, written from the instruction set's semantics (Intel SDM: mulx,
// adcx, adox), not from GMP's assembly.  Each runs over a multiple of four limbs, n > 0, in order
// from the least significant, one limb finished before the next is read, so a destination may be
// one of the sources exactly as in the portable loops.  mulx takes b in rdx and leaves the flags
// alone; adcx carries through CF only and adox through OF only, so the product's high halves and
// the destination run as two carry chains.  The loop steps a negative index with lea and leaves on
// jrcxz, neither of which touches a flag; a compare and branch would clobber both chains.
namespace x86 {

// BMI2 (mulx) and ADX (adcx, adox): cpuid leaf 7, ebx bits 8 and 19.  A static initializer that
// multiplies before this one has run reads false and takes the portable loops, which agree.
inline bool detect_mulx_adx() noexcept {
	unsigned eax = 0, ebx = 0, ecx = 0, edx = 0;
	if (__get_cpuid_max(0, nullptr) < 7) return false;
	__cpuid_count(7, 0, eax, ebx, ecx, edx);
	return ((ebx >> 8) & 1u) != 0 && ((ebx >> 19) & 1u) != 0;
}
inline const bool has_mulx_adx = detect_mulx_adx();

// r[0, n) = a[0, n) * b + c, returns the high limb
inline limb_t mul_4k(limb_t* r, const limb_t* a, std::size_t n, limb_t b, limb_t c) noexcept {
	std::ptrdiff_t i = -static_cast<std::ptrdiff_t>(n);   // 64 bits on every x86-64 ABI, LLP64 too
	__asm__ volatile(
		"xor %%r8d, %%r8d\n\t"                 // CF = OF = 0
		"1:\n\t"
		"mulx (%[a],%[i],8), %%r9, %%r10\n\t"
		"adcx %[c], %%r9\n\t"
		"mov %%r9, (%[r],%[i],8)\n\t"
		"mulx 8(%[a],%[i],8), %%r9, %[c]\n\t"
		"adcx %%r10, %%r9\n\t"
		"mov %%r9, 8(%[r],%[i],8)\n\t"
		"mulx 16(%[a],%[i],8), %%r9, %%r10\n\t"
		"adcx %[c], %%r9\n\t"
		"mov %%r9, 16(%[r],%[i],8)\n\t"
		"mulx 24(%[a],%[i],8), %%r9, %[c]\n\t"
		"adcx %%r10, %%r9\n\t"
		"mov %%r9, 24(%[r],%[i],8)\n\t"
		"lea 4(%[i]), %[i]\n\t"
		"jrcxz 2f\n\t"
		"jmp 1b\n\t"
		"2:\n\t"
		"adcx %%r8, %[c]\n\t"
		: [i] "+c"(i), [c] "+r"(c)
		: [a] "r"(a + n), [r] "r"(r + n), "d"(b)
		: "r8", "r9", "r10", "cc", "memory");
	return c;
}

// r[0, n) += a[0, n) * b + c, returns the carry out of the top
inline limb_t addmul_4k(limb_t* r, const limb_t* a, std::size_t n, limb_t b, limb_t c) noexcept {
	std::ptrdiff_t i = -static_cast<std::ptrdiff_t>(n);
	__asm__ volatile(
		"xor %%r8d, %%r8d\n\t"
		"1:\n\t"
		"mulx (%[a],%[i],8), %%r9, %%r10\n\t"
		"adox %[c], %%r9\n\t"                  // the previous high half, through OF
		"adcx (%[r],%[i],8), %%r9\n\t"         // the destination, through CF
		"mov %%r9, (%[r],%[i],8)\n\t"
		"mulx 8(%[a],%[i],8), %%r9, %[c]\n\t"
		"adox %%r10, %%r9\n\t"
		"adcx 8(%[r],%[i],8), %%r9\n\t"
		"mov %%r9, 8(%[r],%[i],8)\n\t"
		"mulx 16(%[a],%[i],8), %%r9, %%r10\n\t"
		"adox %[c], %%r9\n\t"
		"adcx 16(%[r],%[i],8), %%r9\n\t"
		"mov %%r9, 16(%[r],%[i],8)\n\t"
		"mulx 24(%[a],%[i],8), %%r9, %[c]\n\t"
		"adox %%r10, %%r9\n\t"
		"adcx 24(%[r],%[i],8), %%r9\n\t"
		"mov %%r9, 24(%[r],%[i],8)\n\t"
		"lea 4(%[i]), %[i]\n\t"
		"jrcxz 2f\n\t"
		"jmp 1b\n\t"
		"2:\n\t"
		"adox %%r8, %[c]\n\t"                  // both pending carries fold into the high limb:
		"adcx %%r8, %[c]\n\t"                  // a * b + r + c < 2^(64(n+1)), so neither overflows it
		: [i] "+c"(i), [c] "+r"(c)
		: [a] "r"(a + n), [r] "r"(r + n), "d"(b)
		: "r8", "r9", "r10", "cc", "memory");
	return c;
}

// r[0, n) -= a[0, n) * b + c, returns the borrow out of the top.  The product's limbs p come
// from the OF chain as in addmul_4k; r - p is r + ~p + 1 on the CF chain, which starts set, and
// the borrow is the product's high limb plus one less the final carry.
inline limb_t submul_4k(limb_t* r, const limb_t* a, std::size_t n, limb_t b, limb_t c) noexcept {
	std::ptrdiff_t i = -static_cast<std::ptrdiff_t>(n);
	__asm__ volatile(
		"xor %%r8d, %%r8d\n\t"
		"stc\n\t"
		"1:\n\t"
		"mulx (%[a],%[i],8), %%r9, %%r10\n\t"
		"adox %[c], %%r9\n\t"
		"not %%r9\n\t"
		"adcx (%[r],%[i],8), %%r9\n\t"
		"mov %%r9, (%[r],%[i],8)\n\t"
		"mulx 8(%[a],%[i],8), %%r9, %[c]\n\t"
		"adox %%r10, %%r9\n\t"
		"not %%r9\n\t"
		"adcx 8(%[r],%[i],8), %%r9\n\t"
		"mov %%r9, 8(%[r],%[i],8)\n\t"
		"mulx 16(%[a],%[i],8), %%r9, %%r10\n\t"
		"adox %[c], %%r9\n\t"
		"not %%r9\n\t"
		"adcx 16(%[r],%[i],8), %%r9\n\t"
		"mov %%r9, 16(%[r],%[i],8)\n\t"
		"mulx 24(%[a],%[i],8), %%r9, %[c]\n\t"
		"adox %%r10, %%r9\n\t"
		"not %%r9\n\t"
		"adcx 24(%[r],%[i],8), %%r9\n\t"
		"mov %%r9, 24(%[r],%[i],8)\n\t"
		"lea 4(%[i]), %[i]\n\t"
		"jrcxz 2f\n\t"
		"jmp 1b\n\t"
		"2:\n\t"
		"adox %%r8, %[c]\n\t"
		"cmc\n\t"
		"adcx %%r8, %[c]\n\t"
		: [i] "+c"(i), [c] "+r"(c)
		: [a] "r"(a + n), [r] "r"(r + n), "d"(b)
		: "r8", "r9", "r10", "cc", "memory");
	return c;
}

// r[0, n) = a[0, n) + b[0, n) + c, c in {0, 1}, returns the carry
inline limb_t add_4k(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n, limb_t c) noexcept {
	std::ptrdiff_t i = -static_cast<std::ptrdiff_t>(n);
	__asm__ volatile(
		"neg %[c]\n\t"                         // CF = c
		"1:\n\t"
		"mov (%[a],%[i],8), %%r9\n\t"
		"adc (%[b],%[i],8), %%r9\n\t"
		"mov %%r9, (%[r],%[i],8)\n\t"
		"mov 8(%[a],%[i],8), %%r9\n\t"
		"adc 8(%[b],%[i],8), %%r9\n\t"
		"mov %%r9, 8(%[r],%[i],8)\n\t"
		"mov 16(%[a],%[i],8), %%r9\n\t"
		"adc 16(%[b],%[i],8), %%r9\n\t"
		"mov %%r9, 16(%[r],%[i],8)\n\t"
		"mov 24(%[a],%[i],8), %%r9\n\t"
		"adc 24(%[b],%[i],8), %%r9\n\t"
		"mov %%r9, 24(%[r],%[i],8)\n\t"
		"lea 4(%[i]), %[i]\n\t"
		"jrcxz 2f\n\t"
		"jmp 1b\n\t"
		"2:\n\t"
		"mov $0, %[c]\n\t"
		"adc $0, %[c]\n\t"
		: [i] "+c"(i), [c] "+r"(c)
		: [a] "r"(a + n), [b] "r"(b + n), [r] "r"(r + n)
		: "r9", "cc", "memory");
	return c;
}

// r[0, n) = a[0, n) - b[0, n) - c, c in {0, 1}, returns the borrow
inline limb_t sub_4k(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n, limb_t c) noexcept {
	std::ptrdiff_t i = -static_cast<std::ptrdiff_t>(n);
	__asm__ volatile(
		"neg %[c]\n\t"
		"1:\n\t"
		"mov (%[a],%[i],8), %%r9\n\t"
		"sbb (%[b],%[i],8), %%r9\n\t"
		"mov %%r9, (%[r],%[i],8)\n\t"
		"mov 8(%[a],%[i],8), %%r9\n\t"
		"sbb 8(%[b],%[i],8), %%r9\n\t"
		"mov %%r9, 8(%[r],%[i],8)\n\t"
		"mov 16(%[a],%[i],8), %%r9\n\t"
		"sbb 16(%[b],%[i],8), %%r9\n\t"
		"mov %%r9, 16(%[r],%[i],8)\n\t"
		"mov 24(%[a],%[i],8), %%r9\n\t"
		"sbb 24(%[b],%[i],8), %%r9\n\t"
		"mov %%r9, 24(%[r],%[i],8)\n\t"
		"lea 4(%[i]), %[i]\n\t"
		"jrcxz 2f\n\t"
		"jmp 1b\n\t"
		"2:\n\t"
		"mov $0, %[c]\n\t"
		"adc $0, %[c]\n\t"
		: [i] "+c"(i), [c] "+r"(c)
		: [a] "r"(a + n), [b] "r"(b + n), [r] "r"(r + n)
		: "r9", "cc", "memory");
	return c;
}

// r[0, n) = 2 a[0, n) - b[0, n) mod B^n, carries in c1 and c2 as twice_sub_n_portable's: a + ~b
// and its carry through CF (adcx), plus a through OF (adox), one pass for both (#1828); the not
// that forms ~b leaves both flags alone
inline void twice_sub_4k(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n, limb_t c1, limb_t c2) noexcept {
	std::ptrdiff_t i = -static_cast<std::ptrdiff_t>(n);
	__asm__ volatile(
		"xor %%r10d, %%r10d\n\t"               // CF = OF = 0
		"mov $-1, %%r10\n\t"
		"adcx %[c1], %%r10\n\t"                // CF = c1: all ones + c1 carries when c1 is 1
		"mov $-1, %%r10\n\t"
		"adox %[c2], %%r10\n\t"                // OF = c2, and adox leaves CF alone
		"1:\n\t"
		"mov (%[b],%[i],8), %%r9\n\t"
		"not %%r9\n\t"
		"adcx (%[a],%[i],8), %%r9\n\t"
		"adox (%[a],%[i],8), %%r9\n\t"
		"mov %%r9, (%[r],%[i],8)\n\t"
		"mov 8(%[b],%[i],8), %%r9\n\t"
		"not %%r9\n\t"
		"adcx 8(%[a],%[i],8), %%r9\n\t"
		"adox 8(%[a],%[i],8), %%r9\n\t"
		"mov %%r9, 8(%[r],%[i],8)\n\t"
		"mov 16(%[b],%[i],8), %%r9\n\t"
		"not %%r9\n\t"
		"adcx 16(%[a],%[i],8), %%r9\n\t"
		"adox 16(%[a],%[i],8), %%r9\n\t"
		"mov %%r9, 16(%[r],%[i],8)\n\t"
		"mov 24(%[b],%[i],8), %%r9\n\t"
		"not %%r9\n\t"
		"adcx 24(%[a],%[i],8), %%r9\n\t"
		"adox 24(%[a],%[i],8), %%r9\n\t"
		"mov %%r9, 24(%[r],%[i],8)\n\t"
		"lea 4(%[i]), %[i]\n\t"
		"jrcxz 2f\n\t"
		"jmp 1b\n\t"
		"2:\n\t"
		: [i] "+c"(i)
		: [a] "r"(a + n), [b] "r"(b + n), [r] "r"(r + n), [c1] "r"(c1), [c2] "r"(c2)
		: "r9", "r10", "cc", "memory");
}

}  // namespace x86
#endif

// The kernels: the low n mod 4 limbs by the portable loop, the rest by the assembly when the
// processor has it, its carry passed in
inline constexpr limb_t mul_1(limb_t* r, const limb_t* a, std::size_t n, limb_t b) noexcept {
#if UNIVERSAL_LIMB_KERNELS_X86_64_ASM
	if (n >= 4 && !std::is_constant_evaluated() && x86::has_mulx_adx) {
		const std::size_t t = n & 3u;
		return x86::mul_4k(r + t, a + t, n - t, b, mul_1_portable(r, a, t, b));
	}
#endif
	return mul_1_portable(r, a, n, b);
}

inline constexpr limb_t addmul_1(limb_t* r, const limb_t* a, std::size_t n, limb_t b) noexcept {
#if UNIVERSAL_LIMB_KERNELS_X86_64_ASM
	if (n >= 4 && !std::is_constant_evaluated() && x86::has_mulx_adx) {
		const std::size_t t = n & 3u;
		return x86::addmul_4k(r + t, a + t, n - t, b, addmul_1_portable(r, a, t, b));
	}
#endif
	return addmul_1_portable(r, a, n, b);
}

inline constexpr limb_t add_n(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n) noexcept {
#if UNIVERSAL_LIMB_KERNELS_X86_64_ASM
	if (n >= 4 && !std::is_constant_evaluated()) {   // adc alone: every x86-64 processor has it
		const std::size_t t = n & 3u;
		return x86::add_4k(r + t, a + t, b + t, n - t, add_n_portable(r, a, b, t));
	}
#endif
	return add_n_portable(r, a, b, n);
}

inline constexpr limb_t sub_n(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n) noexcept {
#if UNIVERSAL_LIMB_KERNELS_X86_64_ASM
	if (n >= 4 && !std::is_constant_evaluated()) {
		const std::size_t t = n & 3u;
		return x86::sub_4k(r + t, a + t, b + t, n - t, sub_n_portable(r, a, b, t));
	}
#endif
	return sub_n_portable(r, a, b, n);
}

// r[0, n) = 2 a[0, n) - b[0, n) mod B^n, in one pass where the processor has ADX (#1828); a
// difference one bit apart that cancels its leading bit is half of it. r may be a or b.
inline constexpr void twice_sub_n(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n) noexcept {
	limb_t c1 = 1, c2 = 0;   // a + ~b + 1 = a - b
#if UNIVERSAL_LIMB_KERNELS_X86_64_ASM
	if (n >= 4 && !std::is_constant_evaluated() && x86::has_mulx_adx) {
		const std::size_t t = n & 3u;
		twice_sub_n_portable(r, a, b, t, c1, c2);
		x86::twice_sub_4k(r + t, a + t, b + t, n - t, c1, c2);
		return;
	}
#endif
	twice_sub_n_portable(r, a, b, n, c1, c2);
}

// r[0, n) += c, returns the carry out of the top
inline constexpr limb_t add_1(limb_t* r, std::size_t n, limb_t c) noexcept {
	for (std::size_t i = 0; i < n && c != 0; ++i) {
		r[i] += c;
		c = (r[i] < c) ? 1u : 0u;
	}
	return c;
}

// r[0, m + n) = a[0, m) * b[0, n), n >= 1, schoolbook; r overlaps neither operand
inline constexpr void mul_basecase(limb_t* r, const limb_t* a, std::size_t m, const limb_t* b, std::size_t n) noexcept {
	r[m] = mul_1(r, a, m, b[0]);
	for (std::size_t j = 1; j < n; ++j) r[m + j] = addmul_1(r + j, a, m, b[j]);
}

// r[0, 2n) = a[0, n)^2: each cross product a_i a_j (i < j) once, doubled, plus the squares a_i^2
inline constexpr void sqr_basecase(limb_t* r, const limb_t* a, std::size_t n) noexcept {
	for (std::size_t i = 0; i < 2 * n; ++i) r[i] = 0;
	for (std::size_t i = 0; i + 1 < n; ++i) r[i + n] = addmul_1(r + 2 * i + 1, a + i + 1, n - i - 1, a[i]);
	// double the cross products: their sum is below a^2 / 2, so no bit leaves the top
	limb_t top = 0;
	for (std::size_t k = 0; k < 2 * n; ++k) {
		const limb_t next = r[k] >> 63;
		r[k] = (r[k] << 1) | top;
		top  = next;
	}
	limb_t carry = 0;
	for (std::size_t i = 0; i < n; ++i) {
		limb_t lo, hi;
#if defined(__SIZEOF_INT128__)
		const uint128_t sq = static_cast<uint128_t>(a[i]) * a[i];
		lo = static_cast<limb_t>(sq);
		hi = static_cast<limb_t>(sq >> 64);
#else
		mul128(a[i], a[i], lo, hi);
#endif
		limb_t s = r[2 * i] + carry;
		limb_t c = (s < carry) ? 1u : 0u;
		r[2 * i] = s + lo;
		c += (r[2 * i] < s) ? 1u : 0u;
		s = r[2 * i + 1] + c;
		c = (s < c) ? 1u : 0u;
		r[2 * i + 1] = s + hi;
		carry = c + ((r[2 * i + 1] < s) ? 1u : 0u);
	}
}

// d[0, k) = |x[0, k) - y[0, h)| for h <= k, returns whether x < y
inline constexpr bool abs_diff(limb_t* d, const limb_t* x, std::size_t k, const limb_t* y, std::size_t h) noexcept {
	bool x_smaller = false;
	bool x_high    = false;   // y is zero above h: x is the larger if any of x[h, k) is nonzero
	for (std::size_t j = h; j < k; ++j) x_high = x_high || (x[j] != 0);
	if (!x_high) {
		for (std::size_t j = h; j-- > 0;) {
			if (x[j] != y[j]) { x_smaller = (x[j] < y[j]); break; }
		}
	}
	if (x_smaller) {
		sub_n(d, y, x, h);   // x[h, k) is zero, so the difference has h limbs
		for (std::size_t j = h; j < k; ++j) d[j] = 0;
	}
	else {
		limb_t borrow = sub_n(d, x, y, h);
		for (std::size_t j = h; j < k; ++j) {
			const limb_t xj = x[j];
			d[j]   = xj - borrow;
			borrow = (xj < borrow) ? 1u : 0u;
		}
	}
	return x_smaller;
}

// the scratch limbs mul and mul_n need when neither operand is longer than n limbs: mul_n takes
// below 6n + 64, and an unbalanced mul adds a block product per level of its remainder sequence
constexpr std::size_t scratch_limbs(std::size_t n) noexcept { return 16 * n + 128; }

inline constexpr void mul_n(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n, limb_t* s, bool square) noexcept;
inline constexpr limb_t sub_1(limb_t* r, std::size_t n, limb_t c) noexcept;
inline constexpr limb_t lshift(limb_t* r, const limb_t* a, std::size_t n, unsigned c) noexcept;

// operands of at least this many limbs multiply by Toom-3, below it by Karatsuba (#1814). By
// measurement: from 256 limbs a product is 2-16% faster by Toom-3 (9% at 512, 16% at 1024), at
// 224 and below Karatsuba is as fast or faster; a square gains 4-10% from 384 limbs. The
// evaluations and the interpolation cost about what the saved product earns below that, where
// GMP, with assembly for its passes and its Karatsuba, switches near 80 limbs.
inline constexpr std::size_t toom3_threshold = 240;
inline constexpr std::size_t toom3_sqr_threshold = 352;

// Toom-3 (#1814). With a = a2 x^2 + a1 x + a0 at x = B^k, k = ceil(n / 3), and b likewise, the
// product's five coefficients follow from its values at 0, 1, -1, -2 and infinity, five products
// of k + 1 limbs, by Bodrato's interpolation (Bodrato and Zanoni, Integer and polynomial
// multiplication: towards optimal Toom-Cook matrices, ISSAC 2007):
//     r3 = (v(-2) - v(1)) / 3,  r1 = (v(1) - v(-1)) / 2,  r2 = v(-1) - v(0),
//     r3 = (r2 - r3) / 2 + 2 v(inf),  r2 = r2 + r1 - v(inf),  r1 = r1 - r3,
// every division exact. The values and the coefficients are held in two's complement at a fixed
// width (k + 2 and 2k + 2 limbs), where adding, subtracting and the exact divisions all work
// modulo B^width: a halving is an arithmetic shift, and a third the product by 3^-1 mod 2^64
// limb by limb (Jebelean's exact division). The evaluations, by Bodrato's chain:
//     p = a0 + a2,  a(1) = p + a1,  a(-1) = p - a1,  a(-2) = 2 (a(-1) + a2) - a0.
namespace toom {

	// r[0, w) = -r[0, w) mod B^w
	inline constexpr void negate(limb_t* r, std::size_t w) noexcept {
		limb_t c = 1;
		for (std::size_t i = 0; i < w; ++i) {
			const limb_t v = ~r[i] + c;
			c = (v < c) ? 1u : 0u;
			r[i] = v;
		}
	}

	// whether r[0, w) is negative, read as two's complement
	inline constexpr bool negative(const limb_t* r, std::size_t w) noexcept { return (r[w - 1] >> 63) != 0u; }

	// r[0, w) = r[0, w) + or - b[0, nb) mod B^w, b nonnegative, nb <= w
	inline constexpr void add_short(limb_t* r, std::size_t w, const limb_t* b, std::size_t nb) noexcept {
		add_1(r + nb, w - nb, add_n(r, r, b, nb));
	}
	inline constexpr void sub_short(limb_t* r, std::size_t w, const limb_t* b, std::size_t nb) noexcept {
		sub_1(r + nb, w - nb, sub_n(r, r, b, nb));
	}

	// r[0, w) = r[0, w) / 2, arithmetic: r is even
	inline constexpr void halve(limb_t* r, std::size_t w) noexcept {
		for (std::size_t i = 0; i + 1 < w; ++i) r[i] = (r[i] >> 1) | (r[i + 1] << 63);
		r[w - 1] = static_cast<limb_t>(static_cast<std::int64_t>(r[w - 1]) >> 1);
	}

	// r[0, w) = r[0, w) / 3 mod B^w, exactly: r is a multiple of 3
	inline constexpr void third(limb_t* r, std::size_t w) noexcept {
		constexpr limb_t inv3 = 0xAAAA'AAAA'AAAA'AAABull;   // 3^-1 mod 2^64
		limb_t c = 0;
		for (std::size_t i = 0; i < w; ++i) {
			const limb_t x = r[i];
			const limb_t b = (x < c) ? 1u : 0u;
			const limb_t q = (x - c) * inv3;
			r[i] = q;
			limb_t hi, lo;
#if defined(__SIZEOF_INT128__)
			const uint128_t t = static_cast<uint128_t>(q) * 3u;
			lo = static_cast<limb_t>(t);
			hi = static_cast<limb_t>(t >> 64);
#else
			mul128(q, 3u, lo, hi);
#endif
			(void)lo;
			c = hi + b;
		}
	}

	// e1, em1, em2 (k + 2 limbs each, two's complement) = a(1), a(-1), a(-2) for a = a[0, n)
	// split at k: a0 = a[0, k), a1 = a[k, 2k), a2 = a[2k, n) of s limbs
	inline constexpr void evaluate(limb_t* e1, limb_t* em1, limb_t* em2, const limb_t* a, std::size_t k, std::size_t s) noexcept {
		const std::size_t w = k + 2;
		limb_t* p = em2;   // p = a0 + a2, kept in em2 until it is formed
		for (std::size_t i = 0; i < k; ++i) p[i] = a[i];
		p[k] = 0u;
		p[k + 1] = 0u;
		add_short(p, w, a + 2 * k, s);
		for (std::size_t i = 0; i < w; ++i) { e1[i] = p[i]; em1[i] = p[i]; }
		add_short(e1, w, a + k, k);    // a(1)  = p + a1
		sub_short(em1, w, a + k, k);   // a(-1) = p - a1
		for (std::size_t i = 0; i < w; ++i) em2[i] = em1[i];
		add_short(em2, w, a + 2 * k, s);
		lshift(em2, em2, w, 1);        // exact: |2 (a(-1) + a2)| < 6 B^k
		sub_short(em2, w, a, k);       // a(-2) = 2 (a(-1) + a2) - a0
	}

	// the magnitude of a two's complement value of w limbs, in place; returns whether it was negative
	inline constexpr bool magnitude(limb_t* r, std::size_t w) noexcept {
		const bool neg = negative(r, w);
		if (neg) negate(r, w);
		return neg;
	}

	// v[0, 2k + 2) = x y for x, y of k + 1 limbs whose top limbs are small (an evaluation's, 4 at
	// most), as GMP's Toom codes form them: the product of the k low limbs, which keeps the
	// recursion on k limbs, and the top limbs' cross terms added at B^k, one limb product each
	inline constexpr void product(limb_t* v, const limb_t* x, const limb_t* y, std::size_t k, limb_t* s, bool square) noexcept {
		mul_n(v, x, square ? x : y, k, s, square);
		v[2 * k] = 0u;
		v[2 * k + 1] = 0u;
		const limb_t tx = x[k];
		const limb_t ty = square ? tx : y[k];
		if (square) {
			if (tx != 0u) {
				add_1(v + 2 * k, 2, addmul_1(v + k, x, k, 2 * tx));   // 2 tx x_low B^k
				add_1(v + 2 * k, 2, tx * tx);
			}
			return;
		}
		if (tx != 0u) add_1(v + 2 * k, 2, addmul_1(v + k, y, k, tx));
		if (ty != 0u) add_1(v + 2 * k, 2, addmul_1(v + k, x, k, ty));
		add_1(v + 2 * k, 2, tx * ty);
	}

}  // namespace toom

// r[0, 2n) = a[0, n) * b[0, n), or a^2 when square, by Toom-3, for n >= 5 (n = 4 leaves the top
// third empty; the thresholds lie far above); s holds
// scratch_limbs(n) limbs: 12k + 18 here (six evaluations of k + 2 limbs, three products of
// 2k + 2) and the five products' own after them
inline constexpr void toom3_mul(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n, limb_t* s, bool square) noexcept {
	const std::size_t k  = (n + 2) / 3;
	const std::size_t sl = n - 2 * k;   // a2's and b2's limbs, 1 to k
	const std::size_t we = k + 2;       // an evaluation's width
	const std::size_t L  = 2 * k + 2;   // a coefficient's width
	limb_t* ea1  = s;
	limb_t* eam1 = ea1 + we;
	limb_t* eam2 = eam1 + we;
	limb_t* eb1  = eam2 + we;
	limb_t* ebm1 = eb1 + we;
	limb_t* ebm2 = ebm1 + we;
	limb_t* v1   = ebm2 + we;   // v(1), then r1
	limb_t* vm1  = v1 + L;      // v(-1), then r2
	limb_t* vm2  = vm1 + L;     // v(-2), then r3
	limb_t* ss   = vm2 + L;     // the products' scratch
	toom::evaluate(ea1, eam1, eam2, a, k, sl);
	const bool na1 = toom::magnitude(eam1, we);
	const bool na2 = toom::magnitude(eam2, we);
	bool nb1 = na1, nb2 = na2;
	if (!square) {
		toom::evaluate(eb1, ebm1, ebm2, b, k, sl);
		nb1 = toom::magnitude(ebm1, we);
		nb2 = toom::magnitude(ebm2, we);
	}
	// the magnitudes fit k + 1 limbs: |a(1)| < 3 B^k, |a(-1)| < 2 B^k, |a(-2)| < 5 B^k
	toom::product(v1,  ea1,  eb1,  k, ss, square);
	toom::product(vm1, eam1, ebm1, k, ss, square);
	toom::product(vm2, eam2, ebm2, k, ss, square);
	if (na1 != nb1) toom::negate(vm1, L);
	if (na2 != nb2) toom::negate(vm2, L);
	// v(0) and v(inf) straight into r, which they partly cover: r[0, 2k) and r[4k, 2n)
	mul_n(r, a, square ? a : b, k, ss, square);
	mul_n(r + 4 * k, a + 2 * k, square ? a + 2 * k : b + 2 * k, sl, ss, square);
	const limb_t* v0   = r;
	const limb_t* vinf = r + 4 * k;
	const std::size_t ninf = 2 * sl;
	// the interpolation, in two's complement of L limbs
	sub_n(vm2, vm2, v1, L);  toom::third(vm2, L);             // r3 = (v(-2) - v(1)) / 3
	sub_n(v1, v1, vm1, L);   toom::halve(v1, L);              // r1 = (v(1) - v(-1)) / 2
	toom::sub_short(vm1, L, v0, 2 * k);                       // r2 = v(-1) - v(0)
	sub_n(vm2, vm1, vm2, L); toom::halve(vm2, L);             // r3 = (r2 - r3) / 2 + 2 v(inf)
	toom::add_short(vm2, L, vinf, ninf);
	toom::add_short(vm2, L, vinf, ninf);
	add_n(vm1, vm1, v1, L);  toom::sub_short(vm1, L, vinf, ninf);   // r2 = r2 + r1 - v(inf)
	sub_n(v1, v1, vm2, L);                                    // r1 = r1 - r3
	// r = v(0) + r1 x + r2 x^2 + r3 x^3 + v(inf) x^4: the gap between v(0) and v(inf) cleared, the
	// middle coefficients added in; each is nonnegative, and a b < B^2n leaves nothing past r's end
	for (std::size_t i = 2 * k; i < 4 * k; ++i) r[i] = 0u;
	const std::size_t rn = 2 * n;
	auto add_at = [r, rn, L](const limb_t* c, std::size_t at) {
		const std::size_t len = (at + L <= rn) ? L : rn - at;
		add_1(r + at + len, rn - at - len, add_n(r + at, r + at, c, len));
	};
	add_at(v1, k);
	add_at(vm1, 2 * k);
	add_at(vm2, 3 * k);
}

// r[0, 2n) = a[0, n) * b[0, n), or a^2 when square; s holds scratch_limbs(n) limbs.
// Subtractive Karatsuba: with a = a1 B^k + a0 and b likewise, the middle coefficient is
// z0 + z2 - (a0 - a1)(b0 - b1), where the differences have k limbs and no carry. From
// toom3_threshold limbs, Toom-3 (#1814).
inline constexpr void mul_n(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n, limb_t* s, bool square) noexcept {
	if (n >= (square ? toom3_sqr_threshold : toom3_threshold)) {
		toom3_mul(r, a, b, n, s, square);
		return;
	}
	if (square ? (n < karatsuba_sqr_threshold) : (n < karatsuba_threshold)) {
		if (square) sqr_basecase(r, a, n); else mul_basecase(r, a, n, b, n);
		return;
	}
	const std::size_t k = (n + 1) / 2;   // the low halves have k limbs, the high halves h <= k
	const std::size_t h = n - k;
	limb_t* da = s;
	limb_t* db = s + k;
	limb_t* t  = s + 2 * k;
	limb_t* ss = s + 4 * k;
	const bool na = abs_diff(da, a, k, a + k, h);
	const bool nb = square ? na : abs_diff(db, b, k, b + k, h);
	mul_n(t, da, square ? da : db, k, ss, square);           // t = |a0 - a1| |b0 - b1|, 2k limbs
	mul_n(r, a, b, k, ss, square);                           // z0 = a0 b0 in r[0, 2k)
	mul_n(r + 2 * k, a + k, b + k, h, ss, square);           // z2 = a1 b1 in r[2k, 2n)
	// m = z0 + z2 -/+ t, 2k + 1 limbs, in ss (the recursions are done with it)
	limb_t* m = ss;
	limb_t carry = add_n(m, r, r + 2 * k, 2 * h);
	for (std::size_t j = 2 * h; j < 2 * k; ++j) {
		m[j]  = r[j] + carry;
		carry = (m[j] < carry) ? 1u : 0u;
	}
	m[2 * k] = carry;
	if (na == nb) m[2 * k] -= sub_n(m, m, t, 2 * k);    // (a0 - a1)(b0 - b1) >= 0
	else          m[2 * k] += add_n(m, m, t, 2 * k);
	// r[k, 2n) += m: the product fits in 2n limbs, so whatever m holds past r's end is zero
	const std::size_t room = 2 * n - k;
	const std::size_t len  = (2 * k + 1 < room) ? 2 * k + 1 : room;
	carry = add_n(r + k, r + k, m, len);
	add_1(r + k + len, room - len, carry);
}

// r[0, m + n) = a[0, m) * b[0, n) for any m, n >= 1; r overlaps neither operand, s holds
// scratch_limbs(max(m, n)) limbs.  An unbalanced product multiplies the longer operand a block of
// the shorter's length at a time.
inline constexpr void mul(limb_t* r, const limb_t* a, std::size_t m, const limb_t* b, std::size_t n, limb_t* s) noexcept {
	if (m < n) { const limb_t* tp = a; a = b; b = tp; const std::size_t tn = m; m = n; n = tn; }
	if (n < karatsuba_threshold) { mul_basecase(r, a, m, b, n); return; }
	if (m == n) { mul_n(r, a, b, n, s, false); return; }
	mul_n(r, a, b, n, s, false);   // r[0, 2n)
	limb_t* t = s;                 // a block product, n + c limbs
	for (std::size_t off = n; off < m; off += n) {
		const std::size_t c = (m - off < n) ? (m - off) : n;
		if (c == n) mul_n(t, a + off, b, n, s + 2 * n, false);
		else        mul(t, b, n, a + off, c, s + 2 * n);
		// r[off, off + n) holds the top of the previous blocks; r[off + n, off + n + c) is new
		limb_t carry = add_n(r + off, r + off, t, n);
		for (std::size_t i = 0; i < c; ++i) {
			r[off + n + i] = t[n + i] + carry;
			carry = (r[off + n + i] < carry) ? 1u : 0u;
		}
	}
}


// r[0, n) = the low n limbs of floor(V 2^shift), V the integer of v[0, nv), 32-bit limbs most
// significant first, as efloat holds a significand, for a shift of either sign; returns whether
// the bits a right shift drops are nonzero (#1810, #1811). V is read as 64-bit words, least
// significant first, each read once and shifted once.
inline constexpr bool load_msb_first(limb_t* r, std::size_t n, const std::uint32_t* v, std::size_t nv, std::int64_t shift) noexcept {
	const std::int64_t m = static_cast<std::int64_t>((nv + 1) / 2);
	auto word = [v, nv, m](std::int64_t j) -> limb_t {   // word j of P = V 2^(32 (2m - nv)), an odd count padded at the bottom
		if (j < 0 || j >= m) return 0u;
		const std::size_t hi = static_cast<std::size_t>(2 * m - 2 - 2 * j);
		return (limb_t(v[hi]) << 32) | ((hi + 1 < nv) ? v[hi + 1] : 0u);
	};
	const std::int64_t s = shift - 32 * (2 * m - static_cast<std::int64_t>(nv));   // floor(P 2^s)
	const std::int64_t q = (s >= 0) ? -((s + 63) / 64) : (-s) / 64;               // floor(-s / 64)
	const unsigned     c = static_cast<unsigned>(-s - 64 * q);                    // in [0, 64)
	limb_t low = word(q);
	for (std::size_t k = 0; k < n; ++k) {
		const limb_t high = word(q + static_cast<std::int64_t>(k) + 1);
		r[k] = (c == 0u) ? low : ((low >> c) | (high << (64u - c)));
		low = high;
	}
	if (s >= 0) return false;
	for (std::int64_t j = 0; j < q && j < m; ++j) {
		if (word(j) != 0u) return true;
	}
	return c != 0u && q < m && (word(q) & ((limb_t(1) << c) - 1u)) != 0u;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Short products (#1807): the high half of a product, for a result rounded to about the operands'
// length, as MPFR's mpfr_mulhigh_n. R = r[n - 1, 2n) is formed from every partial product a_i b_j
// with i + j >= n - 1, and from some below it, each one exactly, so R B^(n - 1) <= a b and it
// misses only the a_i b_j with i + j <= n - 2: (s + 1) of them, each below B^(s + 2), at each
// s = i + j, which sum to below (n - 1) B^n B / (B - 1). Hence
//     0 <= a b - R B^(n - 1) < n B^n,
// n units of r[n]. The basecase forms the n (n + 1) / 2 partial products on or above the
// diagonal, about half a product; above mulhigh_threshold, Mulders' recursion (Mulders, On short
// multiplications and divisions, AAECC 11(1), 2000) forms the top k = 6n / 10 limbs' product in
// full by Karatsuba and the two cross terms by short products of the other l = n - k limbs: the
// partial products it adds are again all those with i + j >= n - 1 and some below.

// and k = 6n / 10: both by measurement (#1807, #1816)
inline constexpr std::size_t mulhigh_threshold = UNIVERSAL_LIMB_KERNELS_X86_64_ASM ? 64 : 32;

// r[n - 1, 2n) = R as above, r[0, n - 1) is overwritten; r overlaps neither operand
inline constexpr void mulhigh_basecase(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n) noexcept {
	for (std::size_t i = n - 1; i < 2 * n; ++i) r[i] = 0;
	// a_i times b[n - 1 - i, n): its partial products from i + j = n - 1 up, all aligned at r[n - 1]
	for (std::size_t i = 0; i < n; ++i) r[n + i] = addmul_1(r + n - 1, b + n - 1 - i, i + 1, a[i]);
}

// r[n - 1, 2n) = R as above for a[0, n) b[0, n), or a^2 when square; r[0, n - 1) is overwritten,
// r overlaps neither operand, s holds scratch_limbs(n) limbs
inline constexpr void mulhigh_n(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n, limb_t* s, bool square) noexcept {
	if (n < mulhigh_threshold) {
		mulhigh_basecase(r, a, b, n);
		return;
	}
	const std::size_t k = (6 * n + 9) / 10;   // 2l <= n - 1: the top product covers r[n - 1]
	const std::size_t l = n - k;
	mul_n(r + 2 * l, a + l, b + l, k, s, square);   // the top limbs' product, in r[2l, 2n)
	for (std::size_t i = n - 1; i < 2 * l; ++i) r[i] = 0;
	// the cross terms a[k, n) b[0, l) and a[0, l) b[k, n): their limbs from l - 1 up land on r[n - 1]
	limb_t* t = s;
	limb_t* u = s + 2 * l;
	mulhigh_n(t, a + k, b, l, u, false);
	limb_t carry = add_n(r + n - 1, r + n - 1, t + l - 1, l + 1);
	add_1(r + n + l, n - l, carry);
	if (!square) mulhigh_n(t, a, b + k, l, u, false);   // a square's two cross terms are one
	carry = add_n(r + n - 1, r + n - 1, t + l - 1, l + 1);
	add_1(r + n + l, n - l, carry);
}

// The fixed-point engines' truncated product (#1807): out[0, na + nb - drop) = T with
//     a b / B^drop - 2 < T <= floor(a b / B^drop),
// within 2 units of its last limb where the exact floor is within 1. From truncated_min_limbs
// up, both operands are padded with zero limbs at the bottom to n = max(na, nb, na + nb + 1 -
// drop) limbs, so that the limbs kept start at L = drop + 2n - na - nb >= n + 1 of the padded
// product: mulhigh_n's R is below it by less than n B^n < B^L, one unit of T's last limb. Below
// truncated_min_limbs the product is formed in full. drop <= na + nb; s holds
// truncated_scratch_limbs(na, nb, drop) limbs and overlaps neither operand nor out.
inline constexpr std::size_t truncated_min_limbs = 16;

constexpr std::size_t truncated_width(std::size_t na, std::size_t nb, std::size_t drop) noexcept {
	std::size_t n = (na > nb) ? na : nb;
	if (na + nb + 1 > drop + n) n = na + nb + 1 - drop;
	return n;
}

constexpr std::size_t truncated_scratch_limbs(std::size_t na, std::size_t nb, std::size_t drop) noexcept {
	const std::size_t n = truncated_width(na, nb, drop);
	return 4 * n + scratch_limbs(n);
}

inline constexpr void mul_truncated(limb_t* out, const limb_t* a, std::size_t na, const limb_t* b, std::size_t nb, std::size_t drop, limb_t* s) noexcept {
	const bool square = (a == b && na == nb);
	const std::size_t len = na + nb - drop;
	if (na < truncated_min_limbs || nb < truncated_min_limbs) {
		limb_t* p = s;
		if (square) mul_n(p, a, a, na, p + na + nb, true);
		else        mul(p, a, na, b, nb, p + na + nb);
		for (std::size_t i = 0; i < len; ++i) out[i] = p[drop + i];
		return;
	}
	const std::size_t n  = truncated_width(na, nb, drop);
	limb_t* pa = s;
	limb_t* pb = pa + n;
	limb_t* r  = pb + n;
	for (std::size_t i = 0; i < n - na; ++i) pa[i] = 0u;
	for (std::size_t i = 0; i < na; ++i) pa[n - na + i] = a[i];
	if (!square) {
		for (std::size_t i = 0; i < n - nb; ++i) pb[i] = 0u;
		for (std::size_t i = 0; i < nb; ++i) pb[n - nb + i] = b[i];
	}
	mulhigh_n(r, pa, square ? pa : pb, n, r + 2 * n, square);
	const std::size_t L = drop + 2 * n - na - nb;
	for (std::size_t i = 0; i < len; ++i) out[i] = r[L + i];
}

// Ziv's test on limbs (#1763, #1807): whether a nonzero value a[0, n), within 2^(scale - correct)
// of the exact result, has bits from position precision + 1 to correct - 2, counting its leading
// bit as 0, that are neither all 0 nor all 1. The rounding boundaries at `precision` bits, the
// representable values and the midpoints, are the values whose bits past position precision are
// all 0, so then both the value and the exact result lie on one side of each of them.
inline constexpr bool can_round(const limb_t* a, std::size_t n, unsigned precision, unsigned correct) noexcept {
	if (correct < precision + 4) return false;
	while (n > 0 && a[n - 1] == 0u) --n;
	if (n == 0) return false;
	const std::int64_t lead = 64 * static_cast<std::int64_t>(n) - 1 - std::countl_zero(a[n - 1]);   // the leading bit's index
	auto bit = [a, lead](unsigned p) -> unsigned {
		const std::int64_t q = lead - static_cast<std::int64_t>(p);
		return (q < 0) ? 0u : static_cast<unsigned>((a[q / 64] >> (q % 64)) & 1u);
	};
	const unsigned first = bit(precision + 1);
	for (unsigned p = precision + 2; p <= correct - 2; ++p) {
		if (bit(p) != first) return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Division (#1763).  The divisor is normalized, its top bit set.  Each quotient limb comes from
// a precomputed reciprocal, as Moller and Granlund describe (Improved division by invariant
// integers, IEEE Trans. Computers, 2011): a 3-by-2 limb estimate that is never more than one too
// small, then one multiply-subtract of the divisor.  Above a threshold, the divide-and-conquer
// division of Burnikel and Ziegler (as GMP's dcpi1_div_qr) splits the quotient in halves and
// pays for each half with one product, so the division costs a few multiplications.

// operands whose quotient and divisor both have at least this many limbs divide by divide-and-conquer
inline constexpr std::size_t dc_div_threshold = UNIVERSAL_LIMB_KERNELS_X86_64_ASM ? 32 : 16;   // by measurement (#1769, #1816)

// a + b + carry and a - b - borrow for a carry or borrow of 0 or 1, the one out left in it: an
// adc or sbb chain where the compiler has the builtin (#1830), which clang does not form from
// the comparisons. For the short sums of efloat and fma, which keep their limbs in registers.
inline constexpr limb_t add_carry(limb_t a, limb_t b, limb_t& carry) noexcept {
#if defined(__has_builtin)
#if __has_builtin(__builtin_addcll)
	if (!std::is_constant_evaluated()) {
		unsigned long long out = 0;
		const limb_t r = static_cast<limb_t>(__builtin_addcll(a, b, carry, &out));
		carry = static_cast<limb_t>(out);
		return r;
	}
#endif
#endif
	const limb_t u = a + carry;
	const limb_t v = u + b;
	carry = ((u < carry) || (v < u)) ? 1u : 0u;
	return v;
}
inline constexpr limb_t sub_borrow(limb_t a, limb_t b, limb_t& borrow) noexcept {
#if defined(__has_builtin)
#if __has_builtin(__builtin_subcll)
	if (!std::is_constant_evaluated()) {
		unsigned long long out = 0;
		const limb_t r = static_cast<limb_t>(__builtin_subcll(a, b, borrow, &out));
		borrow = static_cast<limb_t>(out);
		return r;
	}
#endif
#endif
	const limb_t u = a - b;
	const limb_t v = u - borrow;
	borrow = ((a < b) || (u < borrow)) ? 1u : 0u;
	return v;
}

// the high and low limbs of a * b
inline constexpr void umul(limb_t a, limb_t b, limb_t& hi, limb_t& lo) noexcept {
#if defined(__SIZEOF_INT128__)
	const uint128_t t = static_cast<uint128_t>(a) * b;
	lo = static_cast<limb_t>(t);
	hi = static_cast<limb_t>(t >> 64);
#else
	mul128(a, b, lo, hi);
#endif
}

// floor((u1 B + u0) / d) for a normalized d and u1 < d, the quotient fits one limb
inline constexpr limb_t udiv_2by1(limb_t u1, limb_t u0, limb_t d) noexcept {
#if defined(__SIZEOF_INT128__)
	return static_cast<limb_t>(((static_cast<uint128_t>(u1) << 64) | u0) / d);
#else
	// Knuth D on 32-bit digits (Hacker's Delight, divlu): two quotient digits
	constexpr limb_t b32 = limb_t(1) << 32;
	const limb_t dh = d >> 32, dl = d & 0xFFFF'FFFFu;
	const limb_t uh = u0 >> 32, ul = u0 & 0xFFFF'FFFFu;
	limb_t q1 = u1 / dh, r = u1 - q1 * dh;
	while (q1 >= b32 || q1 * dl > ((r << 32) | uh)) { --q1; r += dh; if (r >= b32) break; }
	const limb_t u21 = (u1 << 32) + uh - q1 * d;
	limb_t q0 = u21 / dh;
	r = u21 - q0 * dh;
	while (q0 >= b32 || q0 * dl > ((r << 32) | ul)) { --q0; r += dh; if (r >= b32) break; }
	return (q1 << 32) | q0;
#endif
}

// the reciprocal of a normalized d: floor((B^2 - 1) / d) - B
inline constexpr limb_t invert_limb(limb_t d) noexcept { return udiv_2by1(~d, ~limb_t(0), d); }

// the reciprocal of the normalized two-limb divisor d1 B + d0: floor((B^3 - 1) / (d1 B + d0)) - B
inline constexpr limb_t invert_pi1(limb_t d1, limb_t d0) noexcept {
	limb_t v = invert_limb(d1);
	limb_t p = d1 * v + d0;
	if (p < d0) {
		--v;
		const limb_t mask = (p >= d1) ? ~limb_t(0) : 0u;
		p -= d1;
		v += mask;
		p -= mask & d1;
	}
	limb_t t1, t0;
	umul(d0, v, t1, t0);
	p += t1;
	if (p < t1) {
		--v;
		if (p >= d1 && (p > d1 || t0 >= d0)) --v;
	}
	return v;
}

// q = floor((n1 B + n0) / d), r = the remainder, for a normalized d and n1 < d, by the
// processor's own 128-by-64-bit division where the build has one (#1826): x86-64's divq costs
// about as much as one step by a reciprocal, and a reciprocal costs several, so a quotient of a
// few limbs is faster this way. Elsewhere, by the reciprocal.
#if UNIVERSAL_LIMB_KERNELS_X86_64_ASM
inline constexpr bool hardware_division = true;
#else
inline constexpr bool hardware_division = false;
#endif
inline constexpr void udiv_qr_2by1(limb_t& q, limb_t& r, limb_t n1, limb_t n0, limb_t d, limb_t v) noexcept;
inline constexpr void udiv_qrnnd(limb_t& q, limb_t& r, limb_t n1, limb_t n0, limb_t d) noexcept {
#if UNIVERSAL_LIMB_KERNELS_X86_64_ASM
	if (!std::is_constant_evaluated()) {
		__asm__("divq %4" : "=a"(q), "=d"(r) : "a"(n0), "d"(n1), "rm"(d) : "cc");
		return;
	}
#endif
	udiv_qr_2by1(q, r, n1, n0, d, invert_limb(d));
}

// q = floor((n1 B + n0) / d), r = the remainder, for n1 < d and v = invert_limb(d). Written for
// the remainder's dependency chain from one limb to the next (#1830): the first correction
// selects r or r + d, formed beforehand, where clang added d or 0 after the comparison; the
// second, rare, is a branch, which clang had made a cmov too. a/7 under clang 0.84 of the time.
inline constexpr void udiv_qr_2by1(limb_t& q, limb_t& r, limb_t n1, limb_t n0, limb_t d, limb_t v) noexcept {
	limb_t qh, ql;
	umul(n1, v, qh, ql);
	ql += n0;
	qh += n1 + 1u + ((ql < n0) ? 1u : 0u);
	const limb_t r0   = n0 - qh * d;
	limb_t       r1   = r0 + d;
#if defined(__clang__)
	if (!std::is_constant_evaluated()) __asm__("" : "+r"(r1));   // else clang folds the select back into an add after the comparison
#endif
	const bool   over = (r0 > ql);
	r  = over ? r1 : r0;
	qh = over ? qh - 1u : qh;
#if defined(__GNUC__) || defined(__clang__)
	if (__builtin_expect(r >= d, 0)) { ++qh; r -= d; }
#else
	if (r >= d) { ++qh; r -= d; }
#endif
	q = qh;
}

// q = floor((n2 B^2 + n1 B + n0) / (d1 B + d0)) and the two-limb remainder r1 B + r0, for
// n2 B + n1 < d1 B + d0 and v = invert_pi1(d1, d0)
inline constexpr void udiv_qr_3by2(limb_t& q, limb_t& r1, limb_t& r0, limb_t n2, limb_t n1, limb_t n0, limb_t d1, limb_t d0, limb_t v) noexcept {
	limb_t q0;
	umul(n2, v, q, q0);
	q0 += n1;
	q  += n2 + ((q0 < n1) ? 1u : 0u);
	r1 = n1 - d1 * q;
	// (r1, r0) = (r1, n0) - (d1, d0) - d0 q
	limb_t borrow = (n0 < d0) ? 1u : 0u;
	r0 = n0 - d0;
	r1 = r1 - d1 - borrow;
	limb_t t1, t0;
	umul(d0, q, t1, t0);
	borrow = (r0 < t0) ? 1u : 0u;
	r0 -= t0;
	r1 = r1 - t1 - borrow;
	++q;
	const limb_t mask = (r1 >= q0) ? ~limb_t(0) : 0u;
	q += mask;
	const limb_t a0 = mask & d0;
	r0 += a0;
	r1 += (mask & d1) + ((r0 < a0) ? 1u : 0u);
	if (r1 >= d1 && (r1 > d1 || r0 >= d0)) {
		++q;
		borrow = (r0 < d0) ? 1u : 0u;
		r0 -= d0;
		r1 = r1 - d1 - borrow;
	}
}

// r[0, n) -= a[0, n) * b, returns the borrow out of the top
inline constexpr limb_t submul_1_portable(limb_t* r, const limb_t* a, std::size_t n, limb_t b) noexcept {
	limb_t carry = 0;
	for (std::size_t i = 0; i < n; ++i) {
#if defined(__SIZEOF_INT128__)
		const uint128_t t  = static_cast<uint128_t>(a[i]) * b + carry;
		const limb_t    lo = static_cast<limb_t>(t);
		const limb_t    hi = static_cast<limb_t>(t >> 64);
#else
		limb_t hi, lo;
		umul(a[i], b, hi, lo);
		lo += carry;
		hi += (lo < carry) ? 1u : 0u;
#endif
		const limb_t ri = r[i];
		r[i]  = ri - lo;
		carry = hi + ((ri < lo) ? 1u : 0u);
	}
	return carry;
}

inline constexpr limb_t submul_1(limb_t* r, const limb_t* a, std::size_t n, limb_t b) noexcept {
#if UNIVERSAL_LIMB_KERNELS_X86_64_ASM
	if (n >= 4 && !std::is_constant_evaluated() && x86::has_mulx_adx) {
		const std::size_t t = n & 3u;
		return x86::submul_4k(r + t, a + t, n - t, b, submul_1_portable(r, a, t, b));
	}
#endif
	return submul_1_portable(r, a, n, b);
}

// r[0, n) -= c, returns the borrow out of the top
inline constexpr limb_t sub_1(limb_t* r, std::size_t n, limb_t c) noexcept {
	for (std::size_t i = 0; i < n && c != 0; ++i) {
		const limb_t ri = r[i];
		r[i] = ri - c;
		c = (ri < c) ? 1u : 0u;
	}
	return c;
}

// whether a[0, n) >= b[0, n)
inline constexpr bool geq_n(const limb_t* a, const limb_t* b, std::size_t n) noexcept {
	for (std::size_t i = n; i-- > 0;) {
		if (a[i] != b[i]) return a[i] > b[i];
	}
	return true;
}

// q[0, nn - 1) = the low limbs of floor(np[0, nn) / d), the remainder replaces np[0]; returns
// the quotient's top limb, 0 or 1.  d normalized, v = invert_limb(d).
// divrem_1 without a reciprocal, for a quotient of a few limbs where the build divides in
// hardware (udiv_qrnnd): the same quotient and remainder
inline constexpr limb_t divrem_1_short(limb_t* q, limb_t* np, std::size_t nn, limb_t d) noexcept {
	limb_t r  = np[nn - 1];
	limb_t qh = 0;
	if (r >= d) { r -= d; qh = 1; }
	for (std::size_t i = nn - 1; i-- > 0;) udiv_qrnnd(q[i], r, r, np[i], d);
	np[0] = r;
	return qh;
}

inline constexpr limb_t divrem_1(limb_t* q, limb_t* np, std::size_t nn, limb_t d, limb_t v) noexcept {
	limb_t r  = np[nn - 1];
	limb_t qh = 0;
	if (r >= d) { r -= d; qh = 1; }
	for (std::size_t i = nn - 1; i-- > 0;) udiv_qr_2by1(q[i], r, r, np[i], d, v);
	np[0] = r;
	return qh;
}

// Schoolbook division: q[0, nn - dn) = the low limbs of floor(np[0, nn) / dp[0, dn)), the
// remainder replaces np[0, dn); returns the quotient's top limb, 0 or 1.  dn >= 2, dp normalized,
// v = invert_pi1 of its top two limbs.
inline constexpr limb_t sbpi1_div_qr(limb_t* q, limb_t* np, std::size_t nn, const limb_t* dp, std::size_t dn, limb_t v) noexcept {
	const std::size_t qn = nn - dn;
	limb_t* top = np + qn;
	limb_t  qh  = 0;
	if (geq_n(top, dp, dn)) { sub_n(top, top, dp, dn); qh = 1; }
	const limb_t d1 = dp[dn - 1], d0 = dp[dn - 2];
	limb_t n1 = np[nn - 1];
	for (std::size_t j = qn; j-- > 0;) {
		// divide np[j, j + dn] by dp; n1 is np[j + dn], already reduced below d
		limb_t* w = np + j;
		limb_t qj;
		if (n1 == d1 && w[dn - 1] == d0) {
			// the top two limbs equal the divisor's, so the quotient limb is B - 1: the borrow
			// out of w[0, dn) cancels n1
			qj = ~limb_t(0);
			submul_1(w, dp, dn, qj);
			n1 = w[dn - 1];
		}
		else {
			limb_t n0;
			udiv_qr_3by2(qj, n1, n0, n1, w[dn - 1], w[dn - 2], d1, d0, v);
			const limb_t cy  = submul_1(w, dp, dn - 2, qj);
			const limb_t cy1 = (n0 < cy) ? 1u : 0u;
			n0 -= cy;
			const limb_t cy2 = (n1 < cy1) ? 1u : 0u;
			n1 -= cy1;
			w[dn - 2] = n0;
			if (cy2 != 0u) {   // the estimate was one too large: add the divisor back
				n1 += d1 + add_n(w, w, dp, dn - 1);
				--qj;
			}
		}
		q[j] = qj;
	}
	np[dn - 1] = n1;
	return qh;
}

// sbpi1_div_qr without the reciprocal, for a quotient of a few limbs where the build divides in
// hardware (#1826): each quotient limb estimated from the top two limbs by the divisor's top limb
// (udiv_qrnnd), corrected by the divisor's second limb at most twice, then a multiply-subtract and
// at most one add-back, as Knuth's Algorithm D (TAOCP 2, 4.3.1) on 64-bit digits. A reciprocal
// of two limbs costs as much as several such steps. The same quotient and remainder. Always
// inlined, so that a caller's fixed sizes unroll it: clang left it a call (#1830).
#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
inline constexpr limb_t sb_div_qr_short(limb_t* q, limb_t* np, std::size_t nn, const limb_t* dp, std::size_t dn) noexcept {
	const std::size_t qn = nn - dn;
	limb_t* top = np + qn;
	limb_t  qh  = 0;
	if (geq_n(top, dp, dn)) { sub_n(top, top, dp, dn); qh = 1; }
	const limb_t d1 = dp[dn - 1], d0 = dp[dn - 2];
	for (std::size_t j = qn; j-- > 0;) {
		limb_t* w = np + j;   // divide w[0, dn] by dp; w[dn] B + w[dn - 1] < d1 B + d0
		const limb_t n2 = w[dn], n1 = w[dn - 1], n0 = w[dn - 2];
		limb_t qj = ~limb_t(0), rj = 0;
		bool   refine = true;
		if (n2 < d1) {
			udiv_qrnnd(qj, rj, n2, n1, d1);
		}
		else {   // n2 == d1: the estimate B - 1, with remainder n1 + d1, which may pass B
			rj = n1 + d1;
			refine = (rj >= d1);
		}
		// while qj d0 > rj B + n0: the estimate is at most two too large
		for (int k = 0; refine && k < 2; ++k) {
			limb_t ph = 0, pl = 0;
			umul(qj, d0, ph, pl);
			if (ph < rj || (ph == rj && pl <= n0)) break;
			--qj;
			rj += d1;
			refine = (rj >= d1);   // a remainder past B ends the test: qj is then right
		}
		const limb_t borrow = submul_1(w, dp, dn, qj);
		if (n2 < borrow) {   // still one too large: add the divisor back
			add_n(w, w, dp, dn);
			--qj;
		}
		w[dn] = 0u;
		q[j] = qj;
	}
	return qh;
}

// the scratch limbs div_qr needs for a divisor of dn limbs
constexpr std::size_t div_scratch_limbs(std::size_t dn) noexcept { return dn + scratch_limbs(dn); }

// sbpi1_div_qr for np[0, 2n) and dp[0, n) by divide-and-conquer: the high half of the quotient
// from the top 3n/2 limbs and the high half of the divisor, corrected by the product of that
// half with the low half of the divisor, then the low half the same way.  tp holds
// div_scratch_limbs(n) limbs.
inline constexpr limb_t dcpi1_div_qr_n(limb_t* q, limb_t* np, const limb_t* dp, std::size_t n, limb_t v, limb_t* tp) noexcept {
	const std::size_t lo = n / 2;
	const std::size_t hi = n - lo;
	limb_t qh = (hi < dc_div_threshold) ? sbpi1_div_qr(q + lo, np + 2 * lo, 2 * hi, dp + lo, hi, v)
	                                    : dcpi1_div_qr_n(q + lo, np + 2 * lo, dp + lo, hi, v, tp);
	mul(tp, q + lo, hi, dp, lo, tp + n);
	limb_t cy = sub_n(np + lo, np + lo, tp, n);
	if (qh != 0u) cy += sub_n(np + n, np + n, dp, lo);
	while (cy != 0u) {   // the estimate was too large: at most twice
		qh -= sub_1(q + lo, hi, 1u);
		cy -= add_n(np + lo, np + lo, dp, n);
	}
	const limb_t ql = (lo < dc_div_threshold) ? sbpi1_div_qr(q, np + hi, 2 * lo, dp + hi, lo, v)
	                                          : dcpi1_div_qr_n(q, np + hi, dp + hi, lo, v, tp);
	mul(tp, dp, hi, q, lo, tp + n);
	cy = sub_n(np, np, tp, n);
	if (ql != 0u) cy += sub_n(np + lo, np + lo, dp, hi);
	while (cy != 0u) {
		sub_1(q, lo, 1u);
		cy -= add_n(np, np, dp, n);
	}
	return qh;
}

// A quotient shorter than the divisor, qn = nn - dn < dn: divide the top 2 qn limbs of np by the
// top qn limbs of dp, then subtract the quotient times the divisor's remaining limbs and correct.
inline constexpr limb_t dcpi1_div_qr_short(limb_t* q, limb_t* np, std::size_t nn, const limb_t* dp, std::size_t dn, limb_t v, limb_t* tp) noexcept {
	const std::size_t qn = nn - dn;
	const std::size_t in = dn - qn;
	limb_t qh = dcpi1_div_qr_n(q, np + in, dp + in, qn, v, tp);
	mul(tp, q, qn, dp, in, tp + dn);   // dn limbs
	limb_t cy = sub_n(np, np, tp, dn);
	if (qh != 0u) cy += sub_n(np + qn, np + qn, dp, in);
	while (cy != 0u) {
		qh -= sub_1(q, qn, 1u);
		cy -= add_n(np, np, dp, dn);
	}
	return qh;
}

// q[0, nn - dn) = the low limbs of floor(np[0, nn) / dp[0, dn)), the remainder replaces
// np[0, dn); returns the quotient's top limb, 0 or 1.  nn >= dn >= 1, dp normalized (its top bit
// set) and overlapping neither np nor q, s holds div_scratch_limbs(dn) limbs.
inline constexpr limb_t div_qr(limb_t* q, limb_t* np, std::size_t nn, const limb_t* dp, std::size_t dn, limb_t* s) noexcept {
	const std::size_t qn = nn - dn;
	// a quotient of one or two limbs by one or two divides faster in hardware than through a
	// reciprocal (#1826)
	if (hardware_division && qn <= 2 && dn <= 2) {   // by measurement: a reciprocal pays from there
		return (dn == 1) ? divrem_1_short(q, np, nn, dp[0]) : sb_div_qr_short(q, np, nn, dp, dn);
	}
	if (dn == 1) return divrem_1(q, np, nn, dp[0], invert_limb(dp[0]));
	const limb_t      v  = invert_pi1(dp[dn - 1], dp[dn - 2]);
	if (dn < dc_div_threshold || qn < dc_div_threshold) return sbpi1_div_qr(q, np, nn, dp, dn, v);
	if (qn < dn) return dcpi1_div_qr_short(q, np, nn, dp, dn, v, s);
	// a long quotient: a first block of qn mod dn limbs (or dn), then blocks of dn limbs, each
	// dividing the 2 dn limbs below the remainder of the last
	const std::size_t first = (qn % dn == 0) ? dn : qn % dn;
	std::size_t off = qn - first;
	limb_t qh;
	if (first == dn)                   qh = dcpi1_div_qr_n(q + off, np + off, dp, dn, v, s);
	else if (first < dc_div_threshold) qh = sbpi1_div_qr(q + off, np + off, dn + first, dp, dn, v);
	else                               qh = dcpi1_div_qr_short(q + off, np + off, dn + first, dp, dn, v, s);
	while (off > 0) {
		off -= dn;
		dcpi1_div_qr_n(q + off, np + off, dp, dn, v, s);
	}
	return qh;
}

// Approximate quotients (#1810), as GMP's divappr: the quotient alone, from below by at most
// divappr_error units of its last limb above the exact floor(np / dp). dcpi1_divappr_n forms
// the high half of the quotient exactly, as dcpi1_div_qr_n does, and the low half on the
// divisor's top lo limbs, without the product of that half by the divisor's other limbs or the
// correction it decides. Dividing by the truncated divisor can only raise a quotient, by at most
// 2, as the exact code's correction loop shows; the low half's own recursion adds its error, so
// each level of the recursion adds 2. A low half that reaches B^lo is clamped to B^lo - 1, the
// largest the exact one can be. np is overwritten.
inline constexpr limb_t dcpi1_divappr_n(limb_t* q, limb_t* np, const limb_t* dp, std::size_t n, limb_t v, limb_t* tp) noexcept {
	const std::size_t lo = n / 2;
	const std::size_t hi = n - lo;
	limb_t qh = (hi < dc_div_threshold) ? sbpi1_div_qr(q + lo, np + 2 * lo, 2 * hi, dp + lo, hi, v)
	                                    : dcpi1_div_qr_n(q + lo, np + 2 * lo, dp + lo, hi, v, tp);
	mul(tp, q + lo, hi, dp, lo, tp + n);
	limb_t cy = sub_n(np + lo, np + lo, tp, n);
	if (qh != 0u) cy += sub_n(np + n, np + n, dp, lo);
	while (cy != 0u) {
		qh -= sub_1(q + lo, hi, 1u);
		cy -= add_n(np + lo, np + lo, dp, n);
	}
	const limb_t ql = (lo < dc_div_threshold) ? sbpi1_div_qr(q, np + hi, 2 * lo, dp + hi, lo, v)
	                                          : dcpi1_divappr_n(q, np + hi, dp + hi, lo, v, tp);
	if (ql != 0u) {
		for (std::size_t i = 0; i < lo; ++i) q[i] = ~limb_t(0);
	}
	return qh;
}

// the bound on dcpi1_divappr_n's excess for a quotient of n limbs: 2 a level
constexpr std::size_t divappr_n_error(std::size_t n) noexcept {
	return (n < dc_div_threshold) ? 0u : 2u + divappr_n_error(n / 2);
}

// q[0, nn - dn) + B^(nn - dn) times the returned limb = Q, floor(np / dp) <= Q <= floor(np / dp)
// + divappr_error(nn, dn), for nn >= dn >= 1, dp normalized and overlapping neither np nor q;
// np is overwritten, s holds div_scratch_limbs(dn) limbs. Shapes the approximation does not
// serve divide exactly.
inline constexpr limb_t div_appr(limb_t* q, limb_t* np, std::size_t nn, const limb_t* dp, std::size_t dn, limb_t* s) noexcept {
	const std::size_t qn = nn - dn;
	if (dn < dc_div_threshold || qn < dc_div_threshold) return div_qr(q, np, nn, dp, dn, s);
	const limb_t v = invert_pi1(dp[dn - 1], dp[dn - 2]);
	if (qn == dn) return dcpi1_divappr_n(q, np, dp, dn, v, s);
	if (qn > dn) {
		// a long quotient (#1811), as div_qr divides it: a first block of qn mod dn limbs (or dn)
		// and blocks of dn limbs below it, exact, but for the lowest block, approximate
		const std::size_t first = (qn % dn == 0) ? dn : qn % dn;
		std::size_t off = qn - first;
		limb_t qh;
		if (first == dn)                   qh = dcpi1_div_qr_n(q + off, np + off, dp, dn, v, s);
		else if (first < dc_div_threshold) qh = sbpi1_div_qr(q + off, np + off, dn + first, dp, dn, v);
		else                               qh = dcpi1_div_qr_short(q + off, np + off, dn + first, dp, dn, v, s);
		while (off > dn) {
			off -= dn;
			dcpi1_div_qr_n(q + off, np + off, dp, dn, v, s);
		}
		if (dcpi1_divappr_n(q, np, dp, dn, v, s) != 0u) {   // the lowest block reached B^dn: clamped
			for (std::size_t i = 0; i < dn; ++i) q[i] = ~limb_t(0);
		}
		return qh;
	}
	// a quotient shorter than the divisor: the top 2 qn limbs over the divisor's top qn, as
	// dcpi1_div_qr_short, without the product by the divisor's other limbs; at most 2 more
	const std::size_t in = dn - qn;
	return dcpi1_divappr_n(q, np + in, dp + in, qn, v, s);
}

constexpr std::size_t divappr_error(std::size_t nn, std::size_t dn) noexcept {
	const std::size_t qn = nn - dn;
	if (dn < dc_div_threshold || qn < dc_div_threshold) return 0u;
	if (qn > dn) return divappr_n_error(dn);   // the lowest block's
	return (qn == dn) ? divappr_n_error(qn) : 2u + divappr_n_error(qn);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Square root with remainder (#1763): Zimmermann's Karatsuba square root (INRIA RR-3805, 1999),
// as GMP's mpn_sqrtrem.  With N = a3 b^3 + a2 b^2 + a1 b + a0, (s', r') = sqrtrem(a3 b + a2),
// (q, u) = divrem(r' b + a1, 2 s'), s = s' b + q and r = u b + a0 - q^2, then one correction
// when r < 0: the square root costs one division and one square of half its length per level,
// and both S = floor(sqrt(N)) and the remainder are exact.

// floor(sqrt(a)) for a >= 2^62, r = a - s^2: a double estimate, corrected. In constant
// evaluation, where std::sqrt is not usable (#1819), Newton's iteration on integers from 2^32 - 1,
// above the root, which descends to it
inline constexpr limb_t sqrtrem1(limb_t a, limb_t& r) noexcept {
	limb_t s = 0xFFFF'FFFFu;
	if (std::is_constant_evaluated()) {
		for (limb_t t = (s + a / s) / 2u; t < s; t = (s + a / s) / 2u) s = t;
	}
	else {
		s = static_cast<limb_t>(std::sqrt(static_cast<double>(a)));
		if (s > 0xFFFF'FFFFu) s = 0xFFFF'FFFFu;   // the double rounded up to 2^32
	}
	while (s * s > a) --s;
	while (s < 0xFFFF'FFFFu && (s + 1) * (s + 1) <= a) ++s;
	r = a - s * s;
	return s;
}

// sp[0] = floor(sqrt(np[0, 2))) for np[1] >= 2^62; the remainder, at most 2 sp[0], is rp[0]
// and the returned bit above it.  rp may be np.  The same step as dc_sqrtrem on 32-bit halves.
inline constexpr limb_t sqrtrem2(limb_t* sp, limb_t* rp, const limb_t* np) noexcept {
	const limb_t n0 = np[0];
	limb_t r1;
	const limb_t s1 = sqrtrem1(np[1], r1);   // s1 in [2^31, 2^32), r1 <= 2 s1
	limb_t k = 0;
	while (r1 >= s1) { r1 -= s1; ++k; }      // k s1 2^32 / (2 s1) = k 2^31 exactly
	const limb_t t = (r1 << 32) | (n0 >> 32);
	const limb_t d = 2 * s1;
	const limb_t q = t / d + (k << 31);      // at most 2^32
	const limb_t u = t % d;                  // below 2^33
	// r = u 2^32 + (n0 mod 2^32) - q^2 as a low limb and a signed top
	limb_t  rlo = (u << 32) | (n0 & 0xFFFF'FFFFu);
	int64_t rhi = static_cast<int64_t>(u >> 32);
	if (q == (limb_t(1) << 32)) {
		rhi -= 1;
	}
	else {
		const limb_t qq = q * q;
		rhi -= (rlo < qq) ? 1 : 0;
		rlo -= qq;
	}
	// s = s1 2^32 + q, which reaches 2^64 only when the correction below takes it back
	limb_t slo = (s1 << 32) + q;
	limb_t shi = (slo < (s1 << 32)) ? 1u : 0u;
	if (rhi < 0) {   // r += 2 s - 1, s -= 1, as r += s, --s, r += s
		rlo += slo;
		rhi += static_cast<int64_t>(shi + ((rlo < slo) ? 1u : 0u));
		shi -= (slo == 0u) ? 1u : 0u;
		--slo;
		rlo += slo;
		rhi += static_cast<int64_t>(shi + ((rlo < slo) ? 1u : 0u));
	}
	sp[0] = slo;
	rp[0] = rlo;
	return static_cast<limb_t>(rhi);
}

// sp[0, 2) = floor(sqrt(np[0, 4))) for np[3] >= 2^62; the remainder, at most 2 S, is rp[0, 2)
// and the returned bit above it (#1828). dc_sqrtrem's one level for n = 2 in registers: the
// root s' of the top two limbs by sqrtrem2, the next limb of the root from one division of
// (r' B + np[1]) by 2 s', and the remainder corrected once.
inline constexpr limb_t sqrtrem4(limb_t* sp, limb_t* rp, const limb_t* np) noexcept {
	limb_t s1 = 0, r1 = 0;
	limb_t q = sqrtrem2(&s1, &r1, np + 2);   // s' = s1, r' = q B + r1 <= 2 s'
	if (q != 0u) r1 -= s1;                    // r' - s' < s': divide by s', then halve
	limb_t qh = 0;
	if (r1 >= s1) { r1 -= s1; qh = 1u; }
	limb_t q0 = 0, u = 0;
	udiv_qrnnd(q0, u, r1, np[1], s1);         // (r' B + np[1]) / s' = (q + qh) B + q0, remainder u
	q += qh;
	limb_t c = q0 & 1u;                        // / (2 s') = 2 Q + c: the remainder by 2 s' gains c s'
	limb_t s0 = (q0 >> 1) | (q << 63);
	q >>= 1;
	if (c != 0u) {
		u += s1;
		c = (u < s1) ? 1u : 0u;
	}
	// R = c B^2 + u B + np[0] - s0^2
	limb_t hh = 0, ll = 0;
	umul(s0, s0, hh, ll);
	limb_t r0 = np[0] - ll;
	limb_t borrow = (np[0] < ll) ? 1u : 0u;
	limb_t rh = u - hh;
	const limb_t borrow1 = (u < hh) ? 1u : 0u;
	const limb_t borrow2 = (rh < borrow) ? 1u : 0u;
	rh -= borrow;
	borrow = borrow1 + borrow2;
	int64_t cc = static_cast<int64_t>(c) - static_cast<int64_t>(q + borrow);
	if (cc < 0) {   // R < 0: R += 2 S - 1, S -= 1
		s1 += q;
		q = (s1 < q) ? 1u : 0u;
		// R += 2 S, S = s1 B + s0
		const limb_t d0 = s0 << 1, d1 = (s1 << 1) | (s0 >> 63);
		const limb_t top = s1 >> 63;
		r0 += d0;
		limb_t carry = (r0 < d0) ? 1u : 0u;
		const limb_t t1 = rh + d1;
		const limb_t k1 = (t1 < d1) ? 1u : 0u;
		rh = t1 + carry;
		carry = k1 + ((rh < carry) ? 1u : 0u);
		cc += static_cast<int64_t>(carry + top + 2 * q);
		// R -= 1
		if (r0 == 0u) {
			if (rh == 0u) --cc;
			--rh;
		}
		--r0;
		// S -= 1
		if (s0 == 0u) --s1;
		--s0;
	}
	sp[0] = s0;
	sp[1] = s1;
	rp[0] = r0;
	rp[1] = rh;
	return static_cast<limb_t>(cc);
}

// sp[0, 2) = S within 2^15 units of floor(sqrt(N)), N = np[0, 4) with np[3] >= 2^62, without the
// remainder (#1828). s' = floor(sqrt(N')) and r' = N' - s'^2 for the top two limbs N' by
// sqrtrem2; sqrt(N) lies in [s' B + Q - 1, s' B + Q] for Q = (r' B + np[1] + np[0] / B) / (2 s')
// (dc_sqrtrem's step, whose one correction the -1 is), and Q < B + 1. Q is formed in double
// precision, from a reciprocal of sqrt(N') computed alongside sqrtrem2 in place of a second
// division: at most nine roundings of 2^-53 each and the 2^-63 between sqrt(N') and s' keep it
// within 2^14.2 of its value. Returns false, with sp unset, where Q comes within 2^16 of B and S
// might need s' + 1, and in constant evaluation, where std::sqrt is not usable (#1819): the
// caller's exact root takes over.
inline constexpr bool sqrt_approx4(limb_t* sp, const limb_t* np) noexcept {
	if (std::is_constant_evaluated()) return false;
	constexpr double two64 = 18446744073709551616.0;   // 2^64
	limb_t s1 = 0, r1 = 0;
	const limb_t c = sqrtrem2(&s1, &r1, np + 2);
	const double y = 0.5 / std::sqrt(static_cast<double>(np[3]) * two64 + static_cast<double>(np[2]));
	const double q = ((static_cast<double>(r1) + static_cast<double>(c) * two64) * two64 + static_cast<double>(np[1])) * y;
	if (!(q < two64 - 65536.0)) return false;
	sp[0] = static_cast<limb_t>(q);
	sp[1] = s1;
	return true;
}

// the scratch limbs sqrtrem needs for a root of n limbs
constexpr std::size_t sqrt_scratch_limbs(std::size_t n) noexcept { return n + 2 + div_scratch_limbs(n); }

// sp[0, n) = floor(sqrt(np[0, 2n))) for n >= 2 and np[2n - 1] >= 2^62; the remainder replaces
// np[0, n), with the returned limb, 0 or 1, above it.  s holds sqrt_scratch_limbs(n) limbs.
inline constexpr limb_t dc_sqrtrem(limb_t* sp, limb_t* np, std::size_t n, limb_t* s) noexcept {
	const std::size_t l = n / 2;
	const std::size_t h = n - l;
	// (s', r') for the top 2h limbs: s' in sp[l, n), r' in np[2l, 2l + h) and q
	limb_t q = (h == 1) ? sqrtrem2(sp + l, np + 2 * l, np + 2 * l) : dc_sqrtrem(sp + l, np + 2 * l, h, s);
	if (q != 0u) sub_n(np + 2 * l, np + 2 * l, sp + l, h);   // r' - s' < s': divide by s', then halve
	limb_t* qp = s;   // l + 1 limbs
	qp[l] = div_qr(qp, np + l, n, sp + l, h, s + l + 1);
	q += qp[l];
	limb_t c = qp[0] & 1u;   // (r' b + a1) / s' = 2 q + c: the remainder by 2 s' gains c s'
	for (std::size_t i = 0; i + 1 < l; ++i) sp[i] = (qp[i] >> 1) | (qp[i + 1] << 63);
	sp[l - 1] = (qp[l - 1] >> 1) | (q << 63);
	q >>= 1;
	if (c != 0u) c = add_n(np + l, np + l, sp + l, h);
	mul_n(np + n, sp, sp, l, s, true);   // q^2 above the remainder, where a3 and a2 were
	const limb_t b = q + sub_n(np, np, np + n, 2 * l);
	int64_t cc = static_cast<int64_t>(c) - static_cast<int64_t>((l == h) ? b : sub_1(np + 2 * l, 1, b));
	if (cc < 0) {   // r < 0: r += 2 s - 1, s -= 1
		q = add_1(sp + l, h, q);
		cc += static_cast<int64_t>(addmul_1(np, sp, n, 2u) + 2 * q);
		cc -= static_cast<int64_t>(sub_1(np, n, 1u));
		sub_1(sp, n, 1u);
	}
	return static_cast<limb_t>(cc);
}

// sp[0, n) = floor(sqrt(np[0, 2n))) for np[2n - 1] >= 2^62; the remainder replaces np[0, n),
// with the returned limb, 0 or 1, above it.  s holds sqrt_scratch_limbs(n) limbs.
inline constexpr limb_t sqrtrem(limb_t* sp, limb_t* np, std::size_t n, limb_t* s) noexcept {
	return (n == 1) ? sqrtrem2(sp, np, np) : dc_sqrtrem(sp, np, n, s);
}

// the bound sqrt_approx keeps its root within, in units of its last limb: the one correction
// the exact root may need, and half the approximate quotient's excess, rounded up
constexpr std::size_t sqrt_approx_error(std::size_t n) noexcept {
	return (n == 1) ? 1u : 2u + divappr_error(n, n - n / 2) / 2u;
}

// sp[0, n) = S with floor(sqrt(N)) <= S <= floor(sqrt(N)) + sqrt_approx_error(n), N = np[0, 2n),
// np[2n - 1] >= 2^62, and S < B^n (#1810): dc_sqrtrem's top level without the remainder.
// S = s' B^l + Q, the half-length root s' formed exactly as there and Q from div_appr's
// approximate quotient of (r' B^l + a1) by s', halved: Q is the exact floor((r' B^l + a1) / (2 s'))
// or above it by at most half the quotient's excess, rounded up. With the exact Q, the remainder
// r = u B^l + a0 - Q^2 decides the one correction, which only lowers S, so S is never too small.
// Skipped: the remainders of the division's low halves, the square Q^2 and the correction. An S
// that would reach B^n is B^n - 1, the floor's largest value. np is overwritten; s holds
// sqrt_scratch_limbs(n) limbs.
inline constexpr void sqrt_approx(limb_t* sp, limb_t* np, std::size_t n, limb_t* s) noexcept {
	if (n == 1) {
		sqrtrem2(sp, np, np);
		return;
	}
	const std::size_t l = n / 2;
	const std::size_t h = n - l;
	limb_t q = (h == 1) ? sqrtrem2(sp + l, np + 2 * l, np + 2 * l) : dc_sqrtrem(sp + l, np + 2 * l, h, s);
	if (q != 0u) sub_n(np + 2 * l, np + 2 * l, sp + l, h);
	limb_t* qp = s;
	qp[l] = div_appr(qp, np + l, n, sp + l, h, s + l + 1);
	q += qp[l];
	for (std::size_t i = 0; i + 1 < l; ++i) sp[i] = (qp[i] >> 1) | (qp[i + 1] << 63);
	sp[l - 1] = (qp[l - 1] >> 1) | (q << 63);
	q >>= 1;
	if (q != 0u && add_1(sp + l, h, q) != 0u) {   // S = B^n or above: its floor is B^n - 1
		for (std::size_t i = 0; i < n; ++i) sp[i] = ~limb_t(0);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Cube root with remainder (#1800): the recursion of Zimmermann's square root (dc_sqrtrem above)
// carried over to the cube. With N = N' B^(3l) + A2 B^(2l) + A0, (S', R') = cbrtrem(N') and
// (Q, U) = divrem(R' B^l + A2, 3 S'^2), the root is S = S' B^l + Q and its remainder
//     R = N - S^3 = U B^(2l) + A0 - Q^2 (3 S' B^l + Q),
// since S^3 - S'^3 B^(3l) = 3 S'^2 Q B^(2l) + Q^2 (3 S' B^l + Q). Q drops the last two terms,
// so it is never too small and at most a few units too large (cbrt_overshoot: measured at most 2,
// and 0 when the root has an odd number of limbs, l < h); each unit is
// corrected by R += 3 S^2 - 3 S + 1, S -= 1, with 3 S^2 = 3 S'^2 B^(2l) + 6 S' Q B^l + 3 Q^2
// from the parts already formed. A level costs a square of h limbs, a division of an l-limb
// quotient, and a product of 2l by n limbs for the remainder. The top level can skip the
// remainder: cbrt rounds from S alone unless S lies within cbrt_overshoot of a rounding boundary.

// r[0, 3) = s^3
inline constexpr void cube_1(limb_t* r, limb_t s) noexcept {
	limb_t h, l, a1, a0, b1, b0;
	umul(s, s, h, l);
	umul(l, s, a1, a0);
	umul(h, s, b1, b0);
	r[0] = a0;
	r[1] = a1 + b0;
	r[2] = b1 + ((r[1] < b0) ? 1u : 0u);
}

// r[0, n) = a[0, n) << c, 0 < c < 64, returns the bits shifted out of the top; r may be a
inline constexpr limb_t lshift(limb_t* r, const limb_t* a, std::size_t n, unsigned c) noexcept {
	limb_t out = a[n - 1] >> (64u - c);
	for (std::size_t i = n - 1; i > 0; --i) r[i] = (a[i] << c) | (a[i - 1] >> (64u - c));
	r[0] = a[0] << c;
	return out;
}

// r[0, n) = a[0, n) >> c, 0 < c < 64; r may be a
inline constexpr void rshift(limb_t* r, const limb_t* a, std::size_t n, unsigned c) noexcept {
	for (std::size_t i = 0; i + 1 < n; ++i) r[i] = (a[i] >> c) | (a[i + 1] << (64u - c));
	r[n - 1] = a[n - 1] >> c;
}

// r[0, n) = the low n limbs of floor(a[0, na) 2^shift), for a shift of either sign; returns
// whether the bits a right shift drops below r are nonzero (#1802). r overlaps not a.
inline constexpr bool load_window(limb_t* r, std::size_t n, const limb_t* a, std::size_t na, std::int64_t shift) noexcept {
	const std::int64_t m = static_cast<std::int64_t>(na);
	auto at = [a, m](std::int64_t j) -> limb_t { return (j >= 0 && j < m) ? a[j] : 0u; };
	const std::int64_t q = (shift >= 0) ? -((shift + 63) / 64) : (-shift) / 64;   // floor(-shift / 64)
	const unsigned     c = static_cast<unsigned>(-shift - 64 * q);                  // in [0, 64)
	for (std::size_t k = 0; k < n; ++k) {
		const std::int64_t j = q + static_cast<std::int64_t>(k);   // r[k] holds a[j] >> c and a[j + 1] << (64 - c)
		r[k] = (c == 0u) ? at(j) : ((at(j) >> c) | (at(j + 1) << (64u - c)));
	}
	if (shift >= 0) return false;
	for (std::int64_t j = 0; j < q && j < m; ++j) {
		if (a[j] != 0u) return true;
	}
	return c != 0u && q < m && (a[q] & ((limb_t(1) << c) - 1u)) != 0u;
}

// sp[0] = floor(cbrt(np[0, 3))) for np[2] >= 2^61, rp[0, 3) = the remainder: a double estimate,
// within about 2^12 of the root, one Newton step in double, and an exact correction. In constant
// evaluation, where std::cbrt is not usable (#1819), a binary search over the root's 64 bits,
// which lies in [2^63, 2^64) since N >= 2^189.
inline constexpr void cbrtrem_1(limb_t* sp, limb_t* rp, const limb_t* np) noexcept {
	if (std::is_constant_evaluated()) {
		limb_t s = limb_t(1) << 63;   // s^3 <= N
		limb_t t[3]{};
		for (limb_t bit = limb_t(1) << 62; bit != 0u; bit >>= 1) {
			cube_1(t, s | bit);
			if (geq_n(np, t, 3)) s |= bit;
		}
		cube_1(t, s);
		sp[0] = s;
		sub_n(rp, np, t, 3);
		return;
	}
	const double two64 = 18446744073709551616.0;
	const double c = std::cbrt(std::ldexp(static_cast<double>(np[2]), 128) + std::ldexp(static_cast<double>(np[1]), 64));
	limb_t s = (c >= two64) ? ~limb_t(0) : static_cast<limb_t>(c);
	limb_t t[3];
	cube_1(t, s);
	const bool below = !geq_n(np, t, 3);   // N < s^3
	limb_t d[3];
	if (below) sub_n(d, t, np, 3); else sub_n(d, np, t, 3);
	const double step = (std::ldexp(static_cast<double>(d[2]), 128) + std::ldexp(static_cast<double>(d[1]), 64) + static_cast<double>(d[0])) /
	                    (3.0 * static_cast<double>(s) * static_cast<double>(s));
	const limb_t k = static_cast<limb_t>(step);
	if (below) s -= k; else s = (~limb_t(0) - s < k) ? ~limb_t(0) : s + k;
	for (;;) {   // s^3 <= N
		cube_1(t, s);
		if (geq_n(np, t, 3)) break;
		--s;
	}
	while (s != ~limb_t(0)) {   // (s + 1)^3 > N
		limb_t u[3];
		cube_1(u, s + 1);
		if (!geq_n(np, u, 3)) break;
		++s;
		t[0] = u[0]; t[1] = u[1]; t[2] = u[2];
	}
	sp[0] = s;
	sub_n(rp, np, t, 3);
}

// the scratch limbs cbrtrem needs for a root of n limbs: a level's own 5n + 8, and below them
// the larger of the recursion's, the division's (17 n + 162) and the remainder's (25 n + 152)
constexpr std::size_t cbrt_scratch_limbs(std::size_t n) noexcept { return 64 * n + 512; }

// the most units the root without its remainder can lie above floor(cbrt(N)): Q's excess is
// below Q^2 / (S' B^l) + Q^3 / (3 S'^2 B^(2l)) + R / (3 S'^2 B^(2l)) < 2 + 1 + 1
inline constexpr limb_t cbrt_overshoot = 4;

// sp[0, n) = floor(cbrt(np[0, 3n))) for np[3n - 1] >= 2^61, and rp[0, 2n + 1) = N - S^3 when
// `exact`; without it, S is the root plus 0 to cbrt_overshoot units and rp is left alone.
// s holds cbrt_scratch_limbs(n) limbs.
inline constexpr void cbrtrem(limb_t* sp, limb_t* rp, const limb_t* np, std::size_t n, limb_t* s, bool exact = true) noexcept {
	if (n == 1) {   // exact either way
		limb_t r[3];
		cbrtrem_1(sp, exact ? rp : r, np);
		return;
	}
	const std::size_t l = n / 2;
	const std::size_t h = n - l;
	limb_t* Rh = s;                    // 2h + 1: R'
	limb_t* D  = Rh + 2 * h + 1;       // 2h + 1: 3 S'^2
	limb_t* X  = D + 2 * h + 1;        // l + 2h + 2: R' B^l + A2, then the remainder of the division
	limb_t* q  = X + l + 2 * h + 2;    // l + 2h + 2: the quotient
	limb_t* w  = q + l + 2 * h + 2;
	limb_t* const Sh = sp + l;         // S'
	cbrtrem(Sh, Rh, np + 3 * l, h, w, true);

	// D = 3 S'^2, normalized for the division by a shift of c bits
	mul_n(D, Sh, Sh, h, w, true);
	D[2 * h] = mul_1(D, D, 2 * h, 3u);
	const std::size_t dn = (D[2 * h] != 0u) ? 2 * h + 1 : 2 * h;
	const unsigned c = static_cast<unsigned>(std::countl_zero(D[dn - 1]));
	for (std::size_t i = 0; i < l; ++i) X[i] = np[2 * l + i];
	for (std::size_t i = 0; i < 2 * h + 1; ++i) X[l + i] = Rh[i];
	std::size_t xn = l + 2 * h + 1;
	X[xn] = 0;
	if (c != 0u) {
		lshift(D, D, dn, c);
		X[xn] = lshift(X, X, xn, c);
	}
	++xn;
	const limb_t qtop = div_qr(q, X, xn, D, dn, w);
	const std::size_t qn = xn - dn;
	q[qn] = qtop;
	if (c != 0u) {   // U and D back to scale
		rshift(X, X, dn, c);
		rshift(D, D, dn, c);
	}
	limb_t* U = X;   // dn limbs, and one more for the clamp below
	U[dn] = 0;
	// Q < B^l: the root's low part, since S < (S' + 1) B^l. A larger Q is reduced to B^l - 1,
	// and U gains D for each unit, so that U = X - D Q still holds.
	std::size_t qlen = qn + 1;
	while (qlen > l && q[qlen - 1] == 0u) --qlen;
	if (qlen > l) {   // Q = B^l + q[0] at most: k = q[0] + 1 units above B^l - 1
		U[dn] = addmul_1(U, D, dn, q[0] + 1u);
		for (std::size_t i = 0; i < l; ++i) q[i] = ~limb_t(0);
	}
	for (std::size_t i = 0; i < l; ++i) sp[i] = q[i];
	if (!exact) return;

	// R = U B^(2l) + A0 - Q^2 (3 S' B^l + Q), in 2n + 3 limbs, two's complement
	const std::size_t rn = 2 * n + 3;
	limb_t* R  = w;                    // rn
	limb_t* Q2 = R + rn;               // 2l: Q^2
	limb_t* T  = Q2 + 2 * l;           // n + 1: 3 S' B^l + Q
	limb_t* P  = T + n + 1;            // 2l + n + 1: Q^2 T
	limb_t* G  = P + 2 * l + n + 1;    // rn: 3 S^2 - 3 S + 1
	limb_t* SQ = G + rn;               // n: S' Q
	limb_t* ww = SQ + n;
	for (std::size_t i = 0; i < rn; ++i) R[i] = 0;
	for (std::size_t i = 0; i < 2 * l; ++i) R[i] = np[i];
	add_n(R + 2 * l, R + 2 * l, U, dn + 1);
	mul_n(Q2, sp, sp, l, ww, true);
	for (std::size_t i = 0; i < l; ++i) T[i] = sp[i];
	T[n] = mul_1(T + l, Sh, h, 3u);
	mul(P, Q2, 2 * l, T, n + 1, ww);
	const limb_t borrow = sub_n(R, R, P, 2 * l + n + 1);
	sub_1(R + 2 * l + n + 1, rn - (2 * l + n + 1), borrow);
	if ((R[rn - 1] >> 63) != 0u) {   // S too large: R += 3 S^2 - 3 S + 1, S -= 1, until R >= 0
		mul(SQ, Sh, h, sp, l, ww);   // S' Q
		do {
			for (std::size_t i = 0; i < rn; ++i) G[i] = 0;
			for (std::size_t i = 0; i < dn; ++i) G[2 * l + i] = D[i];
			add_1(G + l + n, rn - l - n, addmul_1(G + l, SQ, n, 6u));
			add_1(G + 2 * l, rn - 2 * l, addmul_1(G, Q2, 2 * l, 3u));
			sub_1(G + n, rn - n, submul_1(G, sp, n, 3u));
			add_1(G, rn, 1u);
			add_n(R, R, G, rn);
			// Q^2 -= 2 Q - 1, S' Q -= S', Q -= 1 (Q >= 1: S never falls below S' B^l)
			sub_1(Q2 + l, l, submul_1(Q2, sp, l, 2u));
			add_1(Q2, 2 * l, 1u);
			sub_1(SQ + h, n - h, sub_n(SQ, SQ, Sh, h));
			sub_1(sp, l, 1u);
		} while ((R[rn - 1] >> 63) != 0u);
	}
	for (std::size_t i = 0; i < 2 * n + 1; ++i) rp[i] = R[i];
}

}}}  // namespace sw::universal::limb_kernels
