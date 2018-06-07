#pragma once
// exceptions.hpp: exceptions for problems in posit calculations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <stdexcept>
#include <string>

struct divide_by_zero 
  : std::runtime_error
{
	divide_by_zero(const std::string& error = "Divide by zero.") : std::runtime_error(error) {}
};

struct integer_divide_by_zero 
  : std::runtime_error
{
	integer_divide_by_zero(const std::string& error = "Integer divide by zero.") : std::runtime_error(error) {}
};

struct shift_too_large
  : std::runtime_error
{
    shift_too_large(const std::string& error = "Shift value too large for given bitset.") : std::runtime_error(error) {}
};

struct hpos_too_large
  : std::runtime_error
{
    hpos_too_large(const std::string& error = "Position of hidden bit too large for given bitset.") : std::runtime_error(error) {}
};

struct rbits_too_large
  : std::runtime_error
{
    rbits_too_large(const std::string& error = "Number of remaining bits too large for this fraction.") : std::runtime_error(error) {}
};

struct cut_off_leading_bit
  : std::runtime_error
{
    cut_off_leading_bit(const std::string& error = "A leading significant bit is cut off.") : std::runtime_error(error) {}
};

struct iteration_bound_too_large
  : std::runtime_error
{
    iteration_bound_too_large(const std::string& error = "Iteration bound is too large.") : std::runtime_error(error) {}
};

struct round_off_all
  : std::runtime_error
{
    round_off_all(const std::string& error = "Cannot round off all bits.") : std::runtime_error(error) {}
};
