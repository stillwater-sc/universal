// accumulator_protocol.cpp: the quire as a pluggable super-accumulator
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The accumulator_* free functions let a linear-algebra library (MTL5's
// mtl::math::accumulator_traits) accumulate in a quire without either library
// including the other. The library finds them by argument-dependent lookup, so
// these tests call them the same way: unqualified, from outside sw::universal.
//
// Relates to stillwater-sc/mtl5#582
#include <universal/utility/directives.hpp>
#include <cmath>
#include <concepts>
#include <initializer_list>
#include <limits>
#include <utility>
#include <type_traits>
#include <universal/number/posit/posit.hpp>
#include <universal/number/bposit/bposit.hpp>
#include <universal/number/bposit/fdp.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/fdp.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/lns/fdp.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/dbns/fdp.hpp>
#include <universal/number/quire/quire.hpp>
#include <universal/verification/test_suite.hpp>

namespace client {

// The protocol as a client library checks it (MTL5's adl_accumulator).
template<typename Acc, typename Value>
concept adl_accumulator = requires(Acc& a, const Acc& ca, const Value& v) {
	accumulator_clear(a);
	accumulator_assign(a, v);
	accumulator_add_product(a, v, v);
	{ accumulator_value(ca, std::type_identity<Value>{}) } -> std::convertible_to<Value>;
};

template<typename Acc>
concept has_merge = requires(Acc& a, const Acc& b) { accumulator_merge(a, b); };

// Each operation through the protocol, for one element type. `big` is a power
// of two, so it's exact in every number system here; for the floating and
// tapered types the spacing at `big` exceeds 1, so 1 + big - big loses the 1
// unless the sum is exact.
template<typename Scalar>
int VerifyAccumulatorProtocol(double big, bool cancels, const std::string& tag) {
	using sw::universal::quire;
	using Quire = quire<Scalar>;
	int nrOfFailedTestCases = 0;

	static_assert(adl_accumulator<Quire, Scalar>);
	static_assert(has_merge<Quire>);

	const Scalar one(1), b(big), mb(-big);

	// clear / assign / value
	Quire q;
	accumulator_assign(q, Scalar(0.75));
	if (accumulator_value(q, std::type_identity<Scalar>{}) != Scalar(0.75)) {
		std::cerr << "FAIL: " << tag << " assign/value round trip\n";
		++nrOfFailedTestCases;
	}
	accumulator_clear(q);
	if (!q.iszero()) {
		std::cerr << "FAIL: " << tag << " clear\n";
		++nrOfFailedTestCases;
	}

	// add_product: 1*1 + big*1 + (-big)*1 is exactly 1
	accumulator_assign(q, one);
	accumulator_add_product(q, b, one);
	accumulator_add_product(q, mb, one);
	if (accumulator_value(q, std::type_identity<Scalar>{}) != one) {
		std::cerr << "FAIL: " << tag << " exact cancellation: got "
		          << accumulator_value(q, std::type_identity<Scalar>{}) << '\n';
		++nrOfFailedTestCases;
	}
	if (accumulator_value(q, std::type_identity<double>{}) != 1.0) {
		std::cerr << "FAIL: " << tag << " value<double>\n";
		++nrOfFailedTestCases;
	}
	// ...which the element arithmetic alone does not deliver
	if (cancels && Scalar((one + b) + mb) == one) {
		std::cerr << "FAIL: " << tag << " fixture does not cancel in element arithmetic\n";
		++nrOfFailedTestCases;
	}

	// merge: the same sum split across two partial quires
	Quire lo, hi;
	accumulator_clear(lo);
	accumulator_clear(hi);
	accumulator_add_product(lo, one, one);
	accumulator_add_product(lo, b, one);
	accumulator_add_product(hi, mb, one);
	accumulator_merge(lo, hi);
	if (accumulator_value(lo, std::type_identity<Scalar>{}) != one) {
		std::cerr << "FAIL: " << tag << " merge\n";
		++nrOfFailedTestCases;
	}
	return nrOfFailedTestCases;
}

// The functions accept only the quire's own element type, so they can't claim
// an unrelated accumulator. The native quires get the protocol from
// native/fdp.hpp, under the same rule.
int VerifyProtocolConstraints() {
	using namespace sw::universal;
	using P16 = posit<16, 1>;
	using P32 = posit<32, 2>;
	static_assert(adl_accumulator<quire<P16>, P16>);
	static_assert(!adl_accumulator<quire<P16>, P32>);
	static_assert(!adl_accumulator<quire<P16>, double>);
	static_assert(!adl_accumulator<double, P16>);
	static_assert(adl_accumulator<quire<float>, float>);
	static_assert(adl_accumulator<quire<double>, double>);
	static_assert(has_merge<quire<float>> && has_merge<quire<double>>);
	static_assert(!adl_accumulator<quire<float>, double>);
	static_assert(!adl_accumulator<quire<double>, float>);
	static_assert(!adl_accumulator<quire<double>, P16>);
	static_assert(!adl_accumulator<quire<P16>, float>);
	return 0;
}

// accumulator_value to float and double rounds to nearest even from the full
// quire. convert_to<>() keeps only the leading 53 bits, so these sums, whose
// deciding bits lie further down, are where the two differ.
int VerifyRoundOut() {
	using namespace sw::universal;
	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;
	using Quire = quire<Scalar>;
	int nrOfFailedTestCases = 0;

	auto p2 = [](int e) { return Scalar(std::ldexp(1.0, e)); };
	// a quire holding the exact sum of the products a[i] * b[i]
	auto sum = [](std::initializer_list<std::pair<Scalar, Scalar>> terms) {
		Quire q;
		accumulator_clear(q);
		for (const auto& [a, b] : terms) accumulator_add_product(q, a, b);
		return q;
	};
	auto check = [&](const char* what, auto got, auto expected) {
		if (!(got == expected) || std::signbit(got) != std::signbit(expected)) {
			std::cerr << "FAIL: " << what << ": got " << got << ", expected " << expected << '\n';
			++nrOfFailedTestCases;
		}
	};
	constexpr std::type_identity<double> to_double{};
	constexpr std::type_identity<float>  to_float{};
	const Scalar one(1), three(3);

	// 1 + 3*2^-54: nearest double is 1 + 2^-52; truncation gives 1
	Quire q = sum({ {one, one}, {three * p2(-27), p2(-27)} });
	check("1 + 3*2^-54 to double", accumulator_value(q, to_double), 1.0 + std::ldexp(1.0, -52));
	if (q.convert_to<double>() == accumulator_value(q, to_double)) {
		std::cerr << "FAIL: fixture does not distinguish rounding from truncation\n";
		++nrOfFailedTestCases;
	}
	// a tie goes to even; any bit below the tie breaks it
	check("1 + 2^-53 to double", accumulator_value(sum({ {one, one}, {p2(-27), p2(-26)} }), to_double), 1.0);
	check("1 + 2^-53 + 2^-80 to double",
	      accumulator_value(sum({ {one, one}, {p2(-27), p2(-26)}, {p2(-40), p2(-40)} }), to_double),
	      1.0 + std::ldexp(1.0, -52));
	check("1 + 2^-52 + 2^-53 to double",
	      accumulator_value(sum({ {one, one}, {p2(-26), p2(-26)}, {p2(-27), p2(-26)} }), to_double),
	      1.0 + std::ldexp(1.0, -51));
	// sign
	check("-(1 + 3*2^-54) to double",
	      accumulator_value(sum({ {-one, one}, {-three * p2(-27), p2(-27)} }), to_double),
	      -(1.0 + std::ldexp(1.0, -52)));
	// float: through double, 1 + 2^-24 + 2^-60 would become the tie 1 + 2^-24 and round to 1
	check("1 + 2^-24 + 2^-60 to float",
	      accumulator_value(sum({ {one, one}, {p2(-12), p2(-12)}, {p2(-30), p2(-30)} }), to_float),
	      1.0f + std::ldexp(1.0f, -23));
	// float gradual underflow: spacing 2^-149
	check("1.5 * 2^-149 to float", accumulator_value(sum({ {Scalar(1.5) * p2(-75), p2(-74)} }), to_float),
	      std::ldexp(1.0f, -148));
	check("2^-150 to float", accumulator_value(sum({ {p2(-75), p2(-75)} }), to_float), 0.0f);
	check("2^-150 + 2^-170 to float", accumulator_value(sum({ {p2(-75), p2(-75)}, {p2(-85), p2(-85)} }), to_float),
	      std::ldexp(1.0f, -149));
	check("-2^-150 to float", accumulator_value(sum({ {-p2(-75), p2(-75)} }), to_float), -0.0f);
	// beyond float's range, within double's
	check("2^200 to float", accumulator_value(sum({ {p2(100), p2(100)} }), to_float),
	      std::numeric_limits<float>::infinity());
	check("2^200 to double", accumulator_value(sum({ {p2(100), p2(100)} }), to_double), std::ldexp(1.0, 200));
	// a 64-bit significand (x87 long double): 2 - 2^-64 is 64 ones and a guard
	// bit, so rounding up carries out of the 64-bit integer to 2
	if constexpr (std::numeric_limits<long double>::digits == 64) {
		Quire c = sum({ {Scalar(2), one}, {-p2(-32), p2(-32)} });
		check("2 - 2^-64 to long double", quire_round_to<long double>(c), 2.0L);
	}
	// zero
	Quire z;
	accumulator_clear(z);
	check("0 to double", accumulator_value(z, to_double), 0.0);
	return nrOfFailedTestCases;
}

// quire<float> and quire<double>: the products are exact, subnormals
// included, and the round-out reads the whole quire.
template<typename Real>
int VerifyNativeRoundOut(const std::string& tag) {
	using sw::universal::quire;
	using Quire = quire<Real>;
	int nrOfFailedTestCases = 0;
	constexpr int digits = std::numeric_limits<Real>::digits;   // 24 or 53
	constexpr std::type_identity<Real> to_real{};

	auto sum = [](std::initializer_list<std::pair<Real, Real>> terms) {
		Quire q;
		accumulator_clear(q);
		for (const auto& [a, b] : terms) accumulator_add_product(q, a, b);
		return q;
	};
	auto check = [&](const char* what, Real got, Real expected) {
		if (!(got == expected) || std::signbit(got) != std::signbit(expected)) {
			std::cerr << "FAIL: " << tag << ' ' << what << ": got " << std::hexfloat << got
			          << ", expected " << expected << std::defaultfloat << '\n';
			++nrOfFailedTestCases;
		}
	};
	auto p2 = [](int e) { return std::ldexp(Real(1), e); };
	const Real one(1);

	// (1 + u)(1 - u) = 1 - u^2 with u = 2^-(digits-1): the element arithmetic
	// rounds the product to 1, the quire keeps it and the final -1 leaves -u^2
	const Real u = p2(1 - digits);
	check("(1+u)(1-u) - 1", accumulator_value(sum({ {one + u, one - u}, {-one, one} }), to_real), -u * u);
	if ((one + u) * (one - u) - one != Real(0)) {
		std::cerr << "FAIL: " << tag << " fixture does not cancel in element arithmetic\n";
		++nrOfFailedTestCases;
	}
	// a tie goes to even; a bit far below the tie breaks it
	const Real half_ulp = p2(-digits);
	check("1 + half ulp", accumulator_value(sum({ {one, one}, {half_ulp, one} }), to_real), one);
	check("1 + half ulp + tiny",
	      accumulator_value(sum({ {one, one}, {half_ulp, one}, {p2(-60), p2(-60)} }), to_real), one + u);
	// quire_resolve rounds as accumulator_value does, not truncating like convert_to
	const Quire t = sum({ {one, one}, {half_ulp, one}, {p2(-60), p2(-60)} });
	check("quire_resolve", quire_resolve(t), one + u);

	// subnormal operands and a product below the smallest subnormal
	const Real dmin = std::numeric_limits<Real>::denorm_min();
	check("denorm_min * 3", accumulator_value(sum({ {dmin, Real(3)} }), to_real), Real(3) * dmin);
	check("denorm_min^2", accumulator_value(sum({ {dmin, dmin} }), to_real), Real(0));
	check("denorm_min/2 + denorm_min^2", accumulator_value(sum({ {dmin, dmin}, {dmin, Real(0.5)} }), to_real), dmin);
	// the top of the range: max * 2 - max * 1 is max, though max * 2 overflows
	const Real mx = std::numeric_limits<Real>::max();
	check("max*2 - max", accumulator_value(sum({ {mx, Real(2)}, {-mx, one} }), to_real), mx);
	check("max*2", accumulator_value(sum({ {mx, Real(2)} }), to_real), std::numeric_limits<Real>::infinity());

	// a NaN or infinite operand makes the quire NaR
	Quire n = sum({ {one, one}, {std::numeric_limits<Real>::infinity(), one} });
	if (!std::isnan(accumulator_value(n, to_real))) {
		std::cerr << "FAIL: " << tag << " infinite operand does not give NaN\n";
		++nrOfFailedTestCases;
	}

	// merge: the cancelling sum split over two partials
	Quire lo = sum({ {one + u, one - u} }), hi = sum({ {-one, one} });
	accumulator_merge(lo, hi);
	check("merge", accumulator_value(lo, to_real), -u * u);
	return nrOfFailedTestCases;
}

} // namespace client

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "quire accumulator protocol";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

	using client::VerifyAccumulatorProtocol;
	nrOfFailedTestCases += ReportTestResult(client::VerifyProtocolConstraints(), "quire", "protocol constraints");
	nrOfFailedTestCases += ReportTestResult(client::VerifyRoundOut(), "quire<cfloat<32,8>>", "round-out to float/double");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<posit<16, 1>>(4096.0, true, "posit<16,1>"), "quire<posit<16,1>>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<posit<32, 2>>(1048576.0 * 1024.0 * 1024.0, true, "posit<32,2>"), "quire<posit<32,2>>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<bposit<16, 6, 5, std::uint16_t>>(4096.0, true, "bposit<16,6,5>"), "quire<bposit<16,6,5>>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<cfloat<16, 5, uint16_t, true, false, false>>(4096.0, true, "cfloat<16,5>"), "quire<cfloat<16,5>>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<cfloat<32, 8, uint32_t, true, false, false>>(134217728.0, true, "cfloat<32,8>"), "quire<cfloat<32,8>>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<fixpnt<16, 8, Modulo, uint16_t>>(64.0, false, "fixpnt<16,8>"), "quire<fixpnt<16,8>>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<lns<16, 8, uint16_t>>(1024.0, true, "lns<16,8>"), "quire<lns<16,8>>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<dbns<8, 4, uint8_t>>(16.0, true, "dbns<8,4>"), "quire<dbns<8,4>>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<float>(33554432.0, true, "float"), "quire<float>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<double>(18014398509481984.0, true, "double"), "quire<double>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(client::VerifyNativeRoundOut<float>("float"), "quire<float>", "exact products, round-out");
	nrOfFailedTestCases += ReportTestResult(client::VerifyNativeRoundOut<double>("double"), "quire<double>", "exact products, round-out");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
