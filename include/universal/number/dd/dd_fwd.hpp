#pragma once
// dd_fwd.hpp : forward declarations of the double-double (dd) floating-point environment
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <string>

namespace sw { namespace universal {

	// forward references
	class dd;

	bool parse(const std::string& number, dd& v);
	
	dd abs(dd);
	dd sqrt(dd);
	dd fabs(dd);

	dd fma(dd const&, dd const&, dd const&);

}} // namespace sw::universal

