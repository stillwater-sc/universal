#pragma once
// math_next.hpp: nextafter/nexttoward functions for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/posit/atomic_fused_operators.hpp>   // posit_fused: the exact comparison in nexttoward

namespace sw { namespace universal {

/*
Parameters
	x  Base value.
	t  Value toward which the return value is approximated.
If both parameters compare equal, the function returns t.

Return Value
	The next representable value after x in the direction of t.

	If x is the largest finite value representable in the type,
	and the result is infinite or not representable, an overflow range error occurs.

	If an overflow range error occurs:
	- And math_errhandling has MATH_ERRNO set: the global variable errno is set to ERANGE.
	- And math_errhandling has MATH_ERREXCEPT set: FE_OVERFLOW is raised.
	*/
// NaR propagates (#2071, user decision): nextafter(x, NaR) and nextafter(NaR, t) are NaR. They
// stepped up or down depending on x. The step is on the encoding: the posits are ordered as their
// encodings are, and x != t keeps the step away from NaR (x < t is never maxpos, x > t never maxneg).
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> nextafter(posit<nbits,es,bt> x, posit<nbits, es, bt> target) {
	if (x.isnar() || target.isnar()) { x.setnar(); return x; }
	if (x == target) return target;
	if (x < target) ++x;
	else --x;
	return x;
}

// nexttoward compares x with a posit<256,5> target exactly, whatever x's configuration: it converted
// x to posit<256,5>, which rounds a wider x and turned the direction around for x next to target
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> nexttoward(posit<nbits,es,bt> x, posit<256, 5> target) {
	if (x.isnar() || target.isnar()) { x.setnar(); return x; }
	using namespace posit_fused;
	constexpr std::size_t Nx = capacity<nbits, es, bt>();
	constexpr std::size_t Nt = capacity<256, 5, std::uint8_t>();
	constexpr std::size_t N = (Nx > Nt) ? Nx : Nt;
	constexpr std::int64_t F = (posit<nbits, es, bt>::fbits > posit<256, 5>::fbits) ? posit<nbits, es, bt>::fbits : posit<256, 5>::fbits;
	term<N> t = from_posit<N>(target);
	t.negative = !t.negative;
	const term<N> diff = add(from_posit<N>(x), t, F, 0);   // its sign is exact: the collapse never flips it
	if (diff.zero) return x;
	if (diff.negative) ++x;   // x < target
	else --x;
	return x;
}

}} // namespace sw::universal
