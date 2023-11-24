#pragma once
// exceptions.hpp: definition of faithful number system exceptions
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
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

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// faithful internal operation exceptions

	struct faithful_internal_exception : public universal_internal_exception {
		faithful_internal_exception(const std::string& err) : universal_internal_exception(std::string("faithful internal exception: ") + err) {};
	};

}} // namespace sw::universal
