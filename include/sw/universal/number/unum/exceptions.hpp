#pragma once
// exceptions.hpp: definition of unum Type I exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <stdexcept>
#include <string>

namespace sw { namespace universal {

// base class for unum arithmetic exceptions
struct unum_arithmetic_exception : public std::runtime_error {
	unum_arithmetic_exception(const std::string& err) : std::runtime_error(err) {}
};

// divide by zero arithmetic exception for unum
struct unum_divide_by_zero : public unum_arithmetic_exception {
	unum_divide_by_zero() : unum_arithmetic_exception("unum division by zero") {}
};

// overflow arithmetic exception for unum
struct unum_overflow : public unum_arithmetic_exception {
	unum_overflow() : unum_arithmetic_exception("unum overflow") {}
};

// base class for unum internal exceptions
struct unum_internal_exception : public std::runtime_error {
	unum_internal_exception(const std::string& err) : std::runtime_error(err) {}
};

}} // namespace sw::universal
