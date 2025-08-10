#pragma once
// exceptions.hpp: exceptions for problems in BLAS calculations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <stdexcept>
#include <sstream>
#include <string>

namespace sw { namespace blas {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// BLAS FUNCTION EXCEPTIONS

	// base class for BLAS exceptions
	struct blas_exception
		: public std::runtime_error
	{
		blas_exception(const std::string& error) 
			: std::runtime_error(std::string("BLAS exception: ") + error) {};
	};

	struct incompatible_matrices {
		incompatible_matrices(size_t arows, size_t acols, size_t brows, size_t bcols, const std::string& op) {
			std::stringstream ss;
			ss << "LHS[ " << arows << " x " << acols << " ] and RHS[ " << brows << " x " << bcols << " ] incompatible for operator '" << op << "'";
			message = ss.str();
		}
		std::string message;
		std::string what() { return message; }
	};

	// base class for matmul exceptions
	struct matmul_incompatible_matrices
		: public std::runtime_error
	{
		matmul_incompatible_matrices(const std::string& error)
			: std::runtime_error(std::string("BLAS matmul operator: ") + error) {
		};
	};

}} // namespace sw::blas
