#pragma once
// exceptions.hpp: exception hierarchy for exceptions during posit calculations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// POSIT ARITHMETIC EXCEPTIONS

// base class for posit arithmetic exceptions
struct posito_arithmetic_exception : public universal_arithmetic_exception {			
	posito_arithmetic_exception(const std::string& error) 
		: universal_arithmetic_exception(std::string("posit arithmetic exception: ") + error) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// not_a_real is thrown when a rvar is NaR
struct posito_nar : public posito_arithmetic_exception {
	posito_nar() : posito_arithmetic_exception("nar (not a real)") {}
};

// divide_by_zero is thrown when the denominator in a division operator is 0
struct posito_divide_by_zero : public posito_arithmetic_exception {
	posito_divide_by_zero() : posito_arithmetic_exception("divide by zero") {}
};

// divide_by_nar is thrown when the denominator in a division operator is NaR
struct posito_divide_by_nar : public posito_arithmetic_exception {
	posito_divide_by_nar() : posito_arithmetic_exception("divide by nar") {}
};

// numerator_is_nar is thrown when the numerator in a division operator is NaR
struct posito_numerator_is_nar : public posito_arithmetic_exception {
	posito_numerator_is_nar() : posito_arithmetic_exception("numerator is nar") {}
};

// operand_is_nar is thrown when an rvar in a binary operator is NaR
struct posito_operand_is_nar : public posito_arithmetic_exception {
	posito_operand_is_nar() : posito_arithmetic_exception("operand is nar") {}
};

// thrown when division yields no signficant fraction bits
struct posito_division_result_is_zero : public posito_arithmetic_exception {
	posito_division_result_is_zero() : posito_arithmetic_exception("division yielded no significant bits") {}
};

// thrown when division yields NaR
struct posito_division_result_is_infinite : public posito_arithmetic_exception {
	posito_division_result_is_infinite() : posito_arithmetic_exception("division yielded infinite") {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// POSIT INTERNAL OPERATION EXCEPTIONS

struct posito_internal_exception	: public universal_internal_exception {
	posito_internal_exception(const std::string& error) 
		: universal_internal_exception(std::string("posit internal exception: ") + error) {};
};

struct posito_hpos_too_large : public posito_internal_exception {
	posito_hpos_too_large() : posito_internal_exception("position of hidden bit too large for given posit") {}
};

struct posito_rbits_too_large : public posito_internal_exception {
	posito_rbits_too_large() :posito_internal_exception("number of remaining bits too large for this fraction") {}
};

}} // namespace sw::universal
