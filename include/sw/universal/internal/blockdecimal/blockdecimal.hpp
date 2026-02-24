#pragma once
// blockdecimal.hpp: redirect to blockdigit with radix=10
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/internal/blockdigit/blockdigit.hpp>

namespace sw { namespace universal {

template<unsigned ndigits>
using blockdecimal = blockdigit<ndigits, 10>;

}} // namespace sw::universal
