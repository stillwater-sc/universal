#pragma once
// dfloat_fwd.hpp :  forward declarations of the decimal floating-point dfloat environment
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>

namespace sw { namespace universal {

	// forward references
	class dd;

	bool parse(const std::string& number, dd& v);
	
	dd abs(const dd);
	dd sqrt(const dd);
	dd fabs(const dd);

}} // namespace sw::universal

