#pragma once
// ieee754_parameter.hpp: database of compiler-specific parameter values for IEEE-754 native types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>

namespace sw { namespace universal {

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

