#pragma once
// core.hpp: the edecimal arithmetic core -- no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the edecimal headers (#1334, Phase 2).
//
//     #include <universal/number/edecimal/edecimal.hpp>   // everything, as before
//     #include <universal/number/edecimal/core.hpp>       // arithmetic only
//
// WHAT "core" EXCLUDES, precisely: the stream family -- <iostream>, <sstream>,
// <iomanip>, <ostream>, <istream>. Measured: this header pulls zero of the five.
//
// It does NOT exclude <string>. operator=(const std::string&) and parse() are part of
// the type's arithmetic surface, exactly as integer's string constructor is, and
// <string> is inside the epic's derived line budget. <cstdio> backs fprintf(stderr,...),
// the Phase 0 idiom adopted specifically so diagnostics do not require <iostream>.
// to_binary()/to_string() are in manipulators.hpp and the stream operators in
// iostream.hpp.
//
// edecimal derives from std::vector<uint8_t>, so <vector> is structural here, not
// incidental.
//
// NOTE: native/ieee754_core.hpp is used rather than native/ieee754.hpp -- convert_ieee754()
// needs extractFields() and ieee754_parameter<>, which are the bit-manipulation half; the
// text half of that header is not needed by the core (#1334).
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <cstdint>
#include <cassert>
#include <vector>
#include <limits>
#include <algorithm>
#include <type_traits>

#include <universal/native/ieee754_core.hpp>
#include <universal/string/strmanip.hpp>          // trim(), used by parse()
#include <universal/utility/string_parse.hpp>     // scan_decimal_float(), used by parse()

#include <universal/number/edecimal/exceptions.hpp>
#include <universal/number/edecimal/edecimal_fwd.hpp>
#include <universal/number/edecimal/edecimal_impl.hpp>
#include <universal/number/edecimal/numeric_limits.hpp>
