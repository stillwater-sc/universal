#pragma once
#include <universal/internal/blockbinary/manipulators.hpp>   // to_binary/to_hex on blockbinary (#1334)
#include <iostream>   // std::cout/cerr used below (#1334: include what you use)
#include <iomanip>
#include <string>
//  blocksignificand_test_suite.hpp : test suite for blocksignificand
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal { namespace internal {

template<typename InputType, typename ResultType, typename RefType>
void ReportBinaryArithmeticErrorBSCustom(const std::string& label, const std::string& op,
	const InputType& lhs, const InputType& rhs, const ResultType& result, const RefType& ref) {
	using namespace sw::universal;
	auto old_precision = std::cerr.precision();
	std::cerr << std::setprecision(20)
		<< label << '\n'
		<< std::setw(NUMBER_COLUMN_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << rhs
		<< " != "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result
		<< " golden reference is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
		<< '\n'
		<< std::setw(NUMBER_COLUMN_WIDTH) << to_binary(lhs)
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << to_binary(rhs)
		<< " != "
		<< std::setw(NUMBER_COLUMN_WIDTH) << to_binary(result)
		<< " golden reference is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << to_binary(ref)
		<< std::setprecision(old_precision)
		<< '\n';
}

// enumerate all addition cases for an blocksignificand<nbits,BlockType> configuration
template<typename blocksignificandConfiguration>
int VerifyBlockSignificandAddition(bool reportTestCases) {
	constexpr unsigned nbits = blocksignificandConfiguration::nbits;
	using BlockType = typename blocksignificandConfiguration::BlockType;

	constexpr unsigned NR_VALUES = (1u << nbits);
	using namespace sw::universal;

	//	cout << endl;
	//	cout << "blocksignificand<" << nbits << ',' << typeid(BlockType).name() << '>' << endl;

	int nrOfFailedTests = 0;

	blocksignificand<nbits, BlockType> a, b, c;
	blockbinary<nbits, BlockType> aref, bref, cref, refResult;
	constexpr size_t nrBlocks = blockbinary<nbits, BlockType>::nrBlocks;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		aref.setbits(i);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			bref.setbits(j);
			cref = aref + bref;
			c.add(a, b);
			for (size_t k = 0; k < nrBlocks; ++k) {
				refResult.setblock(k, c.block(k));
			}

			if (refResult != cref) {
				nrOfFailedTests++;
				if (reportTestCases)	ReportBinaryArithmeticErrorBSCustom("FAIL", "+", a, b, c, refResult);
			}
			else {
				// if (reportTestCases) ReportBinaryArithmeticSuccessBSCustom("PASS", "+", a, b, c, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		//		if (i % 1024 == 0) cout << '.'; /// if you enable this, also add the endl line back in
	}
	//	cout << endl;
	return nrOfFailedTests;
}

// enumerate all addition cases for an blocksignificand configuration
template<typename blocksignificandConfiguration>
int VerifyBlockSignificandSubtraction(bool reportTestCases) {
	constexpr unsigned nbits = blocksignificandConfiguration::nbits;
	using BlockType = typename blocksignificandConfiguration::BlockType;

	constexpr unsigned NR_VALUES = (1u << nbits);
	using namespace sw::universal;

	//	cout << endl;
	//	cout << "blocksignificand<" << nbits << ',' << typeid(BlockType).name() << '>' << endl;

	int nrOfFailedTests = 0;

	blocksignificand<nbits, BlockType> a, b, c;
	blockbinary<nbits, BlockType> aref, bref, cref, refResult;
	for (unsigned i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		aref.setbits(i);
		for (unsigned j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			bref.setbits(j);
			cref = aref - bref;
			c.sub(a, b);
			for (unsigned k = 0; k < blockbinary<nbits, BlockType>::nrBlocks; ++k) {
				refResult.setblock(k, c.block(k));
			}
			if (refResult != cref) {
				nrOfFailedTests++;
				if (reportTestCases)	ReportBinaryArithmeticErrorBSCustom("FAIL", "-", a, b, c, cref);
			}
			else {
				// if (reportTestCases) ReportBinaryArithmeticSuccessBSCustom("PASS", "-", a, b, c, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		//		if (i % 1024 == 0) cout << '.'; /// if you enable this, put the endl back
	}
	//	cout << endl;
	return nrOfFailedTests;
}


// enumerate all addition cases for an blocksignificand<nbits,BlockType> configuration
template<typename blocksignificandConfiguration>
int VerifyBlockSignificandMultiplication(bool reportTestCases) {
	constexpr unsigned nbits = blocksignificandConfiguration::nbits;
	using BlockType = typename blocksignificandConfiguration::BlockType;
	constexpr unsigned fhbits = (nbits >> 1);
	constexpr unsigned fbits = fhbits - 1;
	constexpr unsigned NR_VALUES = (size_t(1) << nbits);
	using namespace sw::universal;

	//	cout << endl;
	//	cout << "blocksignificand<" << nbits << ',' << typeid(BlockType).name() << '>' << endl;

	int nrOfFailedTests = 0;

	blocksignificandConfiguration a, b, c;
	a.setradix(fbits);
	b.setradix(fbits);
	c.setradix(2 * fbits);
	blockbinary<nbits, BlockType> aref, bref, cref, refResult;
	constexpr size_t nrBlocks = blockbinary<nbits, BlockType>::nrBlocks;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		aref.setbits(i);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			bref.setbits(j);
			cref = aref * bref;
			c.mul(a, b);
			for (size_t k = 0; k < nrBlocks; ++k) {
				refResult.setblock(k, c.block(k));
			}

			if (refResult != cref) {
				nrOfFailedTests++;
				if (reportTestCases)	ReportBinaryArithmeticErrorBSCustom("FAIL", "*", a, b, c, refResult);
			}
			else {
				// if (reportTestCases) ReportBinaryArithmeticSuccessBSCustom("PASS", "*", a, b, c, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		//		if (i % 1024 == 0) cout << '.'; /// if you enable this, put the endl back
	}
	//	cout << endl;
	return nrOfFailedTests;
}

// enumerate all division cases for an blocksignificand<nbits,BlockType> configuration
template<typename blocksignificandConfiguration>
int VerifyBlockSignificandDivision(bool reportTestCases) {
	// blocksignificand::div is a FIXED-POINT quotient: for operands A and B read at the
	// dividend's radix R, it is Q = floor(A * 2^R / B), with bit 0 set when the division was
	// inexact (a sticky bit for the rounding that follows), kept modulo 2^nbits, and zero when
	// B is zero. Every pair of nbits-bit operands is checked against that, computed exactly in
	// 64-bit integers (nbits <= 16 here, so A * 2^R < 2^32). It used to be compared with signed
	// integer division of the raw bits, a different operation, and failed every configuration
	// (#1745).
	constexpr unsigned nbits = blocksignificandConfiguration::nbits;
	using BlockType = typename blocksignificandConfiguration::BlockType;
	static_assert(nbits <= 16, "the exact reference holds A * 2^R in 64 bits");

	constexpr std::uint64_t NR_VALUES = (std::uint64_t(1) << nbits);
	constexpr std::uint64_t mask = NR_VALUES - 1u;
	using namespace sw::universal;

	int nrOfFailedTests = 0;
	// the radix of a blocktriple<DIV>'s significand: two integer bits above it
	constexpr unsigned radix = nbits - 2u;
	blocksignificand<nbits, BlockType> a, b, c;
	for (std::uint64_t i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		a.setradix(static_cast<int>(radix));
		for (std::uint64_t j = 0; j < NR_VALUES; ++j) {
			b.setbits(j);
			b.setradix(static_cast<int>(radix));
			c.div(a, b);
			std::uint64_t expected = 0;
			if (j != 0) {
				const std::uint64_t numerator = i << radix;
				const std::uint64_t q = numerator / j;
				expected = (q | ((numerator % j) != 0 ? 1u : 0u)) & mask;
			}
			std::uint64_t result = 0;
			for (unsigned k = 0; k < nbits; ++k) if (c.at(k)) result |= (std::uint64_t(1) << k);
			if (result != expected) {
				++nrOfFailedTests;
				if (reportTestCases && nrOfFailedTests < 10) {
					std::cerr << "FAIL: blocksignificand<" << nbits << "> " << i << " / " << j << " at radix " << radix
					          << " = " << result << ", expected " << expected << '\n';
				}
			}
		}
	}
	return nrOfFailedTests;
}

// TODO: formalism to test blocksignificand rounding
// enumerate all rounding cases for an blocksignificand<nbits,BlockType> configuration
template<typename blocksignificandConfiguration>
int VerifyRounding(bool reportTestCases) {
	constexpr size_t nbits = blocksignificandConfiguration::nbits;
	using BlockType = typename blocksignificandConfiguration::BlockType;

	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	using namespace sw::universal;

	//	std::cout << endl;
	//	std::cout << "blocksignificand<" << nbits << ',' << typeid(BlockType).name() << '>' << endl;

		// two's complement blocksignificands will have the form: 0ii.fffff
		// 
	int nrOfFailedTests = 0;

	blocksignificand<nbits, BlockType> a;
	//constexpr size_t nrBlocks = blockbinary<nbits, BlockType>::nrBlocks;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		a.setradix(5);
		// the LSB that we need to round can be anywhere in the fraction
		// let's pick one that has explicit bits to use for the rounding
		size_t targetLsb = 4;
		bool roundUp = a.roundingDirection(targetLsb);
		std::cout << to_binary(a) << " : round " << (roundUp ? "up " : "dn ") << '\n';
	}
	//	std::cout << endl;
	return nrOfFailedTests;
}

} } } // namespace sw::universal::internal
