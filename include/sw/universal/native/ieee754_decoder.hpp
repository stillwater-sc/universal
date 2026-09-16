#pragma once
// ieee754_decoder.hpp: nonconstexpr union-based decoders for IEEE-754 native types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	////////////////////////////////////////////////////////////////////////
	// union structure helper for single precision floating-point

	union float_decoder {
		float_decoder() : f{ 0.0f } {}
		float_decoder(float _f) : f{ _f } {}
		float f;
		struct {
			uint32_t fraction : 23;
			uint32_t exponent : 8;
			uint32_t sign : 1;
		} parts;
		uint32_t bits;
	};

	////////////////////////////////////////////////////////////////////////
	// union structure helper for double precision floating-point

	union double_decoder {
		double_decoder() : d{ 0.0 } {}
		double_decoder(double _d) : d{ _d } {}
		double d;
		struct {
			uint64_t fraction : 52;
			uint64_t exponent : 11;
			uint64_t sign : 1;
		} parts;
		uint64_t bits;
	};



	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// compiler specific long double IEEE floating point

	/*
	 * In contrast to the single and double-precision formats, this format does not utilize an implicit/hidden bit. 
	 * Rather, bit 63 contains the integer part of the significand and bits 62-0 hold the fractional part. 
	 * Bit 63 will be 1 on all normalized numbers.
	 */

#if defined(ALTERNATIVE_DATA)
#if defined(__aarch64__)
	union long_double_decoder {
		long_double_decoder() : ld{ 0.0l } {}
		long_double_decoder(long double _ld) : ld{ _ld } {}
		long double ld;
		struct {
			uint64_t fraction : 112;
			uint64_t exponent : 15;
			uint64_t sign : 1;
		} parts;
		uint64_t bits[2];
	};
#else
	// long double decoder
	union long_double_decoder {
		long double ld;
		struct {
			uint64_t fraction : 63;
			uint64_t bit63 : 1;
			uint64_t exponent : 15;
			uint64_t sign : 1;
		} parts;
		uint64_t bits[2];
};
#endif // defined(__aarch64__)
#endif // alternative_data

// __arm__ which is defined for 32bit arm, and 32bit arm only.
// __aarch64__ which is defined for 64bit arm, and 64bit arm only.

#if defined(UNIVERSAL_ARCH_POWER)
	union long_double_decoder {
		long_double_decoder() : ld{ 0.0l } {}
		long_double_decoder(long double _ld) : ld{ _ld } {}
		long double ld;
		struct {
			uint64_t fraction : 64;
			uint64_t upper : 48;
			uint64_t exponent : 15;
			uint64_t sign : 1;
		} parts;
		uint64_t bits[2];
	};

#elif defined(UNIVERSAL_ARCH_X86_64)

	union long_double_decoder {
		long_double_decoder() : ld{ 0.0l } {}
		long_double_decoder(long double _ld) : ld{ _ld } {}
		long double ld;
		struct {
			uint64_t fraction : 63;
			uint64_t bit63 : 1;
			uint64_t exponent : 15;
			uint64_t sign : 1;
		} parts;
		uint64_t bits[2];
	};

#elif defined(UNIVERSAL_ARCH_ARM)

#if __LDBL_MANT_DIG__ == 113
	// GCC on Linux aarch64: IEEE 754 binary128 (same layout as POWER)
	union long_double_decoder {
		long_double_decoder() : ld{ 0.0l } {}
		long_double_decoder(long double _ld) : ld{ _ld } {}
		long double ld;
		struct {
			uint64_t fraction : 64;
			uint64_t upper : 48;
			uint64_t exponent : 15;
			uint64_t sign : 1;
		} parts;
		uint64_t bits[2];
	};
#else
	// Apple Clang on macOS aarch64: long double == double
	union long_double_decoder {
		long_double_decoder() : ld{ 0.0l } {}
		long_double_decoder(long double _ld) : ld{ _ld } {}
		long double ld;
		struct {
			uint64_t fraction : 52;
			uint64_t exponent : 11;
			uint64_t sign : 1;
		} parts;
		uint64_t bits[2];
	};
#endif

#elif defined(UNIVERSAL_ARCH_RISCV)

#if __LDBL_MANT_DIG__ == 113
	// the RISC-V ABI defines long double as IEEE 754 binary128: 1 sign, 15 exponent, 112 fraction,
	// and no explicit integer bit. This used to carry the x87 layout -- "for the moment, just use
	// the x86 interpretation" -- which names the same fields over a different format (#1399).
	union long_double_decoder {
		long_double_decoder() : ld{ 0.0l } {}
		long_double_decoder(long double _ld) : ld{ _ld } {}
		long double ld;
		struct {
			uint64_t fraction : 64;   // the low 64 of the 112 fraction bits
			uint64_t upper : 48;      // the high 48
			uint64_t exponent : 15;
			uint64_t sign : 1;
		} parts;
		uint64_t bits[2];
	};
#else
	// A RISC-V target whose long double is not binary128 is not supported by the GNU long double
	// path: its renderers carry an x87 and a binary128 shape only, and ieee754_parameter<long
	// double> describes the x87 one. Rather than a decoder that implies support the rest of the
	// layer does not have, this keeps the previous layout and says what it is.
#pragma message("RISC-V long double is not IEEE binary128: the x87 layout below does not describe it")
	union long_double_decoder {
		long_double_decoder() : ld{ 0.0l } {}
		long_double_decoder(long double _ld) : ld{ _ld } {}
		long double ld;
		struct {
			uint64_t fraction : 63;
			uint64_t bit63 : 1;
			uint64_t exponent : 15;
			uint64_t sign : 1;
		} parts;
		uint64_t bits[2];
	};
#endif

#else
#pragma message("long double unsupported for unidentified architecture")
#endif

}} // namespace sw::universal
