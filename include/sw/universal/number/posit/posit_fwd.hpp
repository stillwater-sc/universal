#pragma once
// posit_fwd.hpp :  forward declarations of the posit environment
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>  // for unsigned
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>

namespace sw { namespace universal {

// posit types
template<unsigned nbits, unsigned es, typename bt> class posit;

// posit-specialized math functions
template<unsigned nbits, unsigned es, typename bt> posit<nbits, es, bt> abs(const posit<nbits, es, bt>&);
template<unsigned nbits, unsigned es, typename bt> posit<nbits, es, bt> sqrt(const posit<nbits, es, bt>&);

// parsing
template<unsigned nbits, unsigned es, typename bt>
bool parse(const std::string& number, posit<nbits, es, bt>& v);

// helpers
template<unsigned nbits, unsigned es, typename bt, unsigned fbits, BlockTripleOperator op> constexpr posit<nbits, es, bt>& convert(const blocktriple<fbits, op, bt>&, posit<nbits, es, bt>&);

template<unsigned nbits, typename bt> constexpr int decode_regime(const blockbinary<nbits, bt>&);

}} // namespace sw::universal
