#pragma once
// exceptions.hpp: exceptions for problems in IEEE float calculations
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <exception>
#include <string>

namespace sw {
	namespace ieee {
		struct divide_by_zero
			: std::runtime_error
		{
			divide_by_zero(const std::string& error = "Divide by zero.") : std::runtime_error(error) {}
		};

	} // namespace ieee

} // namespace sw

