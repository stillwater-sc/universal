#pragma once
// exceptions.hpp: exceptions for problems in blockbinary calculations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <stdexcept>
#include <string>

// blockbinary is a building block: the consuming number system decides its exception behavior
// by defining BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION before including blockbinary.hpp.
//   0 (default): x / 0 and x % 0 return 0
//   1          : x / 0 and x % 0 throw blockbinary_divide_by_zero (abort under -fno-exceptions)

namespace sw { namespace universal {

// base class for blockbinary arithmetic exceptions
struct blockbinary_arithmetic_exception : public std::runtime_error {
	explicit blockbinary_arithmetic_exception(const std::string& error)
		: std::runtime_error(std::string("blockbinary arithmetic exception: ") + error) {}
};

// is thrown when the divisor is 0 in a division or remainder operator
struct blockbinary_divide_by_zero : public blockbinary_arithmetic_exception {
	explicit blockbinary_divide_by_zero(const std::string& error = "divide by zero")
		: blockbinary_arithmetic_exception(error) {}
};

}} // namespace sw::universal
