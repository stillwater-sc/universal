#pragma once
// binaryop_status.cpp : error and success reporting of a binary arithmetic operation
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>

namespace sw { namespace universal {

#define COLUMN_WIDTH 20
template<typename Ty>
void ReportBinaryArithmeticError(const std::string& test_case, const std::string& op, const Ty& a, const Ty& b, const Ty& result, long double reference) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(COLUMN_WIDTH) << a    // to_hex(a, true)
		<< " " << op << " "
		<< std::setw(COLUMN_WIDTH) << b    // to_hex(b, true)
		<< " != "
		<< std::setw(COLUMN_WIDTH) << result // to_hex(result, true) 
		<< " golden reference is "
		<< std::setw(COLUMN_WIDTH) << reference
//		<< " " << to_binary(result, true) << " vs " << to_binary(reference, nbits)
		<< std::setprecision(old_precision)
		<< std::endl;
}

template<typename Ty>
void ReportBinaryArithmeticSuccess(const std::string& test_case, const std::string& op, const Ty& a, const Ty& b, const Ty& result, long double reference) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(COLUMN_WIDTH) << (long long)a    // to_hex(a, true)
		<< " " << op << " "
		<< std::setw(COLUMN_WIDTH) << (long long)b    // to_hex(b, true)
		<< " == "
		<< std::setw(COLUMN_WIDTH) << (long long)result // to_hex(result, true) 
		<< " matches reference "
		<< std::setw(COLUMN_WIDTH) << reference
//		<< " " << to_binary(result, true) << " vs " << to_binary(reference, nbits)
		<< std::setprecision(old_precision)
		<< std::endl;
}

}} // namespace sw::universal
