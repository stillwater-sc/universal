#pragma once
// quire_test_suite.cpp : test suite to verify quire arithmetic
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>

namespace sw { namespace universal {

int TestQuireAccumulationResult(int nrOfFailedTests, const std::string& descriptor)
{
	if (nrOfFailedTests > 0) {
		std::cout << descriptor << " quire accumulation FAIL" << std::endl;
	}
	else {
		std::cout << descriptor << " quire accumulation PASS" << std::endl;
	}
	return nrOfFailedTests;
}

static constexpr unsigned QUIRE_TABLE_WIDTH = 15;

template<unsigned nbits, unsigned es>
void ReportQuireNonZeroError(const std::string& test_result, const std::string& op, unsigned nrOfElements, const posit<nbits, es>& seed, const posit<nbits, es>& presult) {
	std::cerr << test_result << " "
		<< std::setprecision(20)
		<< " " << op
		<< " vector size " << nrOfElements
		<< " seed " << seed << " "
		<< " != "
		<< std::setw(QUIRE_TABLE_WIDTH) << 0 << " instead it yielded "
		<< std::setw(QUIRE_TABLE_WIDTH) << presult
		<< " " << 0 << " vs " << presult.get()
		<< std::setprecision(5)
		<< std::endl;
}

template<unsigned nbits, unsigned es>
void ReportQuireNonZeroSuccess(const std::string& test_result, const std::string& op, unsigned nrOfElements, const posit<nbits, es>& seed, const posit<nbits, es>& presult) {
	std::cerr << test_result
		<< std::setprecision(20)
		<< " " << op
		<< " vector size " << nrOfElements
		<< " seed " << seed << " "
		<< std::setw(QUIRE_TABLE_WIDTH) << presult
		<< " " << presult.get()
		<< std::setprecision(5)
		<< std::endl;
}

// quire value conversion tests

template<unsigned nbits, unsigned es, unsigned capacity>
void GenerateUnsignedIntAssignments() {
	quire<nbits, es, capacity> q;
	unsigned upper_range = q.upper_range();
	std::cout << "Upper range = " << upper_range << std::endl;
	uint64_t i;
	q = 0; std::cout << q << std::endl;
	for (i = 1; i < uint64_t(1) << (upper_range + capacity); i <<= 1) {
		q = i;
		std::cout << q << std::endl;
	}
	i <<= 1;
	try {
		q = i;
	}
	catch (const std::runtime_error& err) {
		std::cerr << "Caught the exception: " << err.what() << ". Value was " << i << std::endl;
	}
}

template<unsigned nbits, unsigned es, unsigned capacity>
void GenerateSignedIntAssignments() {
	quire<nbits, es, capacity> q;
	unsigned upper_range = q.upper_range();
	std::cout << "Upper range = " << upper_range << std::endl;
	int64_t i, upper_limit = -(int64_t(1) << (upper_range + capacity));
	q = 0; std::cout << q << std::endl;
	for (i = -1; i > upper_limit; i *= 2) {
		q = i;
		std::cout << q << std::endl;
	}
	i <<= 1;
	try {
		q = i;
	}
	catch (const std::runtime_error& err) {
		std::cerr << "Caught the exception: " << err.what() << ". RHS was " << i << std::endl;
	}
}

template<unsigned nbits, unsigned es, unsigned capacity, unsigned fbits = 1>
void GenerateValueAssignments() {
	quire<nbits, es, capacity> q;

	// report some parameters about the posit and quire configuration
	unsigned max_scale = q.max_scale();
	unsigned min_scale = q.min_scale();
	std::cout << "Maximum scale  = " << max_scale << " Minimum scale  = " << min_scale << " Dynamic range = " << q.dynamic_range() << std::endl;
	std::cout << "Maxpos Squared = " << maxpos_scale<nbits, es>() * 2 << " Minpos Squared = " << minpos_scale<nbits, es>() * 2 << std::endl;

	// cover the scales with one order outside of the dynamic range of the quire configuration (minpos^2 and maxpos^2)
	for (int scale = int(max_scale + 1); scale >= int(min_scale - 1); scale--) {  // extend by 1 max and min scale to test edge of the quire
		internal::value<fbits> v = std::pow(2.0, scale);
		try {
			q = v;
			std::cout << std::setw(10) << v << q << std::endl;
			internal::value<q.qbits> r = q.to_value();
			double in = double(v);
			double out = (double)r;
			if (std::abs(in - out) > 0.0000001) {
				std::cerr << "quire value conversion failed: " << to_triple(v) << " != " << to_triple(r) << std::endl;
			}
		}
		catch (const quire_exception& err) {
			std::cerr << "Caught the exception: " << err.what() << ". RHS was " << v << " " << to_triple(v) << std::endl;
		}
		catch (...) {
			std::cerr << "Why can't I catch quire_exception type?\n";
		}
	}
}

// Depends on quire assignment to be correct
template<unsigned nbits, unsigned es, unsigned capacity>
int GenerateRegimePatternsForQuireAccumulation(bool bReportIndividualTestCases) {
	int nrOfFailedTests = 0;

	posit<nbits, es> pa, pb, ponemme, ponepme, pmul;

	// generate the minpos to maxpos pattern set
	// example for a posit with nbits = 10
	// minpos = 00_0000_0001
	//        = 00_0000_0011
	//        = 00_0000_0111
	//        = 00_0000_1111
	//        = 00_0001_1111
	//        = 00_0011_1111
	//        = 00_0111_1111
	//        = 00_1111_1111
	//   1.0  = 01_0000_0000
	//        = 01_1000_0000
	//        = 01_1100_0000
	//        = 01_1110_0000
	//        = 01_1111_0000
	//        = 01_1111_1000
	//        = 01_1111_1100
	//        = 01_1111_1110
	// maxpos = 01_1111_1111
	int nrOfPatterns = int(nbits) - 2;
	std::vector<uint64_t> patterns(2 * nrOfPatterns + 1);
	for (int i = 0; i < nrOfPatterns; i++) {
		patterns[i] = ((uint64_t(1) << i) - 1);
		//std::cout << patterns[i] << std::endl;
	}
	uint64_t msb = nbits - 1;
	uint64_t pattern = ((uint64_t(1) << nbits) - 1) & ~(uint64_t(1) << msb);
	uint64_t mask = -1;
	for (int i = 2 * nrOfPatterns; i > nrOfPatterns; i--) {
		patterns[i] = pattern & mask;
		//std::cout << patterns[i] << std::endl;
		mask <<= 1;
	}
	patterns[nrOfPatterns] = pattern & mask; // 1.0

#ifdef CONFIRM_PATTERNS
	std::cout << hex;
	for (std::vector<uint64_t>::const_iterator it = patterns.begin(); it != patterns.end(); it++) {
		std::cout << std::setw(3) << right << *it << std::endl;
	}
	std::cout << dec;
#endif

	quire<nbits, es, capacity> q0;
	quire<nbits, es, capacity> q;

	// accumulate a progressively larger product result
	// starting from minpos^2
	for (unsigned i = 0; i < nbits; i++) {
		pattern = patterns[i];
		//std::cout << "posit pattern: " << pattern << std::endl;
		pa.set_raw_bits(pattern);
		pattern >>= 1;

		ponemme = 1.0f; ponemme--;
		ponepme = 1.0f; ponepme++;
		pb = ponemme;
		pmul = pa * pb;


		q += quire_mul(pa, pb);
		//std::cout << q << std::endl;

		// convert quire to posit
		posit<nbits, es> presult;
		presult.convert(q.to_value());

		if (pmul != presult) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "*", pa, pb, pmul, presult);
		}
		else {
			//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*", pa, pb, pmul, presult);
#if HARDWARE_QA_OUTPUT
			std::cout << to_hex(q0.get()) << " " << to_hex(pa.get()) << " " << to_hex(pb.get()) << " " << to_hex(q.get()) << " " << to_hex(presult.get()) << std::endl;
#endif
		}
	}

