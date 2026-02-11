#pragma once
// manipulators.hpp: definition of manipulation functions for nvblock
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <string>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <universal/number/nvblock/nvblock_fwd.hpp>

namespace sw { namespace universal {

	// Generate a type tag for nvblock types
	template<typename ElementType, size_t BlockSize, typename ScaleType>
	inline std::string type_tag(const nvblock<ElementType, BlockSize, ScaleType>& = {}) {
		// Return NVIDIA alias for the canonical configuration
		if constexpr (std::is_same_v<ElementType, e2m1> &&
			BlockSize == 16 &&
			std::is_same_v<ScaleType, e4m3>) {
			return "nvfp4";
		}

		// Generic fallback: build descriptive name
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
		} else {
			elemName = "unknown";
		}

		std::string scaleName;
		if constexpr (std::is_same_v<ScaleType, e4m3>) {
			scaleName = "e4m3";
		} else if constexpr (std::is_same_v<ScaleType, e5m2>) {
			scaleName = "e5m2";
		} else {
			scaleName = "unknown";
		}

		std::stringstream s;
		s << "nvblock<" << elemName << "," << BlockSize << "," << scaleName << ">";
		return s.str();
	}

	// generate a binary representation of the nvblock scale and elements
	template<typename ElementType, size_t BlockSize, typename ScaleType>
	inline std::string to_binary(const nvblock<ElementType, BlockSize, ScaleType>& blk, bool bNibbleMarker = false) {
		std::stringstream s;
		s << "scale:" << to_binary(blk.block_scale(), bNibbleMarker) << " elements:[";
		for (size_t i = 0; i < BlockSize; ++i) {
			if (i > 0) s << ',';
			s << to_binary(blk.element(i), bNibbleMarker);
			if (i >= 3 && BlockSize > 6) {
				s << ",...(" << (BlockSize - i - 1) << " more)";
				break;
			}
		}
		s << ']';
		return s.str();
	}

}} // namespace sw::universal
