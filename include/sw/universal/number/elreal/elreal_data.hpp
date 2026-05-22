#pragma once
// elreal_data.hpp: tagged-union generator (lazy_generator variant)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase K.2 of follow-up epic #903 (#905).
//
// Background
// ==========
// Phase I (#902) identified the per-operator `std::function` heap
// allocation as a major cost source for elreal arithmetic. Each binary
// operator captures two elreals by value (~216 bytes of capture), which
// blows past libstdc++'s 16-byte std::function SBO and forces a heap
// allocation per call.
//
// This phase replaces `std::function<double(std::size_t)>` with a small
// `std::variant` of known generator shapes. Operand captures are by
// `std::shared_ptr<const elreal>` (16 bytes per operand) rather than by
// value (~104 bytes per operand), so each generator's capture fits
// comfortably inline.
//
// Cost shape per binary op
// ------------------------
//   Today: 1 std::function alloc (~216 bytes capture, memcpy 216 bytes)
//   After: 2 std::make_shared<const elreal> allocs (~120 bytes each,
//          memcpy 104 bytes each) + variant emplace (no alloc)
//
// The new approach trades 1 big alloc for 2 small allocs. On modern
// allocators with small-object pools this is typically a net win. The
// throughput numbers in the Phase K.2 PR document the actual measured
// effect under gcc 13 and clang 18.
//
// Canonical generator shapes (cover all ~25 elreal operators / functions)
// ----------------------------------------------------------------------
//   gen_unary_linear        depth-1 = coeff * a.at(1)
//                           (exp, exp2, expm1, log family, sinh/cosh/tanh,
//                            asinh/acosh/atanh, asin/acos/atan, sin/cos/tan)
//
//   gen_binary_linear       depth-1 = c0 + ca * a.at(1) + cb * b.at(1)
//                           (+, -, *, pow, atan2)
//
//   gen_sqrt                depth-1 = sqrt-specific EFT residual
//
//   gen_unary_neg           depth-k = -wrapped.at(k)  (trampoline)
//
//   gen_rational_residual   depth-1 = exact p - q * c0 residual via EFTs
//                           (elreal(long long p, long long q) constructor)
//
//   std::monostate          no generator (depth-0-only result)

#include <cmath>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <variant>

#include <universal/numerics/error_free_ops.hpp>

namespace sw { namespace universal {

class elreal;  // forward decl -- full definition in elreal_impl.hpp

// Handle to an operand. Reference-counted; copying is cheap (one atomic
// increment). The const-ness reflects the contract that a generator
// only READS from its operands via at(k). Mutation through the
// `mutable` interior of elreal is permitted by the elreal class's
// lazy-materialization design, but the generator doesn't itself
// initiate any mutation beyond demand-driven materialization.
using elreal_handle = std::shared_ptr<const elreal>;

// ---------- generator variant alternatives -----------------------------------

struct gen_unary_linear {
	elreal_handle a;
	double coeff;
};

struct gen_binary_linear {
	elreal_handle a;
	elreal_handle b;
	double c0;
	double ca;
	double cb;
};

struct gen_sqrt {
	elreal_handle a;
	double c0;
	std::size_t lead_idx;
};

struct gen_unary_neg {
	elreal_handle wrapped;
};

struct gen_rational_residual {
	double p;
	double q;
	double c0;
};

using lazy_generator = std::variant<
	std::monostate,
	gen_unary_linear,
	gen_binary_linear,
	gen_sqrt,
	gen_unary_neg,
	gen_rational_residual
>;

// Forward declaration; the body is in elreal_impl.hpp where elreal is
// fully defined (since the variant alternatives invoke elreal::at()).
double evaluate_generator(const lazy_generator& g, std::size_t k);

}}  // namespace sw::universal
