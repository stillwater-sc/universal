#pragma once
// efloat_fwd.hpp: forward definitions of the adaptive precision efloat type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <universal/utility/decimal_to_binary.hpp>   // default_big_bits (parse BigBits default)

namespace sw { namespace universal {

// forward references
template<unsigned nlimbs> class efloat;

// parse a decimal/scientific literal into an efloat, correctly rounded to the value's
// precision whatever the literal's length (#1713). BigBits is the narrowest result width
// tried first; it does not limit the precision (see the definition).
template<unsigned BigBits = sw::universal::decimal_to_binary::default_big_bits, unsigned nlimbs>
bool parse(const std::string& number, efloat<nlimbs>& v);

}}  // namespace sw::universal
