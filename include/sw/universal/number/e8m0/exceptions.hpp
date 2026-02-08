#pragma once
// exceptions.hpp: definition of e8m0 exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for e8m0 arithmetic exceptions
struct e8m0_arithmetic_exception : public universal_arithmetic_exception
{
	explicit e8m0_arithmetic_exception(const std::string& error)
		: universal_arithmetic_exception(std::string("e8m0 arithmetic exception: ") + error) {};
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

struct e8m0_internal_exception : public universal_internal_exception
{
	explicit e8m0_internal_exception(const std::string& error)
		: universal_internal_exception(std::string("e8m0 internal exception: ") + error) {};
};

}} // namespace sw::universal