	return nrOfFailedTests;;
}

template<unsigned nbits, unsigned es>
std::vector< posit<nbits, es> > GenerateVectorForZeroValueFDP(unsigned nrOfElements, const posit<nbits, es>& seed) {
	std::vector< posit<nbits, es> > t(nrOfElements);
	// first half of the vector is positive seed, second half is negative seed
	if (nrOfElements % 2) nrOfElements++; // vector size has to be even to yield zero
	unsigned half = nrOfElements >> 1;
	for (unsigned i = 0; i < half; i++) {
		t[i] = seed;
		t[i + half] = -seed;
	}
	return t;
}



// use a well-defined set of vectors with a known fused dot-product result
// the biggest stress are vectors where the first half is accumulating and the second half is subtracting
template<unsigned nbits, unsigned es, unsigned capacity>
int ValidateQuireAccumulation(bool bReportIndividualTestCases, const std::vector< posit<nbits, es> >& t) {
	int nrOfFailedTests = 0;

	posit<nbits, es> pa, pb, ponemme, ponepme, pmul;
	quire<nbits, es, capacity> q;

	// accumulate a progressively larger product result
	// starting from minpos^2
	for (unsigned i = 0; i < t.size(); i++) {
		pa = t[i];
		//std::cout << "posit pattern: " << pattern << std::endl;

		ponemme = 1.0f; ponemme--;
		ponepme = 1.0f; ponepme++;
		pb = ponemme;
		pmul = pa * pb;

		q += quire_mul(pa, pb);
		//std::cout << q << std::endl;
	}

	// convert quire to posit
	posit<nbits, es> presult;
	convert(q.to_value(), presult);

	if (!presult.iszero()) {
		nrOfFailedTests++;
		if (bReportIndividualTestCases)	ReportQuireNonZeroError("FAIL", "fdp", unsigned(t.size()), t[0], presult);
	}
	else {
		if (bReportIndividualTestCases) ReportQuireNonZeroSuccess("PASS", "fdp", unsigned(t.size()), t[0], presult);
		//std::cout << to_hex(q0.get()) << " " << to_hex(pa.get()) << " " << to_hex(pb.get()) << " " << to_hex(q.get()) << " " << to_hex(presult.get()) << std::endl;
	}

	return nrOfFailedTests;;
}


}} // namespace sw::universal

