#pragma once
// manipulators.hpp: type identifier and stream output for elreal.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <typeinfo>

#include <universal/number/elreal/elreal_fwd.hpp>

namespace sw { namespace universal {

// type_tag: a human-readable identifier for the elreal type (host + default depth).
template <typename FpType>
inline std::string type_tag(const elreal<FpType>& = {}) {
    std::string host = "double";
    if (sizeof(FpType) == sizeof(float))  host = "float";
    return std::string("elreal<") + host + ">";
}

// stream output: print the value at its current precision as a double approximation.
// (A full high-precision decimal printer is a later manipulators item.)
template <typename FpType>
inline std::ostream& operator<<(std::ostream& ostr, const elreal<FpType>& v) {
    return ostr << static_cast<double>(v);
}

}} // namespace sw::universal
