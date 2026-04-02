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
// Generator: callable with signature real g(real x) producing a bracketing sequence
// Refinement: callable with signature real f(real a, real b) producing a bisection point
// nbits: total number of bits (including sign)
// bt: block storage type
template<typename Generator, typename Refinement, unsigned nbits, typename bt = uint8_t>
class bisection;

template<typename Generator, typename Refinement, unsigned nbits, typename bt>
bisection<Generator, Refinement, nbits, bt>
abs(const bisection<Generator, Refinement, nbits, bt>&);

}} // namespace sw::universal
