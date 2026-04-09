#pragma once
// bisection_fwd.hpp: forward declarations for the bisection number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <cstddef>
#include <cstdint>

namespace sw { namespace universal {

// Forward declaration of the bisection number type.
//
// Generator  : callable with signature R g(R x)     producing a bracketing sequence
// Refinement : callable with signature R f(R a, R b) producing a bisection point
// nbits      : total number of bits (including sign)
// bt         : block storage type
// AuxReal    : auxiliary real type used for the interval bisection arithmetic
//              (default: double; use dd or qd for higher encoding accuracy
//               at wider nbits -- see Lindstrom CoNGA'19 Section 4)
template<typename Generator, typename Refinement,
         unsigned nbits, typename bt = uint8_t,
         typename AuxReal = double>
class bisection;

template<typename Generator, typename Refinement,
         unsigned nbits, typename bt, typename AuxReal>
bisection<Generator, Refinement, nbits, bt, AuxReal>
abs(const bisection<Generator, Refinement, nbits, bt, AuxReal>&);

}} // namespace sw::universal
