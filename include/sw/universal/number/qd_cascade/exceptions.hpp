#pragma once
// exceptions.hpp: definition of qd_cascade exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

	// base class for qd_cascade arithmetic exceptions
	struct qd_cascade_arithmetic_exception : public universal_arithmetic_exception {
		qd_cascade_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("qd_cascade arithmetic exception: ") + err) {};
	};

	// divide by zero arithmetic exception for reals
	struct qd_cascade_divide_by_zero : public qd_cascade_arithmetic_exception {
		qd_cascade_divide_by_zero() : qd_cascade_arithmetic_exception("division by zero") {}
	};

	// negative argument to sqrt
	struct qd_cascade_negative_sqrt_arg : public qd_cascade_arithmetic_exception {
		qd_cascade_negative_sqrt_arg() : qd_cascade_arithmetic_exception("negative sqrt argument") {}
	};

	// negative argument to nroot
	struct qd_cascade_negative_nroot_arg : public qd_cascade_arithmetic_exception {
		qd_cascade_negative_nroot_arg() : qd_cascade_arithmetic_exception("negative nroot argument") {}
	};

	///////////////////////////////////////////////////////////////
	// internal implementation exceptions

	// base class for qd_cascade internal exceptions
	struct qd_cascade_internal_exception : public universal_internal_exception {
		qd_cascade_internal_exception(const std::string& err) : universal_internal_exception(std::string("qd_cascade internal exception: ") + err) {};
	};

	struct qd_cascade_word_index_out_of_bounds : public qd_cascade_internal_exception {
		qd_cascade_word_index_out_of_bounds() : qd_cascade_internal_exception("word index out of bounds") {}
	};

}} // namespace sw::universal
