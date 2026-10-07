#pragma once
// attributes.hpp: free functions reporting properties of a poxel
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

template<unsigned nbits, unsigned es, typename bt>
constexpr bool ubit(const poxel<nbits, es, bt>& v) noexcept { return v.ubit(); }

template<unsigned nbits, unsigned es, typename bt>
constexpr bool isexact(const poxel<nbits, es, bt>& v) noexcept { return v.isexact(); }

}} // namespace sw::universal
