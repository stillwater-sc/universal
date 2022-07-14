// bheavior.cpp: test suite runner for arithmetic behavior experiment
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// minimum set of include files to reflect source code dependencies
#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/behavior/arithmetic.hpp>

namespace sw { namespace universal {

	template<size_t _nbits, size_t _rbits, ArithmeticBehavior behavior, typename bt>
	class lns2 {
	public:
		static constexpr size_t   nbits = _nbits;
		static constexpr size_t   rbits = _rbits;
		typedef bt BlockType;
		static constexpr double   scaling = double(1ull << rbits);
		static constexpr size_t   bitsInByte = 8ull;
		static constexpr size_t   bitsInBlock = sizeof(bt) * bitsInByte;
		static constexpr size_t   nrBlocks = (1 + ((nbits - 1) / bitsInBlock));

		lns2() = default;

		lns2& operator=(int rhs) {
			_block[0] = static_cast<BlockType>(rhs);
			return *this;
		}

		explicit operator double() const noexcept {
			return double(_block[0]);
		}

		BlockType block(unsigned i) const noexcept {
			return _block[0];
		}
	protected:

	private:
		BlockType _block[nrBlocks];

		template<size_t nnbits, size_t rrbits, ArithmeticBehavior bbehave, typename nbt>
		friend std::ostream& operator<< (std::ostream& ostr, const lns2<nnbits, rrbits, bbehave, nbt>& r);

	};

	template<size_t nnbits, size_t rrbits, ArithmeticBehavior bbehave, typename nbt>
	std::ostream& operator<< (std::ostream& ostr, const lns2<nnbits, rrbits, bbehave, nbt>& r) {
		return ostr << double(r);
	}

	template<size_t nnbits, size_t rrbits, ArithmeticBehavior bbehave, typename nbt>
	std::string to_binary(const lns2<nnbits, rrbits, bbehave, nbt>& r) {
		std::stringstream s;
		s << to_binary(r.block(0));
		return s.str();
	}

} }  // namespace sw::universal


// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "lns arithmetic behavior validation";
	std::string test_tag    = "arithmetic behavior";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		using Real = lns2<8, 2, Modular, std::uint8_t>;
		bool isTrivial = bool(std::is_trivial<Real>());
		static_assert(std::is_trivial<Real>(), "lns should be trivial but failed the assertion");
		std::cout << (isTrivial ? "lns is trivial" : "lns failed trivial: FAIL") << '\n';

		bool isTriviallyConstructible = bool(std::is_trivially_constructible<Real>());
		static_assert(std::is_trivially_constructible<Real>(), "lns should be trivially constructible but failed the assertion");
		std::cout << (isTriviallyConstructible ? "lns is trivial constructible" : "lns failed trivial constructible: FAIL") << '\n';

		bool isTriviallyCopyable = bool(std::is_trivially_copyable<Real>());
		static_assert(std::is_trivially_copyable<Real>(), "lns should be trivially copyable but failed the assertion");
		std::cout << (isTriviallyCopyable ? "lns is trivially copyable" : "lns failed trivially copyable: FAIL") << '\n';

		bool isTriviallyCopyAssignable = bool(std::is_trivially_copy_assignable<Real>());
		static_assert(std::is_trivially_copy_assignable<Real>(), "lns should be trivially copy-assignable but failed the assertion");
		std::cout << (isTriviallyCopyAssignable ? "lns is trivially copy-assignable" : "lns failed trivially copy-assignable: FAIL") << '\n';
	}

	{
		using Real = lns2<8, 2, Modular, std::uint8_t>;
		Real a, b, c;
		a = 1;
		b = 2;
		c = 3;
		ReportValues(a, "+", b, c);
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
