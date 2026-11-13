#pragma once
// exceptions.hpp: exceptions for problems in blockdigit calculations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <stdexcept>
#include <string>

// blockdigit is a building block: the consuming number system decides its exception behavior
// by defining BLOCKDIGIT_THROW_ARITHMETIC_EXCEPTION before including blockdigit.hpp.
//   0 (default): x / 0 and x % 0 return 0
//   1          : x / 0 and x % 0 throw blockdigit_divide_by_zero (abort under -fno-exceptions)

namespace sw { namespace universal {

// base class for blockdigit arithmetic exceptions
struct blockdigit_arithmetic_exception : public std::runtime_error {
	explicit blockdigit_arithmetic_exception(const std::string& error)
		: std::runtime_error(std::string("blockdigit arithmetic exception: ") + error) {}
};

// is thrown when the divisor is 0 in a division or remainder operator
struct blockdigit_divide_by_zero : public blockdigit_arithmetic_exception {
	explicit blockdigit_divide_by_zero(const std::string& error = "divide by zero")
		: blockdigit_arithmetic_exception(error) {}
};

}} // namespace sw::universal
