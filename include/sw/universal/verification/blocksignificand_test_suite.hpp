#pragma once
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
// TODO: fix test failures in VerifyblocksignificandDivision<blocksignificandConfiguration>
template<typename blocksignificandConfiguration>
int VerifyBlockSignificandDivision(bool reportTestCases) {
	constexpr unsigned nbits = blocksignificandConfiguration::nbits;
	using BlockType = typename blocksignificandConfiguration::BlockType;

	constexpr unsigned NR_VALUES = (1u << nbits);
	using namespace sw::universal;

	//	cout << endl;
	//	cout << "blocksignificand<" << nbits << ',' << typeid(BlockType).name() << '>' << endl;

	int nrOfFailedTests = 0;

	blocksignificand<nbits, BlockType> a, b, c;
	// nbits = 2 * fhbits
	constexpr unsigned fhbits = (nbits >> 1);
	constexpr unsigned fbits = fhbits - 1;
	a.setradix(2 * fbits);
	b.setradix(2 * fbits);
	c.setradix(2 * fbits);
	blockbinary<nbits, BlockType> aref, bref, cref, refResult;
	constexpr unsigned nrBlocks = blockbinary<nbits, BlockType>::nrBlocks;
	for (unsigned i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		aref.setbits(i);
		for (unsigned j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			bref.setbits(j);
			cref = aref / bref;
			c.div(a, b);
			for (unsigned k = 0; k < nrBlocks; ++k) {
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
		//		if (i % 1024 == 0) cout << '.'; /// if you enable this, put the endl back
	}
	//	cout << endl;
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
