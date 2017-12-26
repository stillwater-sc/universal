#pragma once
// exceptions.hpp: exceptions for problems in bitset calculations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <exception>
#include <string>

struct integer_divide_by_zero
  : std::runtime_error
{
    integer_divide_by_zero(const std::string& error = "Integer divide by zero.") : std::runtime_error(error) {}
};

struct cut_off_leading_bit
	: std::runtime_error
{
	cut_off_leading_bit(const std::string& error = "A leading significat bit is cut off.") : std::runtime_error(error) {}
};

struct iteration_bound_too_large
	: std::runtime_error
{
	iteration_bound_too_large(const std::string& error = "Iteration bound too large") : std::runtime_error(error) {}
};