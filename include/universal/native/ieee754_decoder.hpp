#pragma once
// ieee754_decoder.hpp: nonconstexpr union-based decoders for IEEE-754 native types
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
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

}} // namespace sw::universal
