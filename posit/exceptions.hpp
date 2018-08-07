#pragma once
// exceptions.hpp: exceptions for problems in posit calculations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <stdexcept>
#include <string>

struct not_a_real
	: std::runtime_error
{
	not_a_real(const std::string& error = "nar (not a real)") : std::runtime_error(error) {}
};

struct divide_by_zero 
  : std::runtime_error
{
	divide_by_zero(const std::string& error = "divide by zero") : std::runtime_error(error) {}
};

struct divide_by_nar
	: std::runtime_error
{
	divide_by_nar(const std::string& error = "divide by nar") : std::runtime_error(error) {}
};

struct numerator_is_nar
	: std::runtime_error
{
	numerator_is_nar(const std::string& error = "numerator is nar") : std::runtime_error(error) {}
};

struct operand_is_nar
	: std::runtime_error
{
	operand_is_nar(const std::string& error = "operand is nar") : std::runtime_error(error) {}
};

struct integer_divide_by_zero 
  : std::runtime_error
{
	integer_divide_by_zero(const std::string& error = "integer divide by zero") : std::runtime_error(error) {}
};

struct shift_too_large
  : std::runtime_error
{
    shift_too_large(const std::string& error = "shift value too large for given posit") : std::runtime_error(error) {}
};

struct hpos_too_large
  : std::runtime_error
{
    hpos_too_large(const std::string& error = "position of hidden bit too large for given posit") : std::runtime_error(error) {}
};

struct rbits_too_large
  : std::runtime_error
{
    rbits_too_large(const std::string& error = "number of remaining bits too large for this fraction") : std::runtime_error(error) {}
};

struct cut_off_leading_bit
  : std::runtime_error
{
    cut_off_leading_bit(const std::string& error = "leading significant bit is cut off") : std::runtime_error(error) {}
};

struct iteration_bound_too_large
  : std::runtime_error
{
    iteration_bound_too_large(const std::string& error = "iteration bound is too large") : std::runtime_error(error) {}
};

struct round_off_all
  : std::runtime_error
{
    round_off_all(const std::string& error = "cannot round off all bits") : std::runtime_error(error) {}
};

struct division_result_is_zero
	: std::runtime_error
{
	division_result_is_zero(const std::string& error = "division yielded no significant bits") : std::runtime_error(error) {}
};

struct division_result_is_infinite
	: std::runtime_error
{
	division_result_is_infinite(const std::string& error = "division yielded infinite") : std::runtime_error(error) {}
};

struct operand_too_large_for_quire
	: std::runtime_error
{
	operand_too_large_for_quire(const std::string& error = "operand value too large for quire") : std::runtime_error(error) {}
};

struct operand_too_small_for_quire
	: std::runtime_error
{
	operand_too_small_for_quire(const std::string& error = "operand value too small for quire") : std::runtime_error(error) {}
};
