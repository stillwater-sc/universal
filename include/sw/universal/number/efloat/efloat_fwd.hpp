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

// parse a decimal/scientific literal into an efloat. BigBits is the d2b working
// budget: the default handles typical literals; pass a larger value (e.g.
// parse<16384>) to represent very high precision -- the safe target precision
// scales with BigBits (see the definition and issue #1141).
template<unsigned BigBits = sw::universal::decimal_to_binary::default_big_bits, unsigned nlimbs>
bool parse(const std::string& number, efloat<nlimbs>& v);

}}  // namespace sw::universal
