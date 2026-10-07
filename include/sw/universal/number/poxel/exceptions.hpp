#pragma once
// exceptions.hpp: exception hierarchy for exceptions during poxel arithmetic calculations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

struct poxel_arithmetic_exception : public universal_arithmetic_exception {
	poxel_arithmetic_exception(const std::string& error)
		: universal_arithmetic_exception(std::string("poxel arithmetic exception: ") + error) {};
};
struct poxel_operand_is_nar : public poxel_arithmetic_exception {
	poxel_operand_is_nar() : poxel_arithmetic_exception("operand is nar") {}
};
struct poxel_divide_by_zero : public poxel_arithmetic_exception {
	poxel_divide_by_zero() : poxel_arithmetic_exception("divide by zero") {}
};
struct poxel_divide_by_nar : public poxel_arithmetic_exception {
	poxel_divide_by_nar() : poxel_arithmetic_exception("divide by nar") {}
};
struct poxel_numerator_is_nar : public poxel_arithmetic_exception {
	poxel_numerator_is_nar() : poxel_arithmetic_exception("numerator is nar") {}
};

struct poxel_internal_exception : public universal_internal_exception {
	poxel_internal_exception(const std::string& error)
		: universal_internal_exception(std::string("poxel internal exception: ") + error) {};
};

}} // namespace sw::universal
