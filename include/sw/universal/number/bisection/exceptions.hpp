#pragma once
// exceptions.hpp: exception definitions for the bisection number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

struct bisection_arithmetic_exception : public universal_arithmetic_exception {
	bisection_arithmetic_exception(const std::string& err)
		: universal_arithmetic_exception(std::string("bisection arithmetic: ") + err) {}
};

struct bisection_divide_by_zero : public bisection_arithmetic_exception {
	bisection_divide_by_zero() : bisection_arithmetic_exception("division by zero") {}
};

struct bisection_negative_sqrt_arg : public bisection_arithmetic_exception {
	bisection_negative_sqrt_arg() : bisection_arithmetic_exception("negative sqrt argument") {}
};

struct bisection_internal_exception : public universal_internal_exception {
	bisection_internal_exception(const std::string& err)
		: universal_internal_exception(std::string("bisection internal: ") + err) {}
};

}} // namespace sw::universal
