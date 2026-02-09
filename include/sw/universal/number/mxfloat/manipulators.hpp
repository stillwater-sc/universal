#pragma once
// manipulators.hpp: definition of manipulation functions for mxblock
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <universal/number/mxfloat/mxfloat_fwd.hpp>

namespace sw { namespace universal {

	// Generate a type tag for mxblock types
	template<typename ElementType, size_t BlockSize>
	inline std::string type_tag(const mxblock<ElementType, BlockSize>& = {}) {
		std::string elemName;
		if constexpr (std::is_same_v<ElementType, e2m1>) {
			elemName = "e2m1";
		} else if constexpr (std::is_same_v<ElementType, e2m3>) {
			elemName = "e2m3";
		} else if constexpr (std::is_same_v<ElementType, e3m2>) {
			elemName = "e3m2";
		} else if constexpr (std::is_same_v<ElementType, e4m3>) {
			elemName = "e4m3";
		} else if constexpr (std::is_same_v<ElementType, e5m2>) {
			elemName = "e5m2";
		} else if constexpr (std::is_same_v<ElementType, int8_t>) {
			elemName = "int8";
		} else {
			elemName = "unknown";
		}

		// Return OCP alias names for standard configurations
		if constexpr (BlockSize == 32) {
			if constexpr (std::is_same_v<ElementType, e2m1>) return "mxfp4";
			if constexpr (std::is_same_v<ElementType, e3m2>) return "mxfp6";
			if constexpr (std::is_same_v<ElementType, e2m3>) return "mxfp6e2m3";
			if constexpr (std::is_same_v<ElementType, e4m3>) return "mxfp8";
			if constexpr (std::is_same_v<ElementType, e5m2>) return "mxfp8e5m2";
			if constexpr (std::is_same_v<ElementType, int8_t>) return "mxint8";
		}

		// Generic fallback
		std::stringstream s;
		s << "mxblock<" << elemName << "," << BlockSize << ">";
		return s.str();
	}

	// generate a binary representation of the mxblock scale and elements
	template<typename ElementType, size_t BlockSize>
	inline std::string to_binary(const mxblock<ElementType, BlockSize>& blk, bool bNibbleMarker = false) {
		std::stringstream s;
		s << "scale:" << to_binary(blk.scale(), bNibbleMarker) << " elements:[";
		for (size_t i = 0; i < BlockSize; ++i) {
			if (i > 0) s << ',';
			if constexpr (std::is_integral_v<ElementType>) {
				// int8_t: show as 8-bit binary
				uint8_t bits = static_cast<uint8_t>(blk.element(i));
				s << "0b";
				for (int j = 7; j >= 0; --j) {
					s << ((bits & (1u << j)) ? '1' : '0');
				}
			} else {
				s << to_binary(blk.element(i), bNibbleMarker);
			}
			if (i >= 3 && BlockSize > 6) {
				s << ",...(" << (BlockSize - i - 1) << " more)";
				break;
			}
		}
		s << ']';
		return s.str();
	}

}} // namespace sw::universal
