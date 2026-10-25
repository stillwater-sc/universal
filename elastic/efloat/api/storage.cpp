// storage.cpp: efloat's significand storage (#1820)
//
//   allocations  c = a op b and c op= b allocate nothing in a steady state: a value up to the
//                inline limbs never does, from the first operation on, and past them the heap
//                blocks come back from the thread's cache. A replacement operator new counts
//                every allocation the test program makes.
//   copies       copies, moves and assignments between values held inline and on the heap,
//                self-assignment and self-move-assignment, a moved-from value assigned again
//                and moved into, and a std::vector of values that grows.
//   limb views   bits(), limb_vector() and setlimb() show 32-bit limbs, most significant first,
//                as efloat held them before #1820; limb_data() and limb_count() the 64-bit limbs,
//                least significant first, that it holds now; normalize() drops a bottom limb the
//                normalizing shift leaves empty. A parsed literal is normalized like
//                any other value, so "1.5" equals efloat(1.5) (before, it kept zero limbs at the
//                bottom and compared neither equal, less nor greater).
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- the cases above, at 128 to 8192 bits.
//   LEVEL 2 -- more operations counted in the steady state, and more random copies.
//   LEVEL 3 -- as level 2, longer.
//   LEVEL 4 -- as level 3, longer.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <new>
#include <random>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

// every allocation the program makes, through any of the replaceable operators new. gcc reads
// the replaced operators' malloc and free as a mismatch with new and delete, which they are not.
#if defined(__GNUC__) && !defined(__clang__)
#	pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#endif
static unsigned long long allocations = 0;

