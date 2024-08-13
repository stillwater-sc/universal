#pragma once
// qd_fwd.hpp : forward declarations for the quad-double floating-point environment
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>

namespace sw { namespace universal {

	// forward references
	class qd;

	bool parse(const std::string& number, qd& v);
	
	qd abs(qd const&);
	qd sqrt(qd);
	qd fabs(qd);

	qd fma(qd const&, qd const&, qd const&);

}} // namespace sw::universal

