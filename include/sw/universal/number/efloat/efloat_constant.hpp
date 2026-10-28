#pragma once
// efloat_constant.hpp: an efloat value frozen into fixed storage, for constexpr variables of any
// precision (#1819, phase 4)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source
// license.
//
// An efloat keeps its significand inside the value up to EFLOAT_INLINE_LIMBS 64-bit limbs (512 bits
// by default) and on the heap past them (#1820). A constant evaluation may allocate, but a constexpr
// variable cannot keep what it allocated, so an efloat past the inline limbs cannot be one:
//
//     constexpr efloat<4>  p = efloat_pi<4>();     // 128 bits, inline: a constexpr efloat
//     constexpr efloat<32> q = efloat_pi<32>();    // 1024 bits: an error, the limbs are on the heap
//
// efloat_constant<nlimbs> holds the value of an efloat<nlimbs> -- its state, sign, exponent,
// precision and limbs -- in an array sized for nlimbs * 32 bits, so it is a constexpr variable at
// every precision, and turns back into the same efloat, at run time or in a constant expression:
//
//     constexpr efloat_constant<32> pi = efloat_pi<32>();
//     static_assert(pi > 3 && pi < efloat<32>(22) / 7);
//     efloat<32> x = pi;                           // bit for bit efloat_pi<32>()
//
// It compares with itself, with efloat<nlimbs> and with native arithmetic types, and converts to
// float and double; arithmetic goes through the efloat it converts to. A value whose significand is
// longer than nlimbs * 32 bits -- one computed at a precision set past the type's -- is rounded to
// nlimbs * 32 bits, in the current rounding mode (to nearest in a constant evaluation), and keeps
// that precision.
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <universal/number/efloat/efloat_impl.hpp>

namespace sw { namespace universal {

template<unsigned nlimbs>
class efloat_constant {
public:
	using value_type = efloat<nlimbs>;
	using limb_t     = typename value_type::limb_t;
	// 64-bit limbs: as many as nlimbs * 32 bits take
	static constexpr std::size_t capacity = (nlimbs + 1u) / 2u;

	constexpr efloat_constant() noexcept : efloat_constant(value_type{}) {}

	constexpr efloat_constant(const value_type& v) noexcept
		: _limb{}, _count{ 0 }, _exponent{ 0 }, _precision_limbs{ 0 }, _state{ FloatingPointState::Zero }, _sign{ false } {
		if (v.limb_count() > capacity) {
			value_type rounded(v);
			rounded.set_precision(nlimbs * 32u);
			freeze(rounded);
		}
		else {
			freeze(v);
		}
	}

	// the efloat this holds, bit for bit
	constexpr value_type value() const {
		value_type v;
		v._state           = _state;
		v._sign            = _sign;
		v._exponent        = _exponent;
		v._precision_limbs = _precision_limbs;
		v._limb.assign(_limb.data(), _count);
		return v;
	}
	constexpr operator value_type() const { return value(); }
	explicit constexpr operator double() const { return double(value()); }
	explicit constexpr operator float()  const { return float(value()); }

	// the selectors of the efloat
	constexpr bool          iszero()        const noexcept { return _state == FloatingPointState::Zero; }
	constexpr bool          isinf()         const noexcept { return _state == FloatingPointState::Infinite; }
	constexpr bool          isnan()         const noexcept { return _state == FloatingPointState::QuietNaN || _state == FloatingPointState::SignalingNaN; }
	constexpr int           sign()          const noexcept { return _sign ? -1 : 1; }
	constexpr std::int64_t  scale()         const noexcept { return _exponent; }
	constexpr unsigned      get_precision() const noexcept { return value().get_precision(); }
	constexpr std::size_t   limb_count()    const noexcept { return _count; }
	constexpr const limb_t* limb_data()     const noexcept { return _limb.data(); }

	// comparisons: with itself, with efloat<nlimbs>, and with native arithmetic types, each by the
	// efloat comparison (a native operand converts to an efloat exactly first)
#define EFLOAT_CONSTANT_COMPARISON(op)                                                                    \
	friend constexpr bool operator op(const efloat_constant& a, const efloat_constant& b) { return a.value() op b.value(); } \
	friend constexpr bool operator op(const efloat_constant& a, const value_type& b)      { return a.value() op b; }         \
	friend constexpr bool operator op(const value_type& a, const efloat_constant& b)      { return a op b.value(); }         \
	template<typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>                    \
	friend constexpr bool operator op(const efloat_constant& a, Native b) { return a.value() op value_type(b); }              \
	template<typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>                    \
	friend constexpr bool operator op(Native a, const efloat_constant& b) { return value_type(a) op b.value(); }
	EFLOAT_CONSTANT_COMPARISON(==)
	EFLOAT_CONSTANT_COMPARISON(!=)
	EFLOAT_CONSTANT_COMPARISON(<)
	EFLOAT_CONSTANT_COMPARISON(>)
	EFLOAT_CONSTANT_COMPARISON(<=)
	EFLOAT_CONSTANT_COMPARISON(>=)
#undef EFLOAT_CONSTANT_COMPARISON

private:
	std::array<limb_t, capacity> _limb;      // the significand's limbs, least significant first; the rest 0
	std::size_t                  _count;     // how many of them the value holds
	std::int64_t                 _exponent;
	unsigned                     _precision_limbs;
	FloatingPointState           _state;
	bool                         _sign;

	constexpr void freeze(const value_type& v) noexcept {
		_count           = v._limb.size();
		_exponent        = v._exponent;
		_precision_limbs = v._precision_limbs;
		_state           = v._state;
		_sign            = v._sign;
		for (std::size_t i = 0; i < _count; ++i) _limb[i] = v._limb[i];
	}
};

}}  // namespace sw::universal
