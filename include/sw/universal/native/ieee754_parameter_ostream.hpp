#pragma once
// ieee754_parameter_ostream.hpp: output serialization for ieee754_parameter
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/integers.hpp>

namespace sw { namespace universal {

	// output stream operator for ieee754_parameter<Real>
	template<typename Real>
	::std::ostream& operator<<(::std::ostream& ostr, const ieee754_parameter<Real>& v) {
		ostr << "Total number of bits        : " << v.nbits << '\n';
		ostr << "number of exponent bits     : " << v.ebits << '\n';              // number of exponent bits
		ostr << "number of fraction bits     : " << v.fbits << '\n';              // number of fraction bits		
		ostr << "exponent bias               : " << v.bias << '\n';               // exponent bias
		ostr << "sign field mask             : " << to_binary(v.smask, v.nbits, true) << '\n';   // mask of the sign field
		ostr << "exponent field mask         : " << to_binary(v.emask, v.nbits, true) << '\n';   // mask of the exponent field
		ostr << "mask of exponent value      : " << to_binary(v.eallset, v.ebits, true) << '\n'; // mask of exponent value

		ostr << "mask of hidden bit          : " << to_binary(v.hmask, v.nbits, true) << '\n';    // mask of the hidden bit
		ostr << "fraction field mask         : " << to_binary(v.fmask, v.nbits, true) << '\n';    // mask of the fraction field
		ostr << "significant field mask      : " << to_binary(v.hfmask, v.nbits, true) << '\n';   // mask of the signficicant field, i.e. hidden bit + fraction bits
		ostr << "MSB fraction bit mask       : " << to_binary(v.fmsb, v.nbits, true) << '\n';     // mask of the most significant fraction bit
		ostr << "qNaN pattern                : " << to_binary(v.qnanmask, v.nbits, true) << '\n'; // mask of quiet NaN
		ostr << "sNaN pattern                : " << to_binary(v.snanmask, v.nbits, true) << '\n'; // mask of signalling NaN
		ostr << "smallest normal value       : " << v.minNormal << '\n';           // value of smallest normal value
		ostr << "                            : " << to_binary(v.minNormal) << '\n';
		ostr << "smallest subnormal value    : " << v.minSubnormal << '\n';        // value of the smallest subnormal value
		ostr << "                            : " << to_binary(v.minSubnormal) << '\n';
		ostr << "exponent smallest normal    : " << v.minNormalExp << '\n';      // exponent value of smallest    normal value
		ostr << "exponent smallest subnormal : " << v.minSubnormalExp << '\n';   // exponent value of smallest subnormal value
		return ostr;
	}

}} // namespace sw::universal
