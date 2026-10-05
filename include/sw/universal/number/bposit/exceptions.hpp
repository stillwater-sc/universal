#pragma once
// exceptions.hpp: exception hierarchy for exceptions during bposit arithmetic calculations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for bposit arithmetic exceptions
struct bposit_arithmetic_exception : public universal_arithmetic_exception {
	bposit_arithmetic_exception(const std::string& error)
		: universal_arithmetic_exception(std::string("bposit arithmetic exception: ") + error) {};
};

// an operand of a binary operator is NaR
struct bposit_operand_is_nar : public bposit_arithmetic_exception {
	bposit_operand_is_nar() : bposit_arithmetic_exception("operand is nar") {}
};
// the denominator of a division is 0
struct bposit_divide_by_zero : public bposit_arithmetic_exception {
	bposit_divide_by_zero() : bposit_arithmetic_exception("divide by zero") {}
};
// the denominator of a division is NaR
struct bposit_divide_by_nar : public bposit_arithmetic_exception {
	bposit_divide_by_nar() : bposit_arithmetic_exception("divide by nar") {}
};
// the numerator of a division is NaR
struct bposit_numerator_is_nar : public bposit_arithmetic_exception {
	bposit_numerator_is_nar() : bposit_arithmetic_exception("numerator is nar") {}
};

// base class for bposit internal exceptions
struct bposit_internal_exception : public universal_internal_exception {
	bposit_internal_exception(const std::string& error)
		: universal_internal_exception(std::string("bposit internal exception: ") + error) {};
};

}} // namespace sw::universal
