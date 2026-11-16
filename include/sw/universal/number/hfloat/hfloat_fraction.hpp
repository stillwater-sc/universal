#pragma once
// hfloat_fraction.hpp: a fixed-width unsigned integer of 32-bit limbs, the working register of hfloat's arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// hfloat carried its fraction in a uint64_t, so hfp128's 112 fraction bits were cut to the low 64:
// compare, multiply, divide and isone() were wrong and 1.5 / 1.0 divided by zero (#2105). The
// fraction now lives in a wide<capacity>: every operation keeps all of its bits, and products and
// quotients are formed exactly before they are truncated.
#include <cstddef>
#include <cstdint>
#include <universal/internal/blocktype/limb_division.hpp>

namespace sw { namespace universal { namespace hfloat_detail {

template<unsigned capacity>
struct wide {
	static constexpr unsigned L = (capacity + 31u) / 32u;
	std::uint32_t w[L];

	constexpr wide() noexcept : w{} {}

	static constexpr wide from_u64(std::uint64_t v) noexcept {
		wide r;
		r.w[0] = static_cast<std::uint32_t>(v);
		if constexpr (L > 1) r.w[1] = static_cast<std::uint32_t>(v >> 32);
		return r;
	}
	// the same value in a register of another capacity; bits past the new capacity are dropped
	template<unsigned other>
	constexpr wide<other> resize() const noexcept {
		wide<other> r;
		for (unsigned i = 0; i < L && i < wide<other>::L; ++i) r.w[i] = w[i];
		r.truncate(other);
		return r;
	}

	constexpr void clear() noexcept { for (unsigned i = 0; i < L; ++i) w[i] = 0; }
	constexpr bool iszero() const noexcept {
		for (unsigned i = 0; i < L; ++i) if (w[i] != 0) return false;
		return true;
	}
	constexpr bool test(unsigned i) const noexcept {
		return i < 32u * L && ((w[i / 32u] >> (i % 32u)) & 1u) != 0;
	}
	constexpr void set(unsigned i) noexcept {
		if (i < 32u * L) w[i / 32u] |= (std::uint32_t(1) << (i % 32u));
	}
	// the position of the most significant 1 bit, -1 for zero
	constexpr int msb() const noexcept {
		for (unsigned i = L; i-- > 0;) {
			if (w[i] != 0) return static_cast<int>(i * 32u + 31u - limb::leading_zeros(w[i]));
		}
		return -1;
	}
	// keep the low n bits
	constexpr void truncate(unsigned n) noexcept {
		for (unsigned i = 0; i < L; ++i) {
			const unsigned lo = i * 32u;
			if (lo >= n) w[i] = 0;
			else if (n - lo < 32u) w[i] &= (std::uint32_t(1) << (n - lo)) - 1u;
		}
	}
	// shift right by n bits; returns true when a 1 bit was shifted out
	constexpr bool shr(unsigned n) noexcept {
		if (n == 0) return false;
		if (n >= 32u * L) {
			const bool sticky = !iszero();
			clear();
			return sticky;
		}
		const unsigned q = n / 32u;
		const unsigned s = n % 32u;
		bool sticky = false;
		for (unsigned i = 0; i < q; ++i) sticky = sticky || (w[i] != 0);
		if (s != 0) sticky = sticky || ((w[q] & ((std::uint32_t(1) << s) - 1u)) != 0);
		for (unsigned i = 0; i < L; ++i) {
			const std::uint32_t lo = (i + q < L) ? w[i + q] : 0u;
			const std::uint32_t hi = (i + q + 1u < L) ? w[i + q + 1u] : 0u;
			w[i] = (s == 0) ? lo : ((lo >> s) | (hi << (32u - s)));
		}
		return sticky;
	}
	// shift left by n bits; bits past the capacity are dropped
	constexpr void shl(unsigned n) noexcept {
		if (n == 0) return;
		if (n >= 32u * L) { clear(); return; }
		const unsigned q = n / 32u;
		const unsigned s = n % 32u;
		for (unsigned i = L; i-- > 0;) {
			const std::uint32_t hi = (i >= q) ? w[i - q] : 0u;
			const std::uint32_t lo = (i >= q + 1u) ? w[i - q - 1u] : 0u;
			w[i] = (s == 0) ? hi : ((hi << s) | (lo >> (32u - s)));
		}
		truncate(capacity);
	}
	constexpr void add(const wide& b) noexcept {
		std::uint64_t c = 0;
		for (unsigned i = 0; i < L; ++i) {
			const std::uint64_t t = std::uint64_t(w[i]) + b.w[i] + c;
			w[i] = static_cast<std::uint32_t>(t);
			c = t >> 32;
		}
		truncate(capacity);
	}
	// *this -= b, for *this >= b
	constexpr void sub(const wide& b) noexcept {
		std::uint64_t borrow = 0;
		for (unsigned i = 0; i < L; ++i) {
			const std::uint64_t t = std::uint64_t(w[i]) - b.w[i] - borrow;
			w[i] = static_cast<std::uint32_t>(t);
			borrow = (t >> 32) != 0 ? 1u : 0u;
		}
	}
	constexpr void decrement() noexcept {   // *this -= 1, for *this >= 1
		for (unsigned i = 0; i < L; ++i) {
			if (w[i]-- != 0) break;
		}
	}
	constexpr int compare(const wide& b) const noexcept {
		for (unsigned i = L; i-- > 0;) {
			if (w[i] != b.w[i]) return (w[i] < b.w[i]) ? -1 : 1;
		}
		return 0;
	}
	constexpr std::uint64_t low64() const noexcept {
		std::uint64_t v = w[0];
		if constexpr (L > 1) v |= std::uint64_t(w[1]) << 32;
		return v;
	}
	constexpr unsigned significant_limbs() const noexcept {
		for (unsigned i = L; i-- > 0;) if (w[i] != 0) return i + 1u;
		return 0;
	}
};

// the exact product
template<unsigned A, unsigned B>
constexpr wide<A + B> multiply(const wide<A>& a, const wide<B>& b) noexcept {
	wide<A + B> p;
	for (unsigned i = 0; i < wide<A>::L; ++i) {
		std::uint64_t carry = 0;
		for (unsigned j = 0; j < wide<B>::L; ++j) {
			if (i + j >= wide<A + B>::L) break;
			const std::uint64_t t = std::uint64_t(a.w[i]) * b.w[j] + p.w[i + j] + carry;
			p.w[i + j] = static_cast<std::uint32_t>(t);
			carry = t >> 32;
		}
		if (i + wide<B>::L < wide<A + B>::L) p.w[i + wide<B>::L] = static_cast<std::uint32_t>(carry);
	}
	return p;
}

// floor(u / v), for v != 0
template<unsigned N, unsigned D>
constexpr wide<N> divide(const wide<N>& u, const wide<D>& v) noexcept {
	wide<N> q;
	const unsigned m = u.significant_limbs();
	const unsigned n = v.significant_limbs();
	if (m == 0 || n == 0 || m < n) return q;
	std::uint32_t un[wide<N>::L + 1u]{};
	std::uint32_t vn[wide<D>::L]{};
	limb::divide(u.w, m, v.w, n, q.w, un, vn);
	return q;
}

}}}  // namespace sw::universal::hfloat_detail
