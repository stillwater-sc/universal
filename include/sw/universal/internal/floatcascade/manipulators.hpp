#pragma once
// manipulators.hpp: string builders for floatcascade -- the text half
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the floatcascade headers (#1334), matching blockbinary's and blockdigit's
// shape. to_tuple() and to_scientific() build a std::string through a std::stringstream,
// so they need <sstream> but not <iostream>; the stream insertion operator is in
// iostream.hpp.
#include <string>
#include <sstream>
#include <ios>          // std::scientific / std::showpos / std::uppercase
#include <cstddef>      // size_t
#include <cmath>        // std::abs on the exponent
#include <universal/internal/floatcascade/floatcascade.hpp>

namespace sw { namespace universal {


template<size_t N>
std::string to_tuple(const floatcascade<N>& fc) {
    std::stringstream ss;
	ss << std::scientific;
	//ss.setprecision(17); // max precision of double
    ss << "{ ";
    for (size_t i = 0; i < N; ++i) {
        if (i > 0) ss << ", ";
        ss << fc.e[i];
    }
    ss << '}';
    return ss.str();
}

template<size_t N>
std::string to_scientific(const floatcascade<N>& fc,
    int precision = N * 17,
    bool showpos = false,
    bool uppercase = false,
    bool trailing_zeros = true) {
    // precondition: fc is a normalized floatcascade

    // Handle special cases early
    if (fc.isnan(NAN_TYPE_QUIET)) return std::string("qNaN");
    if (fc.isnan(NAN_TYPE_SIGNALLING)) return std::string("sNaN");
    if (fc.isinf(INF_TYPE_POSITIVE)) return std::string("Inf");
    if (fc.isinf(INF_TYPE_NEGATIVE)) return std::string("-Inf");
    if (fc.iszero()) return std::string(showpos ? "+0.0e+0" : "0.0e+0");

    // Step 1: Estimate exponent from the most significant non-zero component
    double hi = fc[0];
    int exp10 = static_cast<int>(std::floor(std::log10(std::abs(hi))));
    if (!std::isfinite(exp10)) exp10 = 0; // handle log10(0) = -inf case
    double scale = std::pow(10.0, -exp10);

	// Step 2: Scale all components so we are in the range [0.0, 10.0)
    double scaled[N];
    for (size_t i = 0; i < N; ++i) {
        scaled[i] = fc[i] * scale;
    }
    double acc = 0.0;
    for (size_t i = 0; (i < N) && (i < 3); ++i) acc += scaled[i];
	bool negative = std::signbit(acc);
	acc = std::abs(acc);

    // Step 3: Generate digits iteratively
    std::string digits;
	digits.reserve(static_cast<size_t>(precision) + 2); // leading digit + precision digits

    for (int i = 0; i <= precision; ++i) {
        int digit = static_cast<int>(acc);
		if (digit > 9) digit = 9; // defensive clamp
        digits.push_back(static_cast<char>('0' + digit));
        acc = (acc - digit) * 10.0;
    }

	// Step 4: Round last digit and propagate carry (with exponent adjustment)
    if (acc >= 5.0) {
		int i = precision;
        for (; i >= 0 && digits[i] == '9'; --i) {
			digits[i] = '0';
        }
        if (i >= 0) {
            digits[i] += 1; // increment this digit
        }
        else {
			// overflowed the leading digit: 9.99... 9 -> 10.0...0
            digits.insert(digits.begin(), '1');
            exp10 += 1;
		}
    }

    // Step 5: Format string
    std::string result;
    if (negative) result += '-';
    else if (showpos) result += '+';

    result += digits[0]; // leading digit
    result += '.';
    if (precision > 0) {
        result.append(digits.begin() + 1, digits.begin() + 1 + precision);
        if (trailing_zeros) {
            // digits already has length precision + 1
        }
        else {
			// trim trailing zeros after decimal point
            while (!result.empty() && result.back() == '0') {
                result.pop_back();
			}
            if (!result.empty() && result.back() == '.') {
                result.pop_back(); // remove decimal point if no digits follow
			}
        }
    }
    else {
		result += '0'; // no precision requested, just add a zero
    }

    result += uppercase ? "E" : "e";
    result += (exp10 >= 0 ? "+" : "-");
    result += std::to_string(std::abs(exp10));

    return result;
}

}} // namespace sw::universal
