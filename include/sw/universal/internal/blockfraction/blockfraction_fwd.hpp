#pragma once
// blockfraction_fwd.hpp: forward definitions for blockfraction
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// forward references
template<unsigned nbits, typename bt> class blockfraction;
template<unsigned nbits, typename bt> constexpr blockfraction<nbits, bt> twosComplementFree(const blockfraction<nbits, bt>&) noexcept;
template<unsigned nbits, typename bt> struct bsquorem;
template<unsigned nbits, typename bt> bsquorem<nbits, bt> longdivision(const blockfraction<nbits, bt>&, const blockfraction<nbits, bt>&);

}} // namespace sw::universal
