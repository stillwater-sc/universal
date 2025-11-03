// manipulators.hpp: definitions of helper functions for double-double (dd) type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <iomanip>
#include <universal/number/dd/dd_fwd.hpp>
#include <universal/native/manipulators.hpp>
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// Generate a type tag for a doubledouble
	template<typename DoubleDoubleType,
		std::enable_if_t< is_dd<DoubleDoubleType>, bool> = true >
	std::string type_tag(DoubleDoubleType = {}) {
		return std::string("double-double");
	}

	inline std::string to_pair(const dd& v, int precision = 17) {
	    std::stringstream s;
	    // 53 bits = 16 decimal digits, 17 to include last, 15 typical valid digits
	    s << std::setprecision(precision) << "( " << v.high() << ", " << v.low() << ')';
	    return s.str();
    }

    inline std::string to_triple(const dd& v, int precision = 17) {
	    std::stringstream s;
	    bool              isneg = v.isneg();
	    int               scale = v.scale();
	    int               exponent;
	    dd                fraction = frexp(v, &exponent);
	    s << '(' << (isneg ? '1' : '0') << ", " << scale << ", " << std::setprecision(precision) << fraction << ')';
	    return s.str();
    }

    inline std::string to_binary(const dd& number, bool nibbleMarker = false) {
	    std::stringstream s;

	    double_decoder decoder;
	    decoder.d        = number.high();
	    int highExponent = static_cast<int>(decoder.parts.exponent) - ieee754_parameter<double>::bias;

	    s << "0b";
	    // print sign bit
	    s << (decoder.parts.sign ? '1' : '0') << '.';

	    // print exponent bits
	    {
		    uint64_t mask = 0x400ull;
		    for (int bit = 10; bit >= 0; --bit) {
			    s << ((decoder.parts.exponent & mask) ? '1' : '0');
			    if (nibbleMarker && bit != 0 && (bit % 4) == 0)
				    s << '\'';
			    mask >>= 1;
		    }
	    }

	    s << '.';

	    // print hi fraction bits
	    uint64_t mask = (1ull << 51);
	    for (int bit = 51; bit >= 0; --bit) {
		    s << ((decoder.parts.fraction & mask) ? '1' : '0');
		    if (nibbleMarker && bit != 0 && ((bit + 1) % 4) == 0)
			    s << '\'';
		    mask >>= 1;
	    }

	    // print lo fraction bits
	    decoder.d = number.low();
	    if (decoder.d == 0.0) {  // special case that has unaligned scales between lo and hi
		    s << '|';            // visual delineation between the two limbs
		    for (int ddbit = 52; ddbit >= 0; --ddbit) {
			    s << '0';
			    if (nibbleMarker && ddbit != 0 && (ddbit % 4) == 0)
				    s << '\'';
		    }
	    } else {
		    //         high limb                             low limb
		    //  52  51 .....               3210    52 51         ......      3210
		    //   h.  ffff ffff ...... ffff ffff     h. ffff ffff ...... ffff ffff
		    // 105 104                        53   52 51         ......      3210    dd_bit
		    //                                      | <--- exponent is exp(hi) - 53
		    //   h.  ffff ffff ...... ffff ffff     0. 0000 000h. ffff ffff ...... ffff ffff
		    //                                                 | <----- exponent would be exp(hi) - 61
		    //   h.  ffff ffff ...... ffff ffff     0. 0000 0000 ...... 000h. ffff ffff ...... ffff ffff
		    //                                                             | <----- exponent would be exp(hi) - 102
		    //   h.  ffff ffff ...... ffff ffff     0. 0000 0000 ...... 0000 000h. ffff ffff ...... ffff ffff
		    //                                                                  | <----- exponent would be exp(hi) - 106
		    // the low segment is always in normal form
		    int lowExponent = static_cast<int>(decoder.parts.exponent) - ieee754_parameter<double>::bias;

		    assert(highExponent >= lowExponent + 53 && "exponent of lower limb is not-aligned");

		    // enumerate in the bit offset space of the double-double
		    // that means, the first bit of the second limb is bit (105 - 53) == 52 and it cycles down to 0
		    // representing 2^-53 through 2^-106 relative to the MSB of the high limb
		    int offset = highExponent - 53 - lowExponent;
		    mask       = (1ull << 51);
		    s << '|';  // visual delineation between the two limbs
		    for (int ddbit = 52; ddbit >= 0; --ddbit) {
			    if (offset == 0) {
				    s << (decoder.d == 0.0 ? '0' : '1');  // show hidden bit when not-zero
			    } else if (offset > 0) {
				    // we have to introduce a leading zero as the hidden bit is positioned at a lower ddbit offset
				    s << '0';
			    } else {
				    // we have reached the fraction bits
				    s << ((decoder.parts.fraction & mask) ? '1' : '0');
				    mask >>= 1;
			    }
			    if (nibbleMarker && ddbit != 0 && (ddbit % 4) == 0)
				    s << '\'';
			    --offset;
		    }
	    }

	    return s.str();
    }

    inline std::string to_components(const dd& number, bool nibbleMarker = false) {
	    std::stringstream s;
	    s << std::setprecision(16);
	    constexpr int nrLimbs = 2;
	    for (int i = 0; i < nrLimbs; ++i) {
		    double_decoder decoder;
		    decoder.d = number[i];

		    std::string label = "x[" + std::to_string(i) + "]";
		    s << label << " : ";
		    s << "0b";
		    // print sign bit
		    s << (decoder.parts.sign ? '1' : '0') << '.';

		    // print the segment's exponent bits
		    {
			    uint64_t mask = 0x400;
			    for (int bit = 10; bit >= 0; --bit) {
				    s << ((decoder.parts.exponent & mask) ? '1' : '0');
				    if (nibbleMarker && bit != 0 && (bit % 4) == 0)
					    s << '\'';
				    mask >>= 1;
			    }
		    }

		    s << '.';

		    // print the segment's fraction bits
		    uint64_t mask = (uint64_t(1) << 51);
		    for (int bit = 51; bit >= 0; --bit) {
			    s << ((decoder.parts.fraction & mask) ? '1' : '0');
			    if (nibbleMarker && bit != 0 && (bit % 4) == 0)
				    s << '\'';
			    mask >>= 1;
		    }

		    s << std::scientific << std::showpos << std::setprecision(15);  // we are printing a double
		    s << " : " << number[i] << " : binary scale " << scale(number[i]) << '\n';
	    }

	    return s.str();
    }

	// generate a binary, color-coded representation of the doubledouble
	inline std::string color_print(const dd& r, bool nibbleMarker = false) {
		std::stringstream s;
		double high = r.high();
		double low = r.low();
		s << color_print<double>(high, nibbleMarker) << ", " << color_print<double>(low, nibbleMarker);
		return s.str();
	}

}} // namespace sw::universal
