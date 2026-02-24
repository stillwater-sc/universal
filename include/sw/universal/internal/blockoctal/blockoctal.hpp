#pragma once
// blockoctal.hpp: redirect to blockdigit with radix=8
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/internal/blockdigit/blockdigit.hpp>

namespace sw { namespace universal {

template<unsigned ndigits>
using blockoctal = blockdigit<ndigits, 8>;

}} // namespace sw::universal
