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
#include <concepts>
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

// The functions accept only the quire's own element type and exclude the
// native quires, so they can't claim an unrelated accumulator.
int VerifyProtocolConstraints() {
	using namespace sw::universal;
	using P16 = posit<16, 1>;
	using P32 = posit<32, 2>;
	static_assert(adl_accumulator<quire<P16>, P16>);
	static_assert(!adl_accumulator<quire<P16>, P32>);
	static_assert(!adl_accumulator<quire<P16>, double>);
	static_assert(!adl_accumulator<double, P16>);
	static_assert(!adl_accumulator<quire<float>, float>);
	static_assert(!adl_accumulator<quire<double>, double>);
	return 0;
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
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<posit<16, 1>>(4096.0, true, "posit<16,1>"), "quire<posit<16,1>>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<posit<32, 2>>(1048576.0 * 1024.0 * 1024.0, true, "posit<32,2>"), "quire<posit<32,2>>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<bposit<16, 6, 5, std::uint16_t>>(4096.0, true, "bposit<16,6,5>"), "quire<bposit<16,6,5>>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<cfloat<16, 5, uint16_t, true, false, false>>(4096.0, true, "cfloat<16,5>"), "quire<cfloat<16,5>>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<cfloat<32, 8, uint32_t, true, false, false>>(134217728.0, true, "cfloat<32,8>"), "quire<cfloat<32,8>>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<fixpnt<16, 8, Modulo, uint16_t>>(64.0, false, "fixpnt<16,8>"), "quire<fixpnt<16,8>>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<lns<16, 8, uint16_t>>(1024.0, true, "lns<16,8>"), "quire<lns<16,8>>", "accumulator protocol");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulatorProtocol<dbns<8, 4, uint8_t>>(16.0, true, "dbns<8,4>"), "quire<dbns<8,4>>", "accumulator protocol");

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
