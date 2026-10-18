#pragma once
// exceptions.hpp: exception hierarchy for exceptions during lindex arithmetic calculations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>        // std::string
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// BASE ARITHMETIC EXCEPTIONS

// base class for lindex arithmetic exceptions
struct lindex_arithmetic_exception : public universal_arithmetic_exception {
	lindex_arithmetic_exception(const std::string& error)
		: universal_arithmetic_exception(std::string("lindex arithmetic exception: ") + error) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// divide_by_zero is thrown when the denominator in a division operator is 0
struct lindex_divide_by_zero : public lindex_arithmetic_exception {
	lindex_divide_by_zero() : lindex_arithmetic_exception("divide by zero") {}
};

// operand_is_nar is thrown when an operand of a binary operator is NaR
struct lindex_operand_is_nar : public lindex_arithmetic_exception {
	lindex_operand_is_nar() : lindex_arithmetic_exception("operand is nar") {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// INTERNAL OPERATION EXCEPTIONS

struct lindex_internal_exception : public universal_internal_exception {
	lindex_internal_exception(const std::string& error)
		: universal_internal_exception(std::string("lindex internal exception: ") + error) {};
};

}} // namespace sw::universal
