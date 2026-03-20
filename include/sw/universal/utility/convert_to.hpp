#pragma once
// convert_to.hpp: more convenient conversion
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

/**
 * @brief Functional wrapper around the library's `convert(src, dst)` customization point.
 *
 * @tparam Target Destination type.
 * @tparam Source Source type.
 *
 * @details `convert_to` exists for code that wants the library's conversion semantics without spelling an
 * intermediate target variable. It still relies on an appropriate `convert(const Source&, Target&)`
 * overload being visible, so it follows the same extension rules as the rest of the Universal conversion layer.
 *
 * @note This is intentionally not the same as `static_cast<Target>(src)`: it routes through Universal's
 * explicit conversion bridge, which can preserve conversion behavior that a plain C++ cast does not express.
 */
template <typename Target, typename Source>
Target convert_to(const Source& src) {
    Target t;
    convert(src, t);
    return t;
}

}} // namespace sw::universal
