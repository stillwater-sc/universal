#pragma once
// quire.hpp: umbrella header for the generalized quire (super-accumulator)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The generalized quire is parameterized on the number type it accumulates for,
// using quire_traits<NumberType> to determine the accumulator size.
//
// This implementation uses blockbinary (limb-based, uint32_t/uint64_t) for fast
// carry propagation, unlike the legacy posit quire which uses bitblock (std::bitset).
//
// Usage:
//   #include <universal/number/cfloat/cfloat.hpp>
//   #include <universal/number/quire/quire.hpp>
//
//   using Scalar = cfloat<32, 8, uint32_t, true, false, false>;
//   sw::universal::quire<Scalar> q;
//   q += blocktriple_product;   // accumulate an unrounded product
//   Scalar result = q.convert_to<Scalar>();
//
// Relates to #345, #546

#include <universal/number/quire/exceptions.hpp>
#include <universal/traits/quire_traits.hpp>
#include <universal/utility/boolean_logic_operators.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>

#include <universal/number/quire/quire_impl.hpp>
