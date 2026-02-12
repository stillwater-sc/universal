#pragma once
// exceptions.hpp: exception hierarchy for exceptions during posit calculations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifndef UNIVERSAL_NUMBER_POSIT_EXCEPTIONS_HPP
#define UNIVERSAL_NUMBER_POSIT_EXCEPTIONS_HPP
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// POSIT ARITHMETIC EXCEPTIONS

// base class for posit arithmetic exceptions
struct posit_arithmetic_exception : public universal_arithmetic_exception {			
	posit_arithmetic_exception(const std::string& error) 
		: universal_arithmetic_exception(std::string("posit arithmetic exception: ") + error) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// not_a_real is thrown when a rvar is NaR
struct posit_nar : public posit_arithmetic_exception {
	posit_nar() : posit_arithmetic_exception("nar (not a real)") {}
};

// divide_by_zero is thrown when the denominator in a division operator is 0
struct posit_divide_by_zero : public posit_arithmetic_exception {
	posit_divide_by_zero() : posit_arithmetic_exception("divide by zero") {}
};

// divide_by_nar is thrown when the denominator in a division operator is NaR
struct posit_divide_by_nar : public posit_arithmetic_exception {
	posit_divide_by_nar() : posit_arithmetic_exception("divide by nar") {}
};

// numerator_is_nar is thrown when the numerator in a division operator is NaR
struct posit_numerator_is_nar : public posit_arithmetic_exception {
	posit_numerator_is_nar() : posit_arithmetic_exception("numerator is nar") {}
};

// operand_is_nar is thrown when an rvar in a binary operator is NaR
struct posit_operand_is_nar : public posit_arithmetic_exception {
	posit_operand_is_nar() : posit_arithmetic_exception("operand is nar") {}
};

// thrown when division yields no signficant fraction bits
struct posit_division_result_is_zero : public posit_arithmetic_exception {
	posit_division_result_is_zero() : posit_arithmetic_exception("division yielded no significant bits") {}
};

// thrown when division yields NaR
struct posit_division_result_is_infinite : public posit_arithmetic_exception {
	posit_division_result_is_infinite() : posit_arithmetic_exception("division yielded infinite") {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// POSIT INTERNAL OPERATION EXCEPTIONS

struct posit_internal_exception	: public universal_internal_exception {
	posit_internal_exception(const std::string& error) 
		: universal_internal_exception(std::string("posit internal exception: ") + error) {};
};

struct posit_hpos_too_large : public posit_internal_exception {
	posit_hpos_too_large() : posit_internal_exception("position of hidden bit too large for given posit") {}
};

struct posit_rbits_too_large : public posit_internal_exception {
	posit_rbits_too_large() :posit_internal_exception("number of remaining bits too large for this fraction") {}
};

}} // namespace sw::universal

#endif // UNIVERSAL_NUMBER_POSIT_EXCEPTIONS_HPP
