#pragma once
// attributes.hpp: functions to query mxblock number system attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <string>
#include <sstream>
#include <cmath>

namespace sw { namespace universal {

	// return the shared scale factor of an mxblock
	template<typename ElementType, size_t BlockSize>
	inline float scale(const mxblock<ElementType, BlockSize>& blk) {
		return blk.scale().to_float();
	}

	// compute the effective dynamic range of an mxblock format
	template<typename ElementType, size_t BlockSize>
	inline std::string mxblock_range() {
		using MX = mxblock<ElementType, BlockSize>;
		std::stringstream s;
		s << std::setw(40) << type_tag(MX{}) << " : ";

		// Compute range in log2 domain to avoid overflow
		// e8m0 scale range: 2^(-127) to 2^(127)
		// Element range depends on ElementType
		double elem_max_log2;
		if constexpr (std::is_integral_v<ElementType>) {
			elem_max_log2 = std::log2(127.0);
		} else {
			ElementType mp;
			mp.maxpos();
			elem_max_log2 = std::log2(static_cast<double>(mp.to_float()));
		}

		// Total range in log2: scale_max_log2 + elem_max_log2 = 127 + elem_max_log2
		// Total min in log2: scale_min_log2 = -127
		double total_max_log2 = 127.0 + elem_max_log2;
		double total_min_log2 = -127.0;

		// Convert to decades: log10(x) = log2(x) / log2(10)
		int decades = static_cast<int>((total_max_log2 - total_min_log2) / std::log2(10.0));

		s << "max = 2^" << std::fixed << std::setprecision(1) << total_max_log2;
		s << ", min = 2^" << total_min_log2;
		s << " (~" << decades << " decades)";
		return s.str();
	}

}} // namespace sw::universal
