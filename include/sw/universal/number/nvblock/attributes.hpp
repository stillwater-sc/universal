#pragma once
// attributes.hpp: functions to query nvblock number system attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace sw { namespace universal {

	// return the block scale factor of an nvblock as float
	template<typename ElementType, size_t BlockSize, typename ScaleType>
	inline float block_scale(const nvblock<ElementType, BlockSize, ScaleType>& blk) {
		return blk.block_scale().to_float();
	}

	// compute the effective dynamic range of an nvblock format
	template<typename ElementType, size_t BlockSize, typename ScaleType>
	inline std::string nvblock_range() {
		using NV = nvblock<ElementType, BlockSize, ScaleType>;
		std::stringstream s;
		s << std::setw(40) << type_tag(NV{}) << " : ";

		// Element max in log2
		ElementType elem_mp;
		elem_mp.maxpos();
		double elem_max_log2 = std::log2(static_cast<double>(elem_mp.to_float()));

		// Scale max in log2
		ScaleType scale_mp;
		scale_mp.maxpos();
		double scale_max_log2 = std::log2(static_cast<double>(scale_mp.to_float()));

		// Scale min in log2 (smallest positive)
		ScaleType scale_minp;
		scale_minp.minpos();
		double scale_min_log2 = std::log2(static_cast<double>(scale_minp.to_float()));

		// Block-only range (without tensor_scale): scale_min to scale_max * elem_max
		double total_max_log2 = scale_max_log2 + elem_max_log2;
		double total_min_log2 = scale_min_log2;

		int decades = static_cast<int>((total_max_log2 - total_min_log2) / std::log2(10.0));

		s << "max = 2^" << std::fixed << std::setprecision(1) << total_max_log2;
		s << ", min = 2^" << total_min_log2;
		s << " (~" << decades << " decades, without tensor_scale)";
		return s.str();
	}

}} // namespace sw::universal
