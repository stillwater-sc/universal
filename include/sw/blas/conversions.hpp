#pragma once
// conversions.hpp: aggregate conversions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <algorithm> // std::transform
#include <iterator>  // std::back_inserter
#include <vector>
#include <ranges>

namespace sw::blas {

    template<typename Narrow, typename Wide>
    std::vector<Narrow> narrow_cast(const std::vector<Wide>& src) {
        std::vector<Narrow> dst;
        dst.reserve(src.size());
        std::transform(src.begin(), src.end(), std::back_inserter(dst),
            [](const Wide& v) { return static_cast<Narrow>(v); });
        return dst;
    }

}  // namespace sw::blas

