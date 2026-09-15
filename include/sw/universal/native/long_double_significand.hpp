#pragma once
// long_double_significand.hpp: the significand of a long double, 64 bits at a time, exactly
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>    // std::frexp, std::ldexp, std::fabs, std::signbit
#include <cstdint>

namespace sw { namespace universal {

#if LONG_DOUBLE_SUPPORT
	// The significand of a finite, non-zero long double, whatever its format, read 64 bits at a
	// time: |v| = 0.w0 w1 w2 ... * 2^(scale() + 1), the leading bit of w0 set. Each step is exact:
	// the remainder is scaled by 2^64, its integer part taken, and subtracted. So the words run out
	// -- empty() -- once every bit has been read: after one word on x87, two on IEEE binary128,
	// and on IBM double-double as many as the gap between its two doubles needs (1 + 2^-1000 spans
	// 1001 bits). A type that rounds reads what it keeps and treats !empty() as sticky; an exact
	// type reads until empty().
	//
	// extractFields() hands out x87-shaped fields, a 64-bit significand, for the types that round
	// to 62 bits or fewer; this is for the ones that keep more (#1517).
	class long_double_significand {
	public:
		explicit long_double_significand(long double v) noexcept
			: _negative(std::signbit(v)), _exponent(0), _rest(0.0l) {
			_rest = std::frexp(std::fabs(v), &_exponent);  // [0.5, 1)
		}

		bool negative() const noexcept { return _negative; }
		int  scale() const noexcept { return _exponent - 1; }  // |v| = 1.f * 2^scale()
		bool empty() const noexcept { return _rest == 0.0l; }

		// the next 64 bits of the significand; 0 once empty()
		std::uint64_t next() noexcept {
			_rest                  = std::ldexp(_rest, 64);  // [0, 2^64): exact
			const std::uint64_t w  = static_cast<std::uint64_t>(_rest);
			_rest                 -= static_cast<long double>(w);  // the bits below: exact
			return w;
		}

	private:
		bool        _negative;
		int         _exponent;
		long double _rest;  // the bits not read yet, in [0, 1)
	};
#endif

}}  // namespace sw::universal
