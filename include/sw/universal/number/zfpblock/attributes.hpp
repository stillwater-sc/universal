#pragma once
// attributes.hpp: functions to query zfpblock number system attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cstddef>
#include <string>
#include <sstream>
#include <iomanip>

namespace sw { namespace universal {

	// Display information about a zfpblock configuration
	template<typename Real, unsigned Dim>
	inline std::string zfp_block_info() {
		using ZFP = zfpblock<Real, Dim>;
		std::stringstream s;
		s << std::setw(20) << type_tag(ZFP{}) << " : "
		  << "dim=" << Dim
		  << ", block_size=" << ZFP::BLOCK_SIZE
		  << ", max_bytes=" << ZFP::MAX_BYTES
		  << ", element=" << (std::is_same_v<Real, float> ? "float" : "double");
		return s.str();
	}

	// Display compression statistics
	template<typename Real, unsigned Dim>
	inline std::string zfp_compression_stats(const zfpblock<Real, Dim>& blk) {
		std::stringstream s;
		s << "compressed: " << blk.compressed_bits() << " bits"
		  << " (" << blk.compressed_bytes() << " bytes)"
		  << ", ratio: " << std::fixed << std::setprecision(2) << blk.compression_ratio() << "x"
		  << ", uncompressed: " << (blk.block_size() * sizeof(Real)) << " bytes";
		return s.str();
	}

}} // namespace sw::universal
