#pragma once
// manipulators.hpp: definition of manipulation functions for zfpblock
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <string>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <universal/number/zfpblock/zfpblock_fwd.hpp>

namespace sw { namespace universal {

	// Generate a type tag for zfpblock types
	template<typename Real, unsigned Dim>
	inline std::string type_tag(const zfpblock<Real, Dim>& = {}) {
		// Return short aliases for canonical configurations
		if constexpr (std::is_same_v<Real, float> && Dim == 1) return "zfp1f";
		else if constexpr (std::is_same_v<Real, float> && Dim == 2) return "zfp2f";
		else if constexpr (std::is_same_v<Real, float> && Dim == 3) return "zfp3f";
		else if constexpr (std::is_same_v<Real, double> && Dim == 1) return "zfp1d";
		else if constexpr (std::is_same_v<Real, double> && Dim == 2) return "zfp2d";
		else if constexpr (std::is_same_v<Real, double> && Dim == 3) return "zfp3d";
		else {
			std::stringstream s;
			s << "zfpblock<" << (std::is_same_v<Real, float> ? "float" : "double")
			  << "," << Dim << ">";
			return s.str();
		}
	}

	// Generate a binary representation of the compressed buffer
	template<typename Real, unsigned Dim>
	inline std::string to_binary(const zfpblock<Real, Dim>& blk, bool = false) {
		std::stringstream s;
		size_t nbytes = blk.compressed_bytes();
		const uint8_t* buf = blk.data();
		s << "bits:" << blk.compressed_bits() << " [";
		for (size_t i = 0; i < nbytes && i < 16; ++i) {
			if (i > 0) s << ' ';
			s << std::hex << std::setfill('0') << std::setw(2)
			  << static_cast<unsigned>(buf[i]);
		}
		if (nbytes > 16) s << " ...(" << (nbytes - 16) << " more bytes)";
		s << ']';
		return s.str();
	}

	// Generate hex representation of the compressed buffer
	template<typename Real, unsigned Dim>
	inline std::string to_hex(const zfpblock<Real, Dim>& blk) {
		std::stringstream s;
		size_t nbytes = blk.compressed_bytes();
		const uint8_t* buf = blk.data();
		for (size_t i = 0; i < nbytes; ++i) {
			s << std::hex << std::setfill('0') << std::setw(2)
			  << static_cast<unsigned>(buf[i]);
		}
		return s.str();
	}

}} // namespace sw::universal
