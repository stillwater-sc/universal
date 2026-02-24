#pragma once
// rational_fwd.hpp: forward references for fixed-sized arbitrary configuration rational arithmetic type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Radix tag types for rational template
	struct base2  { static constexpr unsigned radix = 2;  };
	struct base8  { static constexpr unsigned radix = 8;  };
	struct base10 { static constexpr unsigned radix = 10; };
	struct base16 { static constexpr unsigned radix = 16; };

	// Primary template (unspecialized)
	template<unsigned _ndigits, typename Base = base2, typename bt = uint8_t> class rational;

	// Forward reference for abs
	template<unsigned nbits, typename Base, typename bt>
	rational<nbits, Base, bt> abs(const rational<nbits, Base, bt>&);

}} // namespace sw::universal

