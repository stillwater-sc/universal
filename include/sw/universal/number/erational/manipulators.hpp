#pragma once
// manipulators.hpp: definition of manipulation functions for adaptive precision decimal rational types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Generate a type tag for rational type
	std::string type_tag(const erational& = {}) {
		return "erational";
	}

}} // namespace sw::universal