void* operator new(std::size_t n) {
	++allocations;
	if (void* p = std::malloc(n != 0 ? n : 1)) return p;
	throw std::bad_alloc();
}
void* operator new[](std::size_t n) {
	++allocations;
	if (void* p = std::malloc(n != 0 ? n : 1)) return p;
	throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

namespace {

	// a value of `bits` bits, all of them random, its leading bit set, at the given scale
	template<unsigned nlimbs>
	sw::universal::efloat<nlimbs> random_value(std::mt19937_64& rng, unsigned bits, int64_t scale, bool negative = false) {
		sw::universal::efloat<nlimbs> v(1);
		v.set_precision(bits);
		std::vector<std::uint64_t> w((bits + 63) / 64);
		for (auto& limb : w) limb = rng();
		w.back() |= std::uint64_t(1) << 63;
		w.front() |= 1u;
		v.set_significand(w.data(), w.size());
		v.setexponent(scale);
		v.setsign(negative);
		v.set_precision(bits);   // normalizes, rounding to the precision
		return v;
	}

	template<unsigned nlimbs>
	using operation = std::function<void(sw::universal::efloat<nlimbs>&, const sw::universal::efloat<nlimbs>&, const sw::universal::efloat<nlimbs>&)>;

	template<unsigned nlimbs>
	std::vector<std::pair<const char*, operation<nlimbs>>> operations() {
		using E = sw::universal::efloat<nlimbs>;
		return {
			{ "c = a + b", [](E& c, const E& a, const E& b) { c = a + b; } },
			{ "c = a - b", [](E& c, const E& a, const E& b) { c = a - b; } },
			{ "c = a * b", [](E& c, const E& a, const E& b) { c = a * b; } },
			{ "c = a / b", [](E& c, const E& a, const E& b) { c = a / b; } },
			{ "c = a * a", [](E& c, const E& a, const E&)   { c = a * a; } },
			{ "c = a / 7", [](E& c, const E& a, const E&)   { c = a / 7; } },
			{ "c += b",    [](E& c, const E& a, const E& b) { c = a; c += b; } },
			{ "c -= b",    [](E& c, const E& a, const E& b) { c = a; c -= b; } },
			{ "c *= b",    [](E& c, const E& a, const E& b) { c = a; c *= b; } },
			{ "c /= b",    [](E& c, const E& a, const E& b) { c = a; c /= b; } },
			{ "c = a",     [](E& c, const E& a, const E&)   { c = a; } },
		};
	}

	// values up to the inline limbs allocate nothing, from the first operation on
	int VerifyInlineValuesDoNotAllocate(bool reportTestCases) {
		using namespace sw::universal;
		using E = efloat<4>;   // 128 bits
		int nrOfFailedTestCases = 0;
		static_assert(E::inline_limbs >= 2, "a 128-bit value fits the inline limbs");
		std::mt19937_64 rng(1820);
		const E a = random_value<4>(rng, 128, 3);
		const E b = random_value<4>(rng, 128, -2, true);
		for (const auto& [name, op] : operations<4>()) {
			const unsigned long long before = allocations;
			E c;
			for (int i = 0; i < 4; ++i) op(c, a, b);
			const unsigned long long made = allocations - before;
			if (made != 0) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "FAIL: " << name << " at 128 bits made " << made << " allocations\n";
			}
		}
		return nrOfFailedTestCases;
	}

	// past the inline limbs, the heap blocks come back from the thread's cache: once a loop of
	// operations has run, it allocates nothing more
	int VerifySteadyStateAllocations(bool reportTestCases, int repetitions) {
		using namespace sw::universal;
		using E = efloat<256>;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(18201);
		for (unsigned bits : { 128u, 512u, 2048u, 8192u }) {
			const E a = random_value<256>(rng, bits, 1);
			const E b = random_value<256>(rng, bits, -1, true);
			for (const auto& [name, op] : operations<256>()) {
				E c;
				for (int i = 0; i < 3; ++i) op(c, a, b);   // the cache fills
				const unsigned long long before = allocations;
				for (int i = 0; i < repetitions; ++i) op(c, a, b);
				const unsigned long long made = allocations - before;
				if (made != 0) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cout << "FAIL: " << name << " at " << bits << " bits made " << made << " allocations in " << repetitions << " repetitions\n";
				}
			}
			// a square root's working limbs come from the cache too
			E r;
			for (int i = 0; i < 3; ++i) r = sqrt(a);
			const unsigned long long before = allocations;
			for (int i = 0; i < repetitions; ++i) r = sqrt(a);
			if (allocations != before) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "FAIL: sqrt at " << bits << " bits made " << (allocations - before) << " allocations\n";
			}
		}
		return nrOfFailedTestCases;
	}

	template<unsigned nlimbs>
	bool identical(const sw::universal::efloat<nlimbs>& a, const sw::universal::efloat<nlimbs>& b) {
		return a.sign() == b.sign() && a.scale() == b.scale() && a.bits() == b.bits() && a.get_precision() == b.get_precision()
		    && a.iszero() == b.iszero() && a.isinf() == b.isinf() && a.isnan() == b.isnan();
	}

	// copies, moves and assignments between inline and heap values keep the value
	int VerifyCopiesAndMoves(bool reportTestCases, int samples) {
		using namespace sw::universal;
		using E = efloat<256>;
		int nrOfFailedTestCases = 0;
		auto check = [&](bool ok, const char* what) {
			if (!ok) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "FAIL: " << what << '\n';
			}
		};
		std::mt19937_64 rng(18202);
		const unsigned sizes[] = { 32, 64, 96, 128, 448, 512, 544, 1024, 4096, 8192 };   // inline up to 512 bits
		for (int s = 0; s < samples; ++s) {
			const unsigned bits_x = sizes[rng() % 10], bits_y = sizes[rng() % 10];
			const E x = random_value<256>(rng, bits_x, static_cast<int64_t>(rng() % 40) - 20, (rng() & 1u) != 0u);
			const E y = random_value<256>(rng, bits_y, static_cast<int64_t>(rng() % 40) - 20, (rng() & 1u) != 0u);
			E c(x);
			check(identical(c, x) && c == x, "a copy equals its source");
			E m(std::move(c));
			check(identical(m, x), "a move keeps the value");
			c = y;   // a moved-from value can be assigned again
			check(identical(c, y), "a moved-from value takes a new value");
			c = x;   // into a value that holds a different length
			check(identical(c, x), "a copy assignment across lengths");
			E t(y);
			c = std::move(t);
			check(identical(c, y), "a move assignment across lengths");
			const E& same = c;   // through a reference: the compilers warn of a literal c = c
			c = same;
			check(identical(c, y), "self-assignment");
			E& self = c;   // a move onto itself keeps the value, inline or on the heap (#1830)
			c = std::move(self);
			check(identical(c, y), "self-move-assignment");
			t = x;   // a moved-from value takes a new value, of any length
			check(identical(t, x), "a moved-from value is assigned again");
			E h(y);
			h = std::move(t);   // into a value that holds a heap block or inline limbs
			check(identical(h, x), "a move assignment into a value of the other length");
			t = std::move(h);   // and back into the moved-from value
			check(identical(t, x), "a move assignment into a moved-from value");
			E sum = x + y, again = x;
			again += y;
			check(identical(sum, again), "x + y and x += y agree");
		}
		std::vector<E> values;
		std::vector<E> expected;
		for (int i = 0; i < 64; ++i) {
			const E v = random_value<256>(rng, sizes[i % 10], i);
			values.push_back(v);   // grows, moving the values
			expected.push_back(v);
		}
		bool kept = true;
		for (std::size_t i = 0; i < values.size(); ++i) kept = kept && identical(values[i], expected[i]);
		check(kept, "a std::vector of values keeps them as it grows");
		return nrOfFailedTestCases;
	}

	// the 32-bit view the API shows, and the 64-bit limbs underneath
	int VerifyLimbViews(bool reportTestCases) {
		using namespace sw::universal;
		using E = efloat<8>;
		int nrOfFailedTestCases = 0;
		auto check = [&](bool ok, const char* what) {
			if (!ok) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "FAIL: " << what << '\n';
			}
		};
		const E one_and_a_half(1.5);
		check(one_and_a_half.bits() == std::vector<uint32_t>{ 0xC000'0000u }, "efloat(1.5) has one 32-bit limb");
		check(one_and_a_half.limb_count() == 1 && one_and_a_half.limb_data()[0] == 0xC000'0000'0000'0000ull, "and one 64-bit limb, its leading bit at the top");
		check(one_and_a_half.limb32_count() == 1 && one_and_a_half.limb_vector() == one_and_a_half.bits(), "limb_vector() is bits()");

		const E v(false, 5, std::vector<uint32_t>{ 0x8000'0000u, 0x1u, 0x2u });
		check(v.bits() == std::vector<uint32_t>{ 0x8000'0000u, 0x1u, 0x2u } && v.scale() == 5, "three 32-bit limbs read back as written");
		check(v.limb_count() == 2 && v.limb_data()[1] == 0x8000'0000'0000'0001ull && v.limb_data()[0] == 0x0000'0002'0000'0000ull,
		      "and are held as two 64-bit limbs, the odd one padded at the bottom");

		E w(1.5);
		w.setlimb(3, 0x1234'5678u);   // grows at the bottom: the leading limb keeps its weight
		check(w.bits() == std::vector<uint32_t>{ 0xC000'0000u, 0u, 0u, 0x1234'5678u } && w.scale() == 0, "setlimb past the end grows the significand at the bottom");
		w.setlimb(1, 0xFFFF'FFFFu);
		check(w.bits() == std::vector<uint32_t>{ 0xC000'0000u, 0xFFFF'FFFFu, 0u, 0x1234'5678u }, "setlimb inside the significand");

		// a significand whose bottom limb's bits all move into the limb above when the leading bit
		// moves to the top: normalize() must drop the empty limb
		E n(1);
		const std::uint64_t two[2] = { std::uint64_t(1) << 63, 1u };
		n.set_significand(two, 2);
		n.setexponent(63);   // the top limb's bit 63 weighs 2^63, its bit 0 2^0
		n.normalize();
		check(n == one_and_a_half && n.limb_count() == 1 && n.bits() == one_and_a_half.bits(), "normalize() drops a bottom limb left empty by the shift");

		E p;
		parse<1024>("1.5", p);
		check(p == one_and_a_half && p.bits() == one_and_a_half.bits(), "a parsed 1.5 is efloat(1.5)");
		E tenth;
		parse<1024>("0.1", tenth);
		check(!tenth.bits().empty() && tenth.bits().back() != 0u && tenth == E(1) / E(10), "a parsed 0.1 has no zero limb at the bottom, and is 1/10");
		E zero;
		check(zero.bits() == std::vector<uint32_t>{ 0u } && zero.iszero(), "zero holds one limb of 0");
		E inf;
		inf.setinf();
		check(inf.bits().empty() && inf.limb_count() == 0, "inf holds no limbs");
		return nrOfFailedTestCases;
	}

}  // namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat significand storage";
	std::string test_tag            = "storage";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyLimbViews(reportTestCases), "efloat", "limb views");
	nrOfFailedTestCases += ReportTestResult(VerifySteadyStateAllocations(reportTestCases, 4), "efloat", "steady-state allocations");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyInlineValuesDoNotAllocate(reportTestCases), "efloat", "inline values do not allocate");
	nrOfFailedTestCases += ReportTestResult(VerifySteadyStateAllocations(reportTestCases, 10), "efloat", "steady-state allocations");
	nrOfFailedTestCases += ReportTestResult(VerifyCopiesAndMoves(reportTestCases, 200), "efloat", "copies and moves");
	nrOfFailedTestCases += ReportTestResult(VerifyLimbViews(reportTestCases), "efloat", "limb views");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifySteadyStateAllocations(reportTestCases, 100), "efloat", "steady-state allocations");
	nrOfFailedTestCases += ReportTestResult(VerifyCopiesAndMoves(reportTestCases, 2000), "efloat", "copies and moves");
#	endif

#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifySteadyStateAllocations(reportTestCases, 1000), "efloat", "steady-state allocations");
	nrOfFailedTestCases += ReportTestResult(VerifyCopiesAndMoves(reportTestCases, 20000), "efloat", "copies and moves");
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifySteadyStateAllocations(reportTestCases, 10000), "efloat", "steady-state allocations");
	nrOfFailedTestCases += ReportTestResult(VerifyCopiesAndMoves(reportTestCases, 200000), "efloat", "copies and moves");
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
