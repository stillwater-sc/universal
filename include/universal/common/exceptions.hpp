#pragma once
// exceptions.hpp: base exceptions of the Universal exception hierarchy
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <stdexcept>
#include <string>

namespace sw { namespace universal {

///////////////////////////////////////////////////////////////
// base universal arithmetic exceptions

struct universal_arithmetic_exception : public std::runtime_error {
	universal_arithmetic_exception() : std::runtime_error("default universal arithmetic error") {}
	universal_arithmetic_exception(const std::string& err) : std::runtime_error(err) {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

struct universal_internal_exception : public std::runtime_error {
	universal_internal_exception() : std::runtime_error("default universal internal error") {}
	universal_internal_exception(const std::string& err) : std::runtime_error(err) {}
};

}} // namespace sw::universal
