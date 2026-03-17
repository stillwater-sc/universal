// typed_serialization.cpp: test suite for client-directed serialization
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <universal/number_systems.hpp>
#include <blas/blas.hpp>
#include <blas/generators.hpp>
#include <blas/serialization/datafile.hpp>
#include <blas/serialization/typed_datafile.hpp>
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

	std::string test_suite = "client-directed typed serialization";
	std::string test_tag = "typed_datafile";
	bool reportTestCases = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		// declare a client type list
		using ClientTypes = type_list<float, double, half>;

		// create and populate a vector
		vector<float> v(5);
		for (unsigned i = 0; i < 5; ++i) v[i] = float(i) * 1.5f;

		// save
		typed_datafile<ClientTypes> df;
		df.add(v, "test_vector");
		std::stringstream ss;
		df.save(ss);

		std::cout << "Saved datafile:\n" << ss.str() << '\n';

		// restore
		typed_datafile<ClientTypes> df2;
		if (df2.restore(ss)) {
			std::cout << "Restored successfully, " << df2.size() << " datasets\n";
			std::cout << "Dataset name: " << df2.name(0) << '\n';
		}
		else {
			std::cout << "Restore failed\n";
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1

	// Test 1: float vector round-trip
	{
		int start = nrOfFailedTestCases;
		using ClientTypes = type_list<float, double>;

		vector<float> original(10);
		for (unsigned i = 0; i < 10; ++i) original[i] = float(i) * 0.5f;

		typed_datafile<ClientTypes> saver;
		saver.add(original, "float_vec");
		std::stringstream ss;
		saver.save(ss);

		typed_datafile<ClientTypes> loader;
		if (!loader.restore(ss)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: float vector restore failed\n";
		}
		if (loader.size() != 1) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: expected 1 dataset, got " << loader.size() << '\n';
		}
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: float vector round-trip\n";
		}
	}

	// Test 2: double vector round-trip
	{
		int start = nrOfFailedTestCases;
		using ClientTypes = type_list<float, double>;

		vector<double> original(8);
		for (unsigned i = 0; i < 8; ++i) original[i] = double(i) * 1.25;

		typed_datafile<ClientTypes> saver;
		saver.add(original, "double_vec");
		std::stringstream ss;
		saver.save(ss);

		typed_datafile<ClientTypes> loader;
		if (!loader.restore(ss)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: double vector restore failed\n";
		}
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: double vector round-trip\n";
		}
	}

	// Test 3: cfloat (half) vector round-trip
	{
		int start = nrOfFailedTestCases;
		using ClientTypes = type_list<float, double, half>;

		vector<half> original(6);
		for (unsigned i = 0; i < 6; ++i) original[i] = half(float(i) * 0.25f);

		typed_datafile<ClientTypes> saver;
		saver.add(original, "half_vec");
		std::stringstream ss;
		saver.save(ss);

		typed_datafile<ClientTypes> loader;
		if (!loader.restore(ss)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: half vector restore failed\n";
		}
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: half vector round-trip\n";
		}
	}

	// Test 4: type mismatch -- save with half, restore without half in registry
	{
		int start = nrOfFailedTestCases;
		using SaveTypes = type_list<float, double, half>;
		using LoadTypes = type_list<float, double>;  // no half!

		vector<half> original(4);
		for (unsigned i = 0; i < 4; ++i) original[i] = half(float(i));

		typed_datafile<SaveTypes> saver;
		saver.add(original, "half_data");
		std::stringstream ss;
		saver.save(ss);

		// restore with a registry that lacks half -- should report unsupported type
		typed_datafile<LoadTypes> loader;
		// restore returns true even if some types are unsupported (skipped)
		loader.restore(ss);
		// the dataset should not have been restored
		if (loader.size() != 0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: type mismatch should result in 0 restored datasets\n";
		}
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: type mismatch test\n";
		}
	}

	// Test 5: multiple datasets in one file
	{
		int start = nrOfFailedTestCases;
		using ClientTypes = type_list<float, double>;

		vector<float> fv(3);
		fv[0] = 1.0f; fv[1] = 2.0f; fv[2] = 3.0f;
		vector<double> dv(2);
		dv[0] = 4.5; dv[1] = 5.5;

		typed_datafile<ClientTypes> saver;
		saver.add(fv, "floats");
		saver.add(dv, "doubles");
		std::stringstream ss;
		saver.save(ss);

		typed_datafile<ClientTypes> loader;
		if (!loader.restore(ss)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: multi-dataset restore failed\n";
		}
		if (loader.size() != 2) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: expected 2 datasets, got " << loader.size() << '\n';
		}
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: multiple datasets\n";
		}
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
