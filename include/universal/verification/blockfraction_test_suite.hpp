#pragma once
//  blockfraction_test_suite.hpp : test suite for blockfractionignificant
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal { namespace internal {

template<typename InputType, typename ResultType, typename RefType>
void ReportBinaryArithmeticErrorBFCustom(const std::string& label, const std::string& op,
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


// enumerate all addition cases for an blockfraction<nbits,BlockType> configuration
template<typename blockfractionConfiguration>
int VerifyBlockFractionMultiplication(bool reportTestCases) {
	constexpr unsigned nbits = blockfractionConfiguration::nbits;
	using BlockType = typename blockfractionConfiguration::BlockType;
	constexpr unsigned fhbits = (nbits >> 1);
	constexpr unsigned fbits = fhbits - 1;
	constexpr unsigned NR_VALUES = (size_t(1) << nbits);
	using namespace sw::universal;

	//	cout << endl;
	//	cout << "blockfraction<" << nbits << ',' << typeid(BlockType).name() << '>' << endl;

	int nrOfFailedTests = 0;

	blockfractionConfiguration a, b, c;
	a.setradix(fbits);
	b.setradix(fbits);
	a.setradix(2 * fbits);
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
				if (reportTestCases)	ReportBinaryArithmeticErrorBFCustom("FAIL", "*", a, b, c, refResult);
			}
			else {
				// if (reportTestCases) ReportBinaryArithmeticSuccessBFCustom("PASS", "*", a, b, c, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		//		if (i % 1024 == 0) cout << '.'; /// if you enable this, put the endl back
	}
	//	cout << endl;
	return nrOfFailedTests;
}

} } } // namespace sw::universal::internal
