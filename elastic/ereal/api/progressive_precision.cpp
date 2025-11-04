// progressive_precision.cpp: demonstrate precision scaling with maxlimbs for ereal mathlib functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// OVERVIEW:
// ---------
// This test demonstrates that ereal adaptive-precision arithmetic achieves higher precision
// as maxlimbs increases. Each limb provides ~53 bits (~15.95 decimal digits) of precision.
//
// REFERENCE VALUES:
// -----------------
// All reference values were computed using MPFR (via Python's mpmath library) at 256-bit precision.
// The generation script is documented below for reproducibility.
//
// PYTHON SCRIPT TO GENERATE REFERENCE VALUES:
// --------------------------------------------
// from mpmath import mp
// mp.dps = 100  # 100 decimal digits
//
// # Trigonometric
// print(f"sin(0.5)  = {mp.sin(0.5)}")
// print(f"cos(0.3)  = {mp.cos(0.3)}")
// print(f"tan(0.4)  = {mp.tan(0.4)}")
// print(f"atan(1.0) = {mp.atan(1.0)}")  # π/4
// print(f"asin(0.5) = {mp.asin(0.5)}")  # π/6
// print(f"acos(0.5) = {mp.acos(0.5)}")  # π/3
//
// # Exponential
// print(f"exp(1.0)  = {mp.exp(1.0)}")   # e
// print(f"exp2(3.5) = {mp.power(2, 3.5)}")
// print(f"log(2.0)  = {mp.log(2.0)}")
// print(f"log2(10)  = {mp.log(10, 2)}")
// print(f"log10(100)= {mp.log10(100)}")
//
// # Hyperbolic
// print(f"sinh(0.5) = {mp.sinh(0.5)}")
// print(f"cosh(0.5) = {mp.cosh(0.5)}")
// print(f"tanh(0.5) = {mp.tanh(0.5)}")
// print(f"asinh(1)  = {mp.asinh(1.0)}")
// print(f"acosh(2)  = {mp.acosh(2.0)}")
// print(f"atanh(0.5)= {mp.atanh(0.5)}")
//
// # Power/Root
// print(f"sqrt(2)   = {mp.sqrt(2)}")
// print(f"pow(2,3.5)= {mp.power(2, 3.5)}")
//
// EXPECTED PRECISION:
// -------------------
// Each limb provides ~53 bits = ~15.95 decimal digits
// ereal<4>  : ~64 digits
// ereal<8>  : ~128 digits → expect ≥ 30.0 decimal digits (allowing margin)
// ereal<12> : ~192 digits → expect ≥ 45.0 decimal digits
// ereal<16> : ~256 digits → expect ≥ 60.0 decimal digits
// ereal<19> : ~304 digits → expect ≥ 72.0 decimal digits (maxlimbs=19 is Shewchuk's limit)
//

#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>

namespace sw { namespace universal {

// Helper: compute decimal digits of precision from relative error
double decimal_digits_precision(double relative_error) {
	if (relative_error <= 0.0) return 100.0; // Perfect match
	return -std::log10(relative_error);
}

// Helper: compute relative error between ereal and reference string
template<unsigned maxlimbs>
double compute_relative_error(const ereal<maxlimbs>& computed, const std::string& reference) {
	using Real = ereal<maxlimbs>;

	// Parse reference string directly using string constructor
	// This maintains full precision (all 100+ digits) using the parse() function
	Real ref(reference);

	if (ref.iszero()) {
		// For zero reference, use absolute error
		Real diff = abs(computed - ref);
		return double(diff);
	}

	Real diff = abs(computed - ref);
	Real rel_error = diff / abs(ref);
	return double(rel_error);
}

// Test a single function at multiple precision levels
struct TestResult {
	std::string function_name;
	std::string test_value;
	std::string reference;
	double digits[5];  // ereal<4>, <8>, <12>, <16>, <19>
	bool passed[5];

