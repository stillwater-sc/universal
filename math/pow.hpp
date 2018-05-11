#pragma once
// pow.hpp: pow functions for posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace sw {
	namespace unum {

		template<size_t nbits, size_t es>
		posit<nbits,es> pow(posit<nbits,es> x, posit<nbits, es> y) {
			return posit<nbits,es>(std::pow(double(x), double(y)));
		}

	}  // namespace unum

}  // namespace sw
