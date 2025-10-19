#pragma once
// exceptions.hpp: exception hierarchy for triple-double cascade (td_cascade) exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// ARITHMETIC EXCEPTIONS

// base class for td_cascade arithmetic exceptions
struct td_cascade_arithmetic_exception : public universal_arithmetic_exception {
	explicit td_cascade_arithmetic_exception(const std::string& error) : universal_arithmetic_exception(std::string("td_cascade arithmetic exception: ") + error) {}
};

// divide by zero exception for td_cascade
struct td_cascade_divide_by_zero : public td_cascade_arithmetic_exception {
	td_cascade_divide_by_zero() : td_cascade_arithmetic_exception("division by zero") {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// INTERNAL IMPLEMENTATION EXCEPTIONS

// base class for td_cascade internal exceptions
struct td_cascade_internal_exception : public universal_internal_exception {
	explicit td_cascade_internal_exception(const std::string& error) : universal_internal_exception(std::string("td_cascade internal exception: ") + error) {}
};

struct td_cascade_index_out_of_bounds : public td_cascade_internal_exception {
	td_cascade_index_out_of_bounds() : td_cascade_internal_exception("index out of bounds") {}
};

}} // namespace sw::universal
