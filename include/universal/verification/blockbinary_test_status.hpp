// blockbinary_test_status.cpp : functions to aid in testing and test reporting block binary numbers
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <universal/native/integers.hpp> // for to_binary(int)
#include <universal/internal/blockbinary/blockbinary.hpp>

namespace sw { namespace universal {

#define COLUMN_WIDTH 20
template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
void ReportBinaryArithmeticError(const std::string& test_case, const std::string& op, const blockbinary<nbits, BlockType, NumberType>& a, const blockbinary<nbits, BlockType, NumberType>& b, const blockbinary<nbits, BlockType, NumberType>& result, int64_t reference) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(COLUMN_WIDTH) << (long long)a    // to_hex(a, true)
		<< " " << op << " "
		<< std::setw(COLUMN_WIDTH) << (long long)b    // to_hex(b, true)
		<< " != "
		<< std::setw(COLUMN_WIDTH) << (long long)result // to_hex(result, true) 
		<< " golden reference is "
		<< std::setw(COLUMN_WIDTH) << reference << ' ' << to_binary(reference,false, nbits)
		<< " " << to_binary(result, true) << " vs " << to_binary(reference, false, nbits)
		<< std::setprecision(old_precision)
		<< std::endl;
}

template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
void ReportBinaryArithmeticSuccess(const std::string& test_case, const std::string& op, const blockbinary<nbits, BlockType, NumberType>& a, const blockbinary<nbits, BlockType, NumberType>& b, const blockbinary<nbits, BlockType, NumberType>& result, int64_t reference) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(COLUMN_WIDTH) << (long long)a    // to_hex(a, true)
		<< " " << op << " "
		<< std::setw(COLUMN_WIDTH) << (long long)b    // to_hex(b, true)
		<< " == "
		<< std::setw(COLUMN_WIDTH) << (long long)result // to_hex(result, true) 
		<< " matches reference "
		<< std::setw(COLUMN_WIDTH) << reference << ' ' << to_binary(reference, false, nbits)
		<< " " << to_binary(result, true) << " vs " << to_binary(reference, false, nbits)
		<< std::setprecision(old_precision)
		<< std::endl;
}

template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
void ReportArithmeticShiftError(const std::string& test_case, const std::string& op, const blockbinary<nbits, BlockType, NumberType>& a, const unsigned divider, const blockbinary<nbits, BlockType, NumberType>& result, int64_t reference) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(COLUMN_WIDTH) << (long long)a    // to_hex(a, true)
		<< " " << op << " "
		<< std::setw(COLUMN_WIDTH) << divider    // to_hex(b, true)
		<< " != "
		<< std::setw(COLUMN_WIDTH) << (long long)result // to_hex(result, true) 
		<< " golden reference is "
		<< std::setw(COLUMN_WIDTH) << reference << ' ' << to_binary(reference, false, nbits)
		<< " " << to_binary(result, true) << " vs " << to_binary(reference, false, nbits)
		<< std::setprecision(old_precision)
		<< std::endl;
}

template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
void ReportArithmeticShiftSuccess(const std::string& test_case, const std::string& op, const blockbinary<nbits, BlockType, NumberType>& a, const unsigned divider, const blockbinary<nbits, BlockType, NumberType>& result, int64_t reference) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(COLUMN_WIDTH) << (long long)a
		<< " " << op << " "
		<< std::setw(COLUMN_WIDTH) << divider    
		<< " == "
		<< std::setw(COLUMN_WIDTH) << (long long)result
		<< " matches reference   "
		<< std::setw(COLUMN_WIDTH) << reference << ' ' << to_binary(reference, false, nbits)
		<< " " << to_binary(result, true) << " vs " << to_binary(reference, false, nbits)
		<< std::setprecision(old_precision)
		<< std::endl;
}

}} // namespace sw::universal
