#pragma once
// constants.hpp: definition of math constants in bfloat16 precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// best practice for C++11
// inject mathematical constants in our namespace

// a_b reads a over b, as in 1_pi being 1 over pi

// bfloat16 values
	// we can't do constexpr as it is using memcpy, which is not constexpr
inline const bfloat16 bf_pi_4     { 0.785398163f }; // pi/4
inline const bfloat16 bf_pi_3     { 1.047197551f }; // pi/3
inline const bfloat16 bf_pi_2     { 1.570796327f }; // pi/2
inline const bfloat16 bf_pi       { 3.141592653f }; // pi
inline const bfloat16 bf_3pi_4    { 2.356194490f }; // 3*pi/4
inline const bfloat16 bf_3pi_2    { 4.712388980f }; // 3*pi/2
inline const bfloat16 bf_2pi      { 6.283185307f }; // 2*pi

inline const bfloat16 bf_1_pi     { 0.318309886f }; // 1/pi
inline const bfloat16 bf_2_pi     { 0.636619772f }; // 2/pi
inline const bfloat16 bf_2_sqrtpi { 1.128379167f }; // 2/sqrt(pi)
inline const bfloat16 bf_sqrt2    { 1.414213562f }; // sqrt(2)
inline const bfloat16 bf_1_sqrt2  { 0.707106781f }; // 1/sqrt(2)

inline const bfloat16 bf_e        { 2.718281828f }; // e
inline const bfloat16 bf_log2e    { 1.442695041f }; // log2(e)
inline const bfloat16 bf_log10e   { 0.434294482f }; // log10(e)

inline const bfloat16 bf_ln2      { 0.693147181f }; // ln(2)
inline const bfloat16 bf_ln10     { 2.302585093f }; // ln(10)

}} // namespace sw::universal
