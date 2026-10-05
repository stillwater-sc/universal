#pragma once
// attributes.hpp: free functions reporting properties of a bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
constexpr bool sign(const bposit<nbits, rs, es, bt>& v) noexcept { return v.sign(); }

// binary scale r 2^es + e; 0 for zero and NaR
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
constexpr int scale(const bposit<nbits, rs, es, bt>& v) noexcept { return v.scale(); }

// fraction bits this encoding carries: fbitsmin at the capped regime, fbits at 1.0
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
constexpr unsigned fraction_bits(const bposit<nbits, rs, es, bt>& v) noexcept {
	return (v.iszero() || v.isnar()) ? 0u : v.decode_fields().nf;
}

}} // namespace sw::universal
