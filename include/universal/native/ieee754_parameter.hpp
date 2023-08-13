#pragma once
// ieee754_parameter.hpp: database of compiler-specific parameter values for IEEE-754 native types
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/integers.hpp>

namespace sw { namespace universal {

	// forward definition of to_binary(Real, bool)
	std::string to_binary(float, bool);
	std::string to_binary(double, bool);
	std::string to_binary(long double, bool);

	// IEEE-754 parameter constexpressions
	template<typename Real>
	class ieee754_parameter {
	public:
		static constexpr int      nbits           = 0; // number of bits total
		static constexpr uint64_t smask           = 0; // mask of the sign field
		static constexpr int      ebits           = 0; // number of exponent bits
		static constexpr int      bias            = 0; // exponent bias
		static constexpr uint64_t emask           = 0; // mask of the exponent field
		static constexpr uint64_t eallset         = 0; // mask of exponent value
		static constexpr int      fbits           = 0; // number of fraction bits
		static constexpr uint64_t hmask           = 0; // mask of the hidden bit
		static constexpr uint64_t fmask           = 0; // mask of the fraction field
		static constexpr uint64_t hfmask          = 0; // mask of the signficicant field, i.e. hidden bit + fraction bits
		static constexpr uint64_t fmsb            = 0; // mask of the most significant fraction bit
		static constexpr uint64_t qnanmask        = 0; // mask of quiet NaN
		static constexpr uint64_t snanmask        = 0; // mask of signalling NaN
		static constexpr Real     minNormal       = Real(0.0); // value of smallest normal value
		static constexpr Real     minSubnormal    = Real(0.0); // value of the smallest subnormal value
		static constexpr int      minNormalExp    = 0;   // exponent value of smallest normal value
		static constexpr int      minSubnormalExp = 0;   // exponent value of smallest subnormal value
	};
	
	// *********************************  NOTE **************************************
	// IEEE-754 parameter specializations are in the compiler specific sections below


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

// *****************  NOTE ************************
// compiler specializations for IEEE-754 parameters
#include <universal/native/compiler/ieee754_msvc.hpp>
#include <universal/native/compiler/ieee754_clang.hpp>
#include <universal/native/compiler/ieee754_gcc.hpp>
#include <universal/native/compiler/ieee754_intelicc.hpp>
#include <universal/native/compiler/ieee754_riscv.hpp>
#include <universal/native/compiler/ieee754_ibmxlc.hpp>
#include <universal/native/compiler/ieee754_hpcc.hpp>
#include <universal/native/compiler/ieee754_pgi.hpp>
#include <universal/native/compiler/ieee754_sunpro.hpp>

