#pragma once
// exceptions.hpp: exception hierarchy for exceptions during arithmetic calculations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// BASE ARITHMETIC EXCEPTIONS

// base class for twoparam arithmetic exceptions
struct twoparam_arithmetic_exception : public universal_arithmetic_exception {			
	twoparam_arithmetic_exception(const std::string& error) 
		: universal_arithmetic_exception(std::string("twoparam arithmetic exception: ") + error) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// not_a_real is thrown when a rvar is NaR
struct twoparam_nar : public twoparam_arithmetic_exception {
	twoparam_nar() : twoparam_arithmetic_exception("nar (not a real)") {}
};

// divide_by_zero is thrown when the denominator in a division operator is 0
struct twoparam_divide_by_zero : public twoparam_arithmetic_exception {
	twoparam_divide_by_zero() : twoparam_arithmetic_exception("divide by zero") {}
};

// divide_by_nar is thrown when the denominator in a division operator is NaR
struct twoparam_divide_by_nar : public twoparam_arithmetic_exception {
	twoparam_divide_by_nar() : twoparam_arithmetic_exception("divide by nar") {}
};

// numerator_is_nar is thrown when the numerator in a division operator is NaR
struct twoparam_numerator_is_nar : public twoparam_arithmetic_exception {
	twoparam_numerator_is_nar() : twoparam_arithmetic_exception("numerator is nar") {}
};

// operand_is_nar is thrown when an rvar in a binary operator is NaR
struct twoparam_operand_is_nar : public twoparam_arithmetic_exception {
	twoparam_operand_is_nar() : twoparam_arithmetic_exception("operand is nar") {}
};

// thrown when division yields no signficant fraction bits
struct twoparam_division_result_is_zero : public twoparam_arithmetic_exception {
	twoparam_division_result_is_zero() : twoparam_arithmetic_exception("division yielded no significant bits") {}
};

// thrown when division yields NaR
struct twoparam_division_result_is_nar : public twoparam_arithmetic_exception {
	twoparam_division_result_is_nar() : twoparam_arithmetic_exception("division yielded nar") {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// INTERNAL OPERATION EXCEPTIONS

struct twoparam_internal_exception	: public universal_internal_exception {
	twoparam_internal_exception(const std::string& error) 
		: universal_internal_exception(std::string("twoparam internal exception: ") + error) {};
};

struct twoparam_hpos_too_large : public twoparam_internal_exception {
	twoparam_hpos_too_large() : twoparam_internal_exception("twoparamion of hidden bit too large for given twoparam") {}
};

struct twoparam_rbits_too_large : public twoparam_internal_exception {
	twoparam_rbits_too_large() :twoparam_internal_exception("number of remaining bits too large for this fraction") {}
};


}} // namespace sw::universal
