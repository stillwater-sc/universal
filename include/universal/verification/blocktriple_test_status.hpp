// blocktriple_test_status.cpp : functions to aid in testing and test reporting block binary numbers
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <universal/internal/blocktriple/blocktriple.hpp>

namespace sw { namespace universal {

#define COLUMN_WIDTH 20
template<size_t nbits, typename bt = uint8_t>
void ReportBinaryArithmeticError(const std::string& test_case, const std::string& op, const blocktriple<nbits, bt>& a, const blocktriple<nbits, bt>& b, const blocktriple<nbits, bt>& result, int64_t reference) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(COLUMN_WIDTH) << a    // to_triple(a, true)
		<< " " << op << " "
		<< std::setw(COLUMN_WIDTH) << b    // to_triple(b, true)
		<< " != "
		<< std::setw(COLUMN_WIDTH) << result // to_triple(result, true) 
		<< " golden reference is "
		<< std::setw(COLUMN_WIDTH) << reference << ' ' << to_binary(reference,nbits)
		<< " " << to_binary(result, true) << " vs " << to_binary(reference, nbits)
		<< std::setprecision(old_precision)
		<< std::endl;
}

template<size_t nbits, typename bt = uint8_t>
void ReportBinaryArithmeticSuccess(const std::string& test_case, const std::string& op, const blocktriple<nbits, bt>& a, const blocktriple<nbits, bt>& b, const blocktriple<nbits, bt>& result, int64_t reference) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(COLUMN_WIDTH) << a    // to_triple(a, true)
		<< " " << op << " "
		<< std::setw(COLUMN_WIDTH) << b    // to_triple(b, true)
		<< " == "
		<< std::setw(COLUMN_WIDTH) << result // to_triple(result, true) 
		<< " matches reference "
		<< std::setw(COLUMN_WIDTH) << reference << ' ' << to_binary(reference, nbits)
		<< " " << to_binary(result, true) << " vs " << to_binary(reference, nbits)
		<< std::setprecision(old_precision)
		<< std::endl;
}

template<size_t nbits, typename bt = uint8_t>
void ReportArithmeticShiftError(const std::string& test_case, const std::string& op, const blocktriple<nbits, bt>& a, const size_t divider, const blocktriple<nbits, bt>& result, int64_t reference) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(COLUMN_WIDTH) << a    // to_triple(a, true)
		<< " " << op << " "
		<< std::setw(COLUMN_WIDTH) << divider    // to_triple(b, true)
		<< " != "
		<< std::setw(COLUMN_WIDTH) << result // to_triple(result, true) 
		<< " golden reference is "
		<< std::setw(COLUMN_WIDTH) << reference << ' ' << to_binary(reference, nbits)
		<< " " << to_binary(result, true) << " vs " << to_binary(reference, nbits)
		<< std::setprecision(old_precision)
		<< std::endl;
}

template<size_t nbits, typename bt = uint8_t>
void ReportArithmeticShiftSuccess(const std::string& test_case, const std::string& op, const blocktriple<nbits, bt>& a, const size_t divider, const blocktriple<nbits, bt>& result, int64_t reference) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(COLUMN_WIDTH) << a
		<< " " << op << " "
		<< std::setw(COLUMN_WIDTH) << divider    
		<< " == "
		<< std::setw(COLUMN_WIDTH) << result
		<< " matches reference   "
		<< std::setw(COLUMN_WIDTH) << reference << ' ' << to_binary(reference, nbits)
		<< " " << to_binary(result, true) << " vs " << to_binary(reference, nbits)
		<< std::setprecision(old_precision)
		<< std::endl;
}

}} // namespace sw::universal
