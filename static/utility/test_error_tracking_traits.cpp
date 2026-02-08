// test_error_tracking_traits.cpp: verify error_tracking_traits compile correctly
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#include <iostream>
#include <universal/utility/error_tracking_traits.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/interval/interval.hpp>

using namespace sw::universal;

template<typename T>
void test_traits(const char* name) {
	using traits = error_tracking_traits<T>;
	std::cout << name << ":\n";
	std::cout << "  has_exact_errors:     " << (traits::has_exact_errors ? "yes" : "no") << '\n';
	std::cout << "  exact_multiplication: " << (traits::exact_multiplication ? "yes" : "no") << '\n';
	std::cout << "  tracks_uncertainty:   " << (traits::tracks_uncertainty ? "yes" : "no") << '\n';
	std::cout << "  is_interval_type:     " << (traits::is_interval_type ? "yes" : "no") << '\n';
	std::cout << "  default_strategy:     " << strategy_name(traits::default_strategy) << '\n';
	std::cout << "  nbits:                " << traits::nbits << '\n';
	std::cout << '\n';
}

int main() {
	std::cout << "Error Tracking Traits Test\n";
	std::cout << "==========================\n\n";

	// Native types
	test_traits<float>("float");
	test_traits<double>("double");

	// Universal types
	test_traits<cfloat<32, 8>>("cfloat<32,8>");
	test_traits<posit<32, 2>>("posit<32,2>");
	test_traits<lns<32, 8>>("lns<32,8>");
	test_traits<areal<32, 8>>("areal<32,8>");
	test_traits<interval<double>>("interval<double>");

	// Compile-time assertions
	static_assert(has_exact_errors_v<float>, "float should have exact errors");
	static_assert(has_exact_errors_v<double>, "double should have exact errors");
	static_assert(!has_exact_errors_v<cfloat<32,8>>, "cfloat uses Shadow (no volatile/isfinite)");
	static_assert(!has_exact_errors_v<posit<32,2>>, "posit should NOT have exact errors");

	static_assert(exact_multiplication_v<lns<32,8>>, "lns should have exact multiplication");
	static_assert(!exact_multiplication_v<float>, "float should NOT have exact multiplication");

	static_assert(tracks_uncertainty_v<areal<32,8>>, "areal should track uncertainty");
	static_assert(tracks_uncertainty_v<interval<double>>, "interval should track uncertainty");
	static_assert(!tracks_uncertainty_v<float>, "float should NOT track uncertainty");

	static_assert(is_interval_type_v<areal<32,8>>, "areal is an interval type");
	static_assert(is_interval_type_v<interval<double>>, "interval is an interval type");
	static_assert(!is_interval_type_v<float>, "float is NOT an interval type");

	static_assert(default_strategy_v<float> == ErrorStrategy::Exact, "float default is Exact");
	static_assert(default_strategy_v<posit<32,2>> == ErrorStrategy::Shadow, "posit default is Shadow");
	static_assert(default_strategy_v<areal<32,8>> == ErrorStrategy::Inherent, "areal default is Inherent");

	std::cout << "All compile-time assertions passed!\n";
	std::cout << "error_tracking_traits: PASS\n";

	return 0;
}
