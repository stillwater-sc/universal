#pragma once
// exceptions.hpp: exceptions for problems in quire calculations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <stdexcept>
#include <string>

namespace sw { namespace universal {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// QUIRE ARITHMETIC EXCEPTIONS

// base class for quire exceptions
struct quire_exception
	: public std::runtime_error
{
	quire_exception(const std::string& error) : std::runtime_error(std::string("quire exception: ") + error) {};
};

struct operand_too_large_for_quire
	: public quire_exception
{
	operand_too_large_for_quire(const std::string& error = "operand value too large for quire") : quire_exception(error) {}
};

struct operand_too_small_for_quire
	: public quire_exception
{
	operand_too_small_for_quire(const std::string& error = "operand value too small for quire") : quire_exception(error) {}
};

}} // namespace sw::universal
