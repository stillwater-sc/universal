#pragma once
// arithmetic.hpp: enum types used to classify arithmetic behavior, 
//                 such as Modular vs Saturating arithmetic, Projective vs Real arithmetic

//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>

namespace sw { namespace universal {

	enum class Behavior : uint8_t {Saturating, Wrapping};

	inline std::string type_tag(Behavior behavior) {
		switch (behavior) {
		case Behavior::Saturating:
			return "Saturating";
		case Behavior::Wrapping:
			return "Wrapping";
		default:
			return "unknown arithmetic behavior";
		}
	}

}} // namespace sw::universal
