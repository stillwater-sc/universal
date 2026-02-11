// chaotic_bank.cpp: The Chaotic Bank Society - demonstrating ubit tracking through iterations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.

/*
 * The Chaotic Bank Society Problem:
 *
 * Initial deposit: (e - 1) dollars, where e = 2.71828...
 *
 * Each year: balance = balance * year - 1
 *
 * After 25 years, what is the balance?
 *
 * Mathematical answer: ~$0.0399387296732302
 *
 * IEEE behavior: The balance goes NEGATIVE after about 13 years, which is
 * physically impossible for a bank account (you can't have negative money
 * in a deposit account). IEEE floats give a nonsensical result.
 *
 * areal with ubit: The uncertainty should accumulate with each iteration,
 * warning the programmer before the result becomes meaningless.
 *
 * This demonstrates: IEEE silently produces impossible results.
 *                    areal's ubit tracks growing uncertainty.
 */

#include <universal/utility/directives.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <iomanip>
#include <cmath>
#include <vector>

namespace sw { namespace universal {

template<typename Scalar>
struct BankResult {
	int year;
	Scalar balance;
	bool uncertain;
};

template<typename Scalar>
std::vector<BankResult<Scalar>> chaotic_bank(int years) {
	std::vector<BankResult<Scalar>> results;

	// Initial deposit: e - 1
	Scalar e_minus_1 = Scalar(std::exp(1.0) - 1.0);
	Scalar balance = e_minus_1;

	results.push_back({0, balance, false});

	for (int year = 1; year <= years; ++year) {
		balance = balance * Scalar(year) - Scalar(1);

		bool uncertain = false;
		if constexpr (std::is_same_v<Scalar, areal<32, 8, uint32_t>> ||
		              std::is_same_v<Scalar, areal<64, 11, uint64_t>> ||
		              std::is_same_v<Scalar, areal<16, 5, uint16_t>>) {
			uncertain = balance.at(0); // ubit is LSB
		}

		results.push_back({year, balance, uncertain});
	}

	return results;
}

template<typename Scalar>
void test_chaotic_bank(const std::string& type_name, int years) {
	auto results = chaotic_bank<Scalar>(years);

	std::cout << "\n" << type_name << ":\n";
	std::cout << std::setw(6) << "Year" << std::setw(25) << "Balance" << std::setw(15) << "Status" << '\n';
	std::cout << std::string(50, '-') << '\n';

	int first_negative_year = -1;
	int first_uncertain_year = -1;

	for (const auto& r : results) {
		std::cout << std::setw(6) << r.year
		          << std::setw(25) << std::setprecision(15) << double(r.balance);

		if (r.uncertain) {
			std::cout << std::setw(15) << "[UNCERTAIN]";
			if (first_uncertain_year < 0) first_uncertain_year = r.year;
		} else if (double(r.balance) < 0) {
			std::cout << std::setw(15) << "[NEGATIVE!]";
			if (first_negative_year < 0) first_negative_year = r.year;
		} else {
			std::cout << std::setw(15) << "";
		}
		std::cout << '\n';
	}

	std::cout << "\nFinal balance after " << years << " years: $"
	          << std::setprecision(15) << double(results.back().balance) << '\n';
	std::cout << "Correct answer: $0.0399387296732302\n";

	if (first_negative_year > 0) {
		std::cout << "WARNING: Balance went negative at year " << first_negative_year
		          << " (impossible!)\n";
	}
	if (first_uncertain_year > 0) {
		std::cout << "INFO: Uncertainty detected starting at year " << first_uncertain_year << '\n';
	}
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::cout << "The Chaotic Bank Society Problem\n";
	std::cout << "Initial deposit: (e - 1) = $" << std::setprecision(15) << (std::exp(1.0) - 1.0) << '\n';
	std::cout << "Annual operation: balance = balance * year - 1\n";
	std::cout << "Correct balance after 25 years: $0.0399387296732302\n";
	std::cout << std::string(80, '=') << '\n';

	constexpr int years = 25;

	// IEEE types - go negative (impossible!)
	std::cout << "\n=== IEEE Floating-Point (goes negative - IMPOSSIBLE) ===";
	test_chaotic_bank<float>("float", years);
	test_chaotic_bank<double>("double", years);

	// cfloat for comparison
	std::cout << "\n=== cfloat (IEEE-style, no ubit) ===";
	test_chaotic_bank<cfloat<32, 8, uint32_t>>("cfloat<32,8>", years);
	test_chaotic_bank<cfloat<64, 11, uint64_t>>("cfloat<64,11>", years);

	// areal with ubit
	std::cout << "\n=== areal (with ubit uncertainty tracking) ===";
	test_chaotic_bank<areal<32, 8, uint32_t>>("areal<32,8>", years);
	test_chaotic_bank<areal<64, 11, uint64_t>>("areal<64,11>", years);

	std::cout << "\n" << std::string(80, '=') << '\n';
	std::cout << "Key insight:\n";
	std::cout << "  - IEEE floats produce negative balances (physically impossible)\n";
	std::cout << "  - A negative bank balance should never occur from this formula\n";
	std::cout << "  - areal's ubit warns when precision loss makes results unreliable\n";
	std::cout << "  - The ubit serves as an early warning system\n";

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
