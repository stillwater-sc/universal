//  blockbinary_helpers.cpp : functions to aid in testing and test reporting block binary numbers
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/native/integers.hpp> // for to_binary(int)
#include <universal/native/blockbinary.hpp>

#define COLUMN_WIDTH 20
template<size_t nbits, typename Ty = uint8_t>
void ReportBinaryArithmeticError(std::string test_case, std::string op, const sw::unum::blockbinary<nbits, Ty>& a, const sw::unum::blockbinary<nbits, Ty>& b, const sw::unum::blockbinary<nbits, Ty>& result, int64_t reference) {
	using namespace sw::unum;
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(COLUMN_WIDTH) << to_hex(a, true)
		<< " " << op << " "
		<< std::setw(COLUMN_WIDTH) << to_hex(b, true)
		<< " != "
		<< std::setw(COLUMN_WIDTH) << to_hex(result, true) << " golden reference is "
		<< std::setw(COLUMN_WIDTH) << reference
		<< " " << to_binary(result, true) << " vs " << to_binary(reference, nbits)
		<< std::setprecision(old_precision)
		<< std::endl;
}

template<size_t nbits, typename Ty = uint8_t>
void ReportBinaryArithmeticSuccess(std::string test_case, std::string op, const sw::unum::blockbinary<nbits, Ty>& a, const sw::unum::blockbinary<nbits, Ty>& b, const sw::unum::blockbinary<nbits, Ty>& result, int64_t reference) {
	using namespace sw::unum;
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(COLUMN_WIDTH) << to_hex(a)
		<< " " << op << " "
		<< std::setw(COLUMN_WIDTH) << to_hex(b)
		<< " == "
		<< std::setw(COLUMN_WIDTH) << to_hex(result) << " matches reference "
		<< std::setw(COLUMN_WIDTH) << reference
		<< " " << to_binary(result, true) << " vs " << to_binary(reference, nbits)
		<< std::setprecision(old_precision)
		<< std::endl;
}


