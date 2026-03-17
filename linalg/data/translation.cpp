// translation.cpp: test suite for data file translation between type sets
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <cmath>
#include <universal/number_systems.hpp>
#include <numeric/containers.hpp>
#include <blas/serialization/datafile.hpp>
#include <blas/serialization/typed_datafile.hpp>
#include <blas/serialization/translator.hpp>
#include <universal/verification/test_suite.hpp>

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#define REGRESSION_LEVEL_1 1
#endif

int main()
try {
	using namespace sw::universal;
	using namespace sw::numeric::containers;
	using namespace sw::blas;

	std::string test_suite = "data file translation tests";
	std::string test_tag = "translator";
	bool reportTestCases = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		// translate float data to double
		using SrcTypes = type_list<float>;
		using DstTypes = type_list<double>;

		// create source data
		vector<float> v(5);
		for (unsigned i = 0; i < 5; ++i) v[i] = float(i) * 1.5f;

		typed_datafile<SrcTypes> src;
		src.add(v, "test_data");
		std::stringstream src_stream;
		src.save(src_stream);

		std::cout << "Source:\n" << src_stream.str() << '\n';

		// translate
		type_map mapping = { make_mapping<float, double>() };
		std::stringstream dst_stream;
		translate<SrcTypes, DstTypes>(src_stream, dst_stream, mapping);

		std::cout << "Translated:\n" << dst_stream.str() << '\n';

		// restore as double
		typed_datafile<DstTypes> dst;
		dst.restore(dst_stream);
		std::cout << "Restored " << dst.size() << " datasets\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1

	// Test 1: identity translation (float -> float)
	{
		int start = nrOfFailedTestCases;
		using Types = type_list<float>;

		vector<float> v(5);
		for (unsigned i = 0; i < 5; ++i) v[i] = float(i) * 0.5f;

		typed_datafile<Types> src;
		src.add(v, "identity_test");
		std::stringstream src_ss;
		src.save(src_ss);

		type_map mapping = { make_mapping<float, float>() };
		std::stringstream dst_ss;
		if (!translate<Types, Types>(src_ss, dst_ss, mapping)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: identity translation failed\n";
		}

		typed_datafile<Types> dst;
		if (!dst.restore(dst_ss)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: identity restore failed\n";
		}
		if (dst.size() != 1) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: expected 1 dataset\n";
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: identity translation\n";
	}

	// Test 2: float -> double translation (widening)
	{
		int start = nrOfFailedTestCases;
		using SrcTypes = type_list<float>;
		using DstTypes = type_list<double>;

		vector<float> v(4);
		v[0] = 1.5f; v[1] = -2.25f; v[2] = 0.0f; v[3] = 100.0f;

		typed_datafile<SrcTypes> src;
		src.add(v, "widening");
		std::stringstream src_ss;
		src.save(src_ss);

		type_map mapping = { make_mapping<float, double>() };
		std::stringstream dst_ss;
		if (!translate<SrcTypes, DstTypes>(src_ss, dst_ss, mapping)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: widening translation failed\n";
		}

		typed_datafile<DstTypes> dst;
		if (!dst.restore(dst_ss)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: widening restore failed\n";
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: float->double translation\n";
	}

	// Test 3: double -> float translation (narrowing)
	{
		int start = nrOfFailedTestCases;
		using SrcTypes = type_list<double>;
		using DstTypes = type_list<float>;

		vector<double> v(3);
		v[0] = 1.0; v[1] = 2.0; v[2] = 3.0;

		typed_datafile<SrcTypes> src;
		src.add(v, "narrowing");
		std::stringstream src_ss;
		src.save(src_ss);

		type_map mapping = { make_mapping<double, float>() };
		std::stringstream dst_ss;
		if (!translate<SrcTypes, DstTypes>(src_ss, dst_ss, mapping)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: narrowing translation failed\n";
		}

		typed_datafile<DstTypes> dst;
		if (!dst.restore(dst_ss)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: narrowing restore failed\n";
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: double->float translation\n";
	}

	// Test 4: unmapped type produces warning
	{
		int start = nrOfFailedTestCases;
		using SrcTypes = type_list<float, double>;
		using DstTypes = type_list<double>;

		vector<float> v(2);
		v[0] = 1.0f; v[1] = 2.0f;

		typed_datafile<SrcTypes> src;
		src.add(v, "unmapped");
		std::stringstream src_ss;
		src.save(src_ss);

		// no mapping for float -> anything
		type_map empty_map;
		std::stringstream dst_ss;
		// should succeed but skip unmapped type
		translate<SrcTypes, DstTypes>(src_ss, dst_ss, empty_map);

		typed_datafile<DstTypes> dst;
		dst.restore(dst_ss);
		// no datasets should be restored (float was skipped)
		if (dst.size() != 0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: unmapped type should produce 0 datasets\n";
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: unmapped type test\n";
	}

	// Test 5: multi-dataset translation with mixed types
	{
		int start = nrOfFailedTestCases;
		using SrcTypes = type_list<float, double>;
		using DstTypes = type_list<float, double>;

		vector<float> fv(3);
		fv[0] = 1.0f; fv[1] = 2.0f; fv[2] = 3.0f;
		vector<double> dv(2);
		dv[0] = 4.5; dv[1] = 5.5;

		typed_datafile<SrcTypes> src;
		src.add(fv, "floats");
		src.add(dv, "doubles");
		std::stringstream src_ss;
		src.save(src_ss);

		type_map mapping = {
			make_mapping<float, float>(),
			make_mapping<double, double>()
		};
		std::stringstream dst_ss;
		if (!translate<SrcTypes, DstTypes>(src_ss, dst_ss, mapping)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: multi-dataset translation failed\n";
		}

		typed_datafile<DstTypes> dst;
		if (!dst.restore(dst_ss)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: multi-dataset restore failed\n";
		}
		if (dst.size() != 2) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: expected 2 datasets, got " << dst.size() << '\n';
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: multi-dataset translation\n";
	}

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
