#pragma once
// posit_fwd.hpp :  forward declaration of the posit class
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
namespace unum {

	// Forward definitions
	template<size_t nbits, size_t es> class posit;
	template<size_t nbits, size_t es> posit<nbits, es> abs(const posit<nbits, es>& p);
	template<size_t nbits, size_t es> posit<nbits, es> sqrt(const posit<nbits, es>& p);
	template<size_t nbits, size_t es> constexpr posit<nbits, es>& minpos(posit<nbits, es>& p);
	template<size_t nbits, size_t es> constexpr posit<nbits, es>& maxpos(posit<nbits, es>& p);

} // namespace unum
} // namespace sw

