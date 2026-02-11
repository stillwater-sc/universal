#pragma once
// interval_fwd.hpp: forward declarations for the parameterized interval number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// forward declaration of the interval class template
template<typename Scalar> class interval;

// forward declaration of free functions
template<typename Scalar> interval<Scalar> abs(const interval<Scalar>&);
template<typename Scalar> interval<Scalar> sqrt(const interval<Scalar>&);

}} // namespace sw::universal
