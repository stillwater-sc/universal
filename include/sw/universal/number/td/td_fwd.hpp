#pragma once
// td_fwd.hpp : forward declarations of the triple-double (td) floating-point environment
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <string>

namespace sw { namespace universal {

	// forward references
	class td;

	bool parse(const std::string& number, td& v);
	
	td abs(td);
	td sqrt(td);
	td fabs(td);

	td fma(td const&, td const&, td const&);

}} // namespace sw::universal

