#pragma once
// icf_array_bounds.hpp: narrow suppression of a GCC -Warray-bounds false positive
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Every Universal storage class indexes its limb array from a bit index the same way:
//
//     static constexpr unsigned nrBlocks = 1 + ((nbits - 1) / bitsInBlock);
//     bt _block[nrBlocks];
//
//     constexpr bool at(unsigned bitIndex) const noexcept {
//         if (bitIndex >= nbits) return false;
//         bt word = _block[bitIndex / bitsInBlock];
//         ...
//
// The guard makes the access in bounds for EVERY instantiation: bitIndex < nbits
// implies bitIndex / bitsInBlock <= (nbits - 1) / bitsInBlock == nrBlocks - 1.
//
// GCC nonetheless reports -Warray-bounds on that access at -O2 and above, and it is a
// false positive produced by identical-code-folding (-fipa-icf, on by default at -O2).
// Once a caller's loop bounds prove the guard redundant, at<N> and at<M> have
// byte-identical bodies, so GCC keeps one and aliases the rest -- then charges the
// surviving body's subscripts against ONE instantiation's _block extent. The subscripts
// it names are exactly the largest VALID index of the instantiation the code really came
// from. Two observed examples:
//
//   blocksignificand<29,uint8_t>::_block is uint8_t[4]; GCC reported subscripts 4, 6 and
//   10 against it -- the top blocks of nbits = 33, 56 and 87 (nrBlocks 5, 7 and 11), the
//   ADD/MUL/DIV significands of blocktriple<27>, all folded onto at<29>.
//
//   cfloat<32,8,uint8_t>::_block is uint8_t[4]; GCC reported subscript 4 against it --
//   the fifth block of cfloat<48,8,uint8_t>, which has 6.
//
// Confirmed diagnosis: -fno-ipa-icf removes every one of them.
//
// Suppress narrowly at the access, rather than building with -fno-ipa-icf: that is a
// codegen change made to silence a diagnostic, and being a header-only library it would
// not reach consumers of these headers anyway. Keep the scope to the single indexing
// statement so a genuine out-of-range access anywhere else still gets diagnosed.
//
// clang and MSVC do not warn here.

#if defined(__GNUC__) && !defined(__clang__)
#define UNIVERSAL_ICF_ARRAY_BOUNDS_PUSH \
	_Pragma("GCC diagnostic push")      \
	_Pragma("GCC diagnostic ignored \"-Warray-bounds\"")
#define UNIVERSAL_ICF_ARRAY_BOUNDS_POP \
	_Pragma("GCC diagnostic pop")
#else
#define UNIVERSAL_ICF_ARRAY_BOUNDS_PUSH
#define UNIVERSAL_ICF_ARRAY_BOUNDS_POP
#endif
