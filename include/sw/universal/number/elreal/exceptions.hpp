#pragma once
// exceptions.hpp: exception hierarchy for the elreal (Exact Lazy Real) number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for elreal arithmetic exceptions
struct elreal_arithmetic_exception : public universal_arithmetic_exception {
	elreal_arithmetic_exception(const std::string& err)
		: universal_arithmetic_exception(std::string("elreal arithmetic exception: ") + err) {}
};

///////////////////////////////////////////////////////////////////////////////////////
/// specialized arithmetic exceptions

// thrown when a NaN operand is encountered
struct elreal_not_a_number : public elreal_arithmetic_exception {
	elreal_not_a_number() : elreal_arithmetic_exception("not a number") {}
};

// thrown when a division by zero is attempted
struct elreal_divide_by_zero : public elreal_arithmetic_exception {
	elreal_divide_by_zero() : elreal_arithmetic_exception("divide by zero") {}
};

// thrown when the denominator in a division is NaN
struct elreal_divide_by_nan : public elreal_arithmetic_exception {
	elreal_divide_by_nan() : elreal_arithmetic_exception("divide by nan") {}
};

// thrown when sqrt is applied to a negative argument
struct elreal_negative_sqrt_arg : public elreal_arithmetic_exception {
	elreal_negative_sqrt_arg() : elreal_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////////////////////////////
/// lazy-real-specific exceptions

// Comparisons of lazy reals can be undecidable in general (the "is x == y?" problem).
// When a refinement budget is exhausted before the comparison resolves, callers may
// see this exception (controlled by the ELREAL_THROW_ARITHMETIC_EXCEPTION guard and
// the per-call budget policy that lands in Phase D).
struct elreal_undecidable_comparison : public elreal_arithmetic_exception {
	elreal_undecidable_comparison()
		: elreal_arithmetic_exception("comparison undecidable within refinement budget") {}
};

// Thrown when any operation's per-call refinement budget is exhausted before
// producing a definite answer.
struct elreal_budget_exhausted : public elreal_arithmetic_exception {
	elreal_budget_exhausted()
		: elreal_arithmetic_exception("refinement budget exhausted") {}
};

///////////////////////////////////////////////////////////////////////////////////////
/// REAL INTERNAL OPERATION EXCEPTIONS

struct elreal_internal_exception : public universal_internal_exception {
	elreal_internal_exception(const std::string& err)
		: universal_internal_exception(std::string("elreal internal exception: ") + err) {}
};

}} // namespace sw::universal
