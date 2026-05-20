// api.cpp: Phase A canary test for the elreal (Exact Lazy Real) number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase A scope: exercise the foundation skeleton (issue #874 of epic #873).
// The lazy machinery (closure-based generator, multi-component refinement,
// arithmetic) is intentionally absent here; this test just verifies the type
// compiles, constructs from native types, and that the refinement-protocol
// entry points are callable.
//
// As later phases land, this file grows into a full API tour analogous to
// elastic/ereal/api/api.cpp.

#include <universal/utility/directives.hpp>

#define ELREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "elreal<> Phase A foundation skeleton";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- behavioral traits ---------------------------------------------------
	{
		// Universal's library-wide convention is to *report* triviality rather
		// than assert it for elastic / variable-precision types. elreal holds
		// std::vector<double>, so it is neither trivially constructible nor
		// trivially copyable -- that is expected.
		bool isTrivial = std::is_trivial<elreal>::value;
		std::string testType = type_tag(elreal{});
		std::cout << testType
			<< (isTrivial ? " is trivial" : " is not trivial")
			<< " (expected: not trivial for an elastic type)\n";
	}

	// --- construction --------------------------------------------------------
	std::cout << "+--------- elreal construction\n";
	{
		elreal zero;
		elreal one(1.0);
		elreal pi_approx(3.141592653589793);
		elreal neg(-2.5);
		elreal big(1.0e100);
		elreal i(int{42});
		elreal ll(static_cast<long long>(1) << 40);

		std::cout << "zero:        " << to_binary(zero)      << '\n';
		std::cout << "1.0:         " << to_binary(one)       << '\n';
		std::cout << "pi approx:   " << to_binary(pi_approx) << '\n';
		std::cout << "-2.5:        " << to_binary(neg)       << '\n';
		std::cout << "1e100:       " << to_binary(big)       << '\n';
		std::cout << "int(42):     " << to_binary(i)         << '\n';
		std::cout << "1LL << 40:   " << to_binary(ll)        << '\n';

		// Verify the elementary invariants we *can* check in Phase A.
		if (!zero.iszero())             { ++nrOfFailedTestCases; std::cerr << "FAIL: default-constructed elreal is not zero\n"; }
		if (double(one)        != 1.0)  { ++nrOfFailedTestCases; std::cerr << "FAIL: elreal(1.0) decoded != 1.0\n"; }
		if (double(neg)        != -2.5) { ++nrOfFailedTestCases; std::cerr << "FAIL: elreal(-2.5) decoded != -2.5\n"; }
		if (double(i)          != 42.0) { ++nrOfFailedTestCases; std::cerr << "FAIL: elreal(int 42) decoded != 42.0\n"; }
	}

	// --- specific values -----------------------------------------------------
	std::cout << "+--------- elreal specific values\n";
	{
		elreal infpos(SpecificValue::infpos);
		elreal infneg(SpecificValue::infneg);
		elreal qnan(SpecificValue::qnan);
		elreal zero(SpecificValue::zero);

		std::cout << "+inf:        " << to_binary(infpos) << '\n';
		std::cout << "-inf:        " << to_binary(infneg) << '\n';
		std::cout << "qnan:        " << to_binary(qnan)   << '\n';
		std::cout << "zero:        " << to_binary(zero)   << '\n';

		if (!infpos.isinf()) { ++nrOfFailedTestCases; std::cerr << "FAIL: +inf isinf() returned false\n"; }
		if (!infneg.isinf()) { ++nrOfFailedTestCases; std::cerr << "FAIL: -inf isinf() returned false\n"; }
		if (!qnan.isnan())   { ++nrOfFailedTestCases; std::cerr << "FAIL: qnan isnan() returned false\n"; }
		if (!zero.iszero())  { ++nrOfFailedTestCases; std::cerr << "FAIL: SpecificValue::zero iszero() returned false\n"; }
	}

	// --- refinement protocol -------------------------------------------------
	std::cout << "+--------- elreal refinement protocol (Phase A: stub)\n";
	{
		elreal v(3.14);
		// at(0) should return the leading component.
		if (v.at(0) != 3.14) {
			++nrOfFailedTestCases;
			std::cerr << "FAIL: at(0) did not return the leading component\n";
		}
		// at(k) for k beyond the materialised depth returns the implicit zero
		// extension in Phase A. Phase C will replace this with generator-driven
		// refinement.
		if (v.at(5) != 0.0) {
			++nrOfFailedTestCases;
			std::cerr << "FAIL: at(beyond depth) did not return 0.0 (Phase A stub)\n";
		}
		// refine_to is a no-op in Phase A; verify it does not crash and does
		// not change observable state.
		std::size_t depth_before = v.computed_depth();
		v.refine_to(80);
		std::size_t depth_after  = v.computed_depth();
		if (depth_before != depth_after) {
			++nrOfFailedTestCases;
			std::cerr << "FAIL: refine_to changed computed_depth in Phase A (should be no-op)\n";
		}
		std::cout << "computed_depth after refine_to(80): " << v.computed_depth() << '\n';
	}

	// --- traits --------------------------------------------------------------
	std::cout << "+--------- elreal trait dispatch\n";
	{
		static_assert(is_elreal< elreal >,         "is_elreal<elreal> must be true");
		static_assert(!is_elreal< double >,        "is_elreal<double> must be false");
		std::cout << "is_elreal<elreal> = true (compile-time)\n";
	}

	// --- numeric_limits ------------------------------------------------------
	std::cout << "+--------- std::numeric_limits<elreal>\n";
	{
		using limits = std::numeric_limits<elreal>;
		std::cout << "digits         = " << limits::digits         << '\n';
		std::cout << "digits10       = " << limits::digits10       << '\n';
		std::cout << "is_signed      = " << limits::is_signed      << '\n';
		std::cout << "is_exact       = " << limits::is_exact       << '\n';
		std::cout << "is_bounded     = " << limits::is_bounded     << '\n';
		std::cout << "has_infinity   = " << limits::has_infinity   << '\n';
		std::cout << "has_quiet_NaN  = " << limits::has_quiet_NaN  << '\n';
		std::cout << "epsilon        = " << double(limits::epsilon())  << '\n';

		if (!limits::is_specialized) {
			++nrOfFailedTestCases;
			std::cerr << "FAIL: numeric_limits<elreal>::is_specialized is false\n";
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception\n";
	return EXIT_FAILURE;
}
