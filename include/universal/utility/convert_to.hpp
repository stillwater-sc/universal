#pragma once
// convert_to.hpp: more convenient conversion
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

/** Convert from \p Source to \p Target in a functional style using \ref convert.
 * Example: 'convert_to<double>(p)' where 'p' is a posit.
 * The benefit is that no variable has to be created for converting into it. **/
template <typename Target, typename Source>
Target convert_to(const Source& src) {
    Target t;
    convert(src, t);
    return t;
}

}} // namespace sw::universal