	TestResult(const std::string& name, const std::string& test_val, const std::string& ref)
		: function_name(name), test_value(test_val), reference(ref) {
		for (int i = 0; i < 5; ++i) {
			digits[i] = 0.0;
			passed[i] = false;
		}
	}
};

// Expected precision thresholds (decimal digits)
const double PRECISION_THRESHOLDS[5] = {
	15.0,  // ereal<4>
	30.0,  // ereal<8>
	45.0,  // ereal<12>
	60.0,  // ereal<16>
	72.0   // ereal<19>
};

const char* MAXLIMBS_LABELS[5] = {
	"ereal<4> ",
	"ereal<8> ",
	"ereal<12>",
	"ereal<16>",
	"ereal<19>"
};

// Template to test a function at all precision levels
template<typename Func>
TestResult test_function_progressive(
	const std::string& name,
	const std::string& test_value,
	const std::string& reference,
	Func func)
{
	TestResult result(name, test_value, reference);

	// Convert test value to double
	double test_val_double = std::stod(test_value);

	// Test ereal<4>
	{
		using Real = ereal<4>;
		Real input(test_val_double);
		Real computed = func(input);
		double rel_error = compute_relative_error(computed, reference);
		result.digits[0] = decimal_digits_precision(rel_error);
		result.passed[0] = (result.digits[0] >= PRECISION_THRESHOLDS[0]);
	}

	// Test ereal<8>
	{
		using Real = ereal<8>;
		Real input(test_val_double);
		Real computed = func(input);
		double rel_error = compute_relative_error(computed, reference);
		result.digits[1] = decimal_digits_precision(rel_error);
		result.passed[1] = (result.digits[1] >= PRECISION_THRESHOLDS[1]);
	}

	// Test ereal<12>
	{
		using Real = ereal<12>;
		Real input(test_val_double);
		Real computed = func(input);
		double rel_error = compute_relative_error(computed, reference);
		result.digits[2] = decimal_digits_precision(rel_error);
		result.passed[2] = (result.digits[2] >= PRECISION_THRESHOLDS[2]);
	}

	// Test ereal<16>
	{
		using Real = ereal<16>;
		Real input(test_val_double);
		Real computed = func(input);
		double rel_error = compute_relative_error(computed, reference);
		result.digits[3] = decimal_digits_precision(rel_error);
		result.passed[3] = (result.digits[3] >= PRECISION_THRESHOLDS[3]);
	}

	// Test ereal<19>
	{
		using Real = ereal<19>;
		Real input(test_val_double);
		Real computed = func(input);
		double rel_error = compute_relative_error(computed, reference);
		result.digits[4] = decimal_digits_precision(rel_error);
		result.passed[4] = (result.digits[4] >= PRECISION_THRESHOLDS[4]);
	}

	return result;
}

// Print test result with verbose output
void print_result(const TestResult& result) {
	std::cout << "\n" << result.function_name << " = " << result.reference.substr(0, 40);
	if (result.reference.length() > 40) std::cout << "...";
	std::cout << "\n";

	for (int i = 0; i < 5; ++i) {
		std::cout << "  " << MAXLIMBS_LABELS[i] << " : "
		          << std::fixed << std::setprecision(1) << std::setw(5) << result.digits[i]
		          << " digits  [" << (result.passed[i] ? "PASS" : "FAIL")
		          << ": ≥" << std::setw(4) << PRECISION_THRESHOLDS[i] << " expected]";

		if (!result.passed[i]) {
			std::cout << " *** PRECISION LOSS DETECTED ***";
		}
		std::cout << "\n";
	}
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::cout << "Progressive Precision Validation - ereal mathlib\n";
	std::cout << "=================================================\n";
	std::cout << "\nDemonstrating that precision scales with maxlimbs:\n";
	std::cout << "  ereal<4>  : ~64 bits  → expect ≥15.0 decimal digits\n";
	std::cout << "  ereal<8>  : ~128 bits → expect ≥30.0 decimal digits\n";
	std::cout << "  ereal<12> : ~192 bits → expect ≥45.0 decimal digits\n";
	std::cout << "  ereal<16> : ~256 bits → expect ≥60.0 decimal digits\n";
	std::cout << "  ereal<19> : ~304 bits → expect ≥72.0 decimal digits\n";

	std::vector<TestResult> results;

	// ============================================================================
	// TRIGONOMETRIC FUNCTIONS
	// ============================================================================
	std::cout << "\n\n" << std::string(80, '=') << "\n";
	std::cout << "TRIGONOMETRIC FUNCTIONS\n";
	std::cout << std::string(80, '=') << "\n";

	// sin(0.5)
	{
		std::string ref = "0.4794255386042030002732879352155713880818033679406006751886166131255350002878148322096312593584388216822360379827881";
		auto result = test_function_progressive("sin(0.5)", "0.5", ref,
			[](auto x) { return sin(x); });
		print_result(result);
		results.push_back(result);
	}

	// cos(0.3)
	{
		std::string ref = "0.9553364891256060004824327720529678097339139475361667095294594785628626284032262808544623978143285414705738040906012";
		auto result = test_function_progressive("cos(0.3)", "0.3", ref,
			[](auto x) { return cos(x); });
		print_result(result);
		results.push_back(result);
	}

	// tan(0.4)
	{
		std::string ref = "0.4227932187381618116931497609557478883481494163513254278090894820786333046691327681475264935806695554378711804484897";
		auto result = test_function_progressive("tan(0.4)", "0.4", ref,
			[](auto x) { return tan(x); });
		print_result(result);
		results.push_back(result);
	}

	// atan(1.0) = π/4
	{
		std::string ref = "0.7853981633974483096156608458198757210492923498437764552437361480769541015715522496570087063355292669955370216084252";
		auto result = test_function_progressive("atan(1.0) [π/4]", "1.0", ref,
			[](auto x) { return atan(x); });
		print_result(result);
		results.push_back(result);
	}

	// asin(0.5) = π/6
	{
		std::string ref = "0.5235987755982988730771072305465838140328615665625176368291574320513027343810348330856695354450976446636856806947501";
		auto result = test_function_progressive("asin(0.5) [π/6]", "0.5", ref,
			[](auto x) { return asin(x); });
		print_result(result);
		results.push_back(result);
	}

	// acos(0.5) = π/3
	{
		std::string ref = "1.0471975511965977461542144610931676280657231331250352736583148641026054687620696661713390708901952893273713613895003";
		auto result = test_function_progressive("acos(0.5) [π/3]", "0.5", ref,
			[](auto x) { return acos(x); });
		print_result(result);
		results.push_back(result);
	}

	// ============================================================================
	// EXPONENTIAL FUNCTIONS
	// ============================================================================
	std::cout << "\n\n" << std::string(80, '=') << "\n";
	std::cout << "EXPONENTIAL FUNCTIONS\n";
	std::cout << std::string(80, '=') << "\n";

	// exp(1.0) = e
	{
		std::string ref = "2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274274663919320030";
		auto result = test_function_progressive("exp(1.0) [e]", "1.0", ref,
			[](auto x) { return exp(x); });
		print_result(result);
		results.push_back(result);
	}

	// exp2(3.5) = 2^3.5
	{
		std::string ref = "11.313708498984760390413509793678608625401020174408749910990316968806148217965042679622508083576029169945606040605569";
		auto result = test_function_progressive("exp2(3.5)", "3.5", ref,
			[](auto x) { return exp2(x); });
		print_result(result);
		results.push_back(result);
	}

	// exp10(1.5) = 10^1.5
	{
		std::string ref = "31.622776601683793319988935444327185337195551393252168268575048527925944386392382213442481083793002951873472841528400";
		auto result = test_function_progressive("exp10(1.5)", "1.5", ref,
			[](auto x) { return exp10(x); });
		print_result(result);
		results.push_back(result);
	}

	// ============================================================================
	// LOGARITHM FUNCTIONS
	// ============================================================================
	std::cout << "\n\n" << std::string(80, '=') << "\n";
	std::cout << "LOGARITHM FUNCTIONS\n";
	std::cout << std::string(80, '=') << "\n";

	// log(2.0) = ln(2)
	{
		std::string ref = "0.6931471805599453094172321214581765680755001343602552541206800094933936219696947156058633269964186875420014810205706";
		auto result = test_function_progressive("log(2.0) [ln(2)]", "2.0", ref,
			[](auto x) { return log(x); });
		print_result(result);
		results.push_back(result);
	}

	// log2(10.0)
	{
		std::string ref = "3.3219280948873623478703194294893901758648313930245806120547563958159347766086252158501397433593701550370162060715096";
		auto result = test_function_progressive("log2(10.0)", "10.0", ref,
			[](auto x) { return log2(x); });
		print_result(result);
		results.push_back(result);
	}

	// log10(100.0) = 2.0 (exact)
	{
		std::string ref = "2.0";
		auto result = test_function_progressive("log10(100.0)", "100.0", ref,
			[](auto x) { return log10(x); });
		print_result(result);
		results.push_back(result);
	}

	// ============================================================================
	// HYPERBOLIC FUNCTIONS
	// ============================================================================
	std::cout << "\n\n" << std::string(80, '=') << "\n";
	std::cout << "HYPERBOLIC FUNCTIONS\n";
	std::cout << std::string(80, '=') << "\n";

	// sinh(0.5)
	{
		std::string ref = "0.5210953054937473616224256264115338908227967395892080826402541122932743168317203184713358105094227541023704408852603";
		auto result = test_function_progressive("sinh(0.5)", "0.5", ref,
			[](auto x) { return sinh(x); });
		print_result(result);
		results.push_back(result);
	}

	// cosh(0.5)
	{
		std::string ref = "1.1276259652063807852262251614026720125478471180986674836290696978149515094021871428580466125732910130093919532057963";
		auto result = test_function_progressive("cosh(0.5)", "0.5", ref,
			[](auto x) { return cosh(x); });
		print_result(result);
		results.push_back(result);
	}

	// tanh(0.5)
	{
		std::string ref = "0.4621171572600097585023184836436725108210941790546185593449757916976392348691534336814753146855984174452409883405474";
		auto result = test_function_progressive("tanh(0.5)", "0.5", ref,
			[](auto x) { return tanh(x); });
		print_result(result);
		results.push_back(result);
	}

	// asinh(1.0)
	{
		std::string ref = "0.8813735870195430252326093249797923090281603282616354107532956086252745362489405650896089311571393832711353539486524";
		auto result = test_function_progressive("asinh(1.0)", "1.0", ref,
			[](auto x) { return asinh(x); });
		print_result(result);
		results.push_back(result);
	}

	// acosh(2.0)
	{
		std::string ref = "1.3169578969248167086250463473079684440269819714675164797684722569204228929052466195534439706186403763338066537774832";
		auto result = test_function_progressive("acosh(2.0)", "2.0", ref,
			[](auto x) { return acosh(x); });
		print_result(result);
		results.push_back(result);
	}

	// atanh(0.5)
	{
		std::string ref = "0.5493061443340548456976226184612628523237452789113747258673471668187471466093044834368078774068660443939850145329706";
		auto result = test_function_progressive("atanh(0.5)", "0.5", ref,
			[](auto x) { return atanh(x); });
		print_result(result);
		results.push_back(result);
	}

	// ============================================================================
	// POWER AND ROOT FUNCTIONS
	// ============================================================================
	std::cout << "\n\n" << std::string(80, '=') << "\n";
	std::cout << "POWER AND ROOT FUNCTIONS\n";
	std::cout << std::string(80, '=') << "\n";

	// sqrt(2.0)
	{
		std::string ref = "1.4142135623730950488016887242096980785696718753769480731766797379907324784621070388503875343276415727350138462309122";
		auto result = test_function_progressive("sqrt(2.0)", "2.0", ref,
			[](auto x) { return sqrt(x); });
		print_result(result);
		results.push_back(result);
	}

	// pow(2.0, 3.5)
	{
		std::string ref = "11.313708498984760390413509793678608625401020174408749910990316968806148217965042679622508083576029169945606040605569";
		auto result = test_function_progressive("pow(2.0, 3.5)", "2.0", ref,
			[](auto x) {
				using Real = decltype(x);
				return pow(x, Real(3.5));
			});
		print_result(result);
		results.push_back(result);
	}

	// ============================================================================
	// SUMMARY
	// ============================================================================
	std::cout << "\n\n" << std::string(80, '=') << "\n";
	std::cout << "SUMMARY\n";
	std::cout << std::string(80, '=') << "\n\n";

	// Count passes/fails for each precision level
	int passed_by_level[5] = {0, 0, 0, 0, 0};
	int total_by_level[5] = {0, 0, 0, 0, 0};

	for (const auto& result : results) {
		for (int i = 0; i < 5; ++i) {
			total_by_level[i]++;
			if (result.passed[i]) {
				passed_by_level[i]++;
			}
		}
	}

	std::cout << "Functions tested: " << results.size() << "\n\n";

	for (int i = 0; i < 5; ++i) {
		std::cout << MAXLIMBS_LABELS[i] << " : "
		          << passed_by_level[i] << "/" << total_by_level[i] << " passed";
		if (passed_by_level[i] == total_by_level[i]) {
			std::cout << " ✓\n";
		} else {
			std::cout << " ✗ FAILURES DETECTED\n";
		}
	}

	// Overall pass/fail
	bool all_passed = true;
	for (int i = 0; i < 5; ++i) {
		if (passed_by_level[i] != total_by_level[i]) {
			all_passed = false;
			break;
		}
	}

	std::cout << "\n";
	if (all_passed) {
		std::cout << "Progressive precision validation: PASS\n";
		std::cout << "All functions achieve expected precision scaling with maxlimbs.\n";
		return EXIT_SUCCESS;
	} else {
		std::cout << "Progressive precision validation: FAIL\n";
		std::cout << "Some functions do not achieve expected precision scaling.\n";
		return EXIT_FAILURE;
	}
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
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
