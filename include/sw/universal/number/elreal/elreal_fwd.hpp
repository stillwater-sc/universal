#pragma once
// elreal_fwd.hpp : forward declarations of elreal (Exact Lazy Real, McCleeary 2019)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cstddef>

namespace sw { namespace universal {

// elreal is the Exact Lazy Real type: a non-templated class that represents
// a real value as a lazy stream of progressively refined double-precision
// components. See elreal_impl.hpp for the storage representation, approximation
// element, and refinement protocol.
class elreal;

// free-function helpers (full definitions land in Phase C / E)
elreal abs(const elreal&);
elreal fabs(const elreal&);

}} // namespace sw::universal
