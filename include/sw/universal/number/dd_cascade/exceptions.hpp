#pragma once
// exceptions.hpp: definition of dd_cascade exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

	// base class for dd_cascade arithmetic exceptions
	struct dd_cascade_arithmetic_exception : public universal_arithmetic_exception {
		dd_cascade_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("dd_cascade arithmetic exception: ") + err) {};
	};

	// divide by zero arithmetic exception for reals
	struct dd_cascade_divide_by_zero : public dd_cascade_arithmetic_exception {
		dd_cascade_divide_by_zero() : dd_cascade_arithmetic_exception("division by zero") {}
	};

	// negative argument to sqrt
	struct dd_cascade_negative_sqrt_arg : public dd_cascade_arithmetic_exception {
		dd_cascade_negative_sqrt_arg() : dd_cascade_arithmetic_exception("negative sqrt argument") {}
	};

	// negative argument to nroot
	struct dd_cascade_negative_nroot_arg : public dd_cascade_arithmetic_exception {
		dd_cascade_negative_nroot_arg() : dd_cascade_arithmetic_exception("negative nroot argument") {}
	};

	///////////////////////////////////////////////////////////////
	// internal implementation exceptions

	// base class for dd_cascade internal exceptions
	struct dd_cascade_internal_exception : public universal_internal_exception {
		dd_cascade_internal_exception(const std::string& err) : universal_internal_exception(std::string("dd_cascade internal exception: ") + err) {};
	};

	struct dd_cascade_word_index_out_of_bounds : public dd_cascade_internal_exception {
		dd_cascade_word_index_out_of_bounds() : dd_cascade_internal_exception("word index out of bounds") {}
	};

}} // namespace sw::universal
