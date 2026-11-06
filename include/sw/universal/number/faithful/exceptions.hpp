#pragma once
// exceptions.hpp: definition of faithful number system exceptions
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <stdexcept>     // std::runtime_error
#include <string>        // std::string
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

	// base class for faithful arithmetic exceptions
	struct faithful_arithmetic_exception : public universal_arithmetic_exception {
		faithful_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("faithful arithmetic exception: ") + err) {};
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////
	///	specialized exceptions to aid application level exception handling

	// faithful_not_a_number is thrown when a rvar is NaN
	struct faithful_not_a_number : public faithful_arithmetic_exception {
		faithful_not_a_number() : faithful_arithmetic_exception("not a number") {}
	};

	// divide by zero arithmetic exception for faithfuls
	struct faithful_divide_by_zero : public std::runtime_error {
		faithful_divide_by_zero() : std::runtime_error("division by zero") {}
	};

	// operand_is_nan is thrown when an rvar in a binary operator is NaN
	struct faithful_operand_is_nan : public faithful_arithmetic_exception {
		faithful_operand_is_nan() : faithful_arithmetic_exception("operand is nan") {}
	};

	// negative argument to sqrt
	struct faithful_negative_sqrt_arg : public faithful_arithmetic_exception {
		faithful_negative_sqrt_arg() : faithful_arithmetic_exception("negative sqrt argument") {}
	};

	// an argument outside a function's domain: sin(inf), asin(2), acosh(0.5), atanh(2), pow(-2, 0.5)
	struct faithful_domain_error : public faithful_arithmetic_exception {
		faithful_domain_error(const std::string& function) : faithful_arithmetic_exception(function + ": argument outside the domain") {}
	};

	// negative argument to log, log2, log10, or an argument below -1 to log1p
	struct faithful_negative_log_arg : public faithful_arithmetic_exception {
		faithful_negative_log_arg() : faithful_arithmetic_exception("negative log argument") {}
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// faithful internal operation exceptions

	struct faithful_internal_exception : public universal_internal_exception {
		faithful_internal_exception(const std::string& err) : universal_internal_exception(std::string("faithful internal exception: ") + err) {};
	};

}} // namespace sw::universal
