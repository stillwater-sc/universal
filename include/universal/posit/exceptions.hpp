#pragma once
// exceptions.hpp: exceptions for problems in posit calculations
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <stdexcept>
#include <string>

// TODO: why can't I namespace exceptions?
//namespace sw {
//	namespace unum {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// POSIT ARITHMETIC EXCEPTIONS

// base class for posit arithmetic exceptions
struct posit_arithmetic_exception
	: public std::runtime_error
{			
	posit_arithmetic_exception(const std::string& error) : std::runtime_error(std::string("posit arithmetic exception: ") + error) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// not_a_real is thrown when a rvar is NaR
struct not_a_real
	: posit_arithmetic_exception
{
	not_a_real(const std::string& error = "nar (not a real)") : posit_arithmetic_exception(error) {}
};

// divide_by_zero is thrown when the denominator in a division operator is 0
struct divide_by_zero 
	: posit_arithmetic_exception
{
	divide_by_zero(const std::string& error = "divide by zero") : posit_arithmetic_exception(error) {}
};

// divide_by_nar is thrown when the denominator in a division operator is NaR
struct divide_by_nar
	: posit_arithmetic_exception
{
	divide_by_nar(const std::string& error = "divide by nar") : posit_arithmetic_exception(error) {}
};

// numerator_is_nar is thrown when the numerator in a division operator is NaR
struct numerator_is_nar
	: posit_arithmetic_exception
{
	numerator_is_nar(const std::string& error = "numerator is nar") : posit_arithmetic_exception(error) {}
};

// operand_is_nar is thrown when an rvar in a binary operator is NaR
struct operand_is_nar
	: public posit_arithmetic_exception
{
	operand_is_nar(const std::string& error = "operand is nar") : posit_arithmetic_exception(error) {}
};

// thrown when division yields no signficant fraction bits
struct division_result_is_zero
	: posit_arithmetic_exception
{
	division_result_is_zero(const std::string& error = "division yielded no significant bits") : posit_arithmetic_exception(error) {}
};

// thrown when division yields NaR
struct division_result_is_infinite
	: posit_arithmetic_exception
{
	division_result_is_infinite(const std::string& error = "division yielded infinite") : posit_arithmetic_exception(error) {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// POSIT INTERNAL OPERATION EXCEPTIONS

struct posit_internal_exception
	: public std::runtime_error
{		
	posit_internal_exception(const std::string& error) : std::runtime_error(std::string("posit internal exception") + error) {};

};

struct shift_too_large
	: posit_internal_exception
{
	shift_too_large(const std::string& error = "shift value too large for given posit") : posit_internal_exception(error) {}
};

struct hpos_too_large
	: posit_internal_exception
{
	hpos_too_large(const std::string& error = "position of hidden bit too large for given posit") : posit_internal_exception(error) {}
};

struct rbits_too_large
	: posit_internal_exception
{
	rbits_too_large(const std::string& error = "number of remaining bits too large for this fraction") :posit_internal_exception(error) {}
};

struct cut_off_leading_bit
	: posit_internal_exception
{
	cut_off_leading_bit(const std::string& error = "leading significant bit is cut off") : posit_internal_exception(error) {}
};

struct iteration_bound_too_large
	: posit_internal_exception
{
	iteration_bound_too_large(const std::string& error = "iteration bound is too large") : posit_internal_exception(error) {}
};

struct round_off_all
	: posit_internal_exception
{
	round_off_all(const std::string& error = "cannot round off all bits") : posit_internal_exception(error) {}
};


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

