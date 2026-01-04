// constexpr.cpp: compile time tests for blocktriple constexpr
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>
#include <iostream>
#include <iomanip>
#include <cstdint>

// minimum set of include files to reflect source code dependencies
#include <universal/internal/blocktriple/blocktriple.hpp>

namespace sw::experiment {
	template<size_t nbits, typename bt = uint32_t>
	class blocktriple {
	public:
		static constexpr size_t bitsInByte = 8;
		static constexpr size_t bitsInBlock = sizeof(bt) * bitsInByte;

		static constexpr size_t nrBlocks = 1ull + ((nbits - 1ull) / bitsInBlock);

		blocktriple(const blocktriple&) noexcept = default;
		blocktriple(blocktriple&&) noexcept = default;

		blocktriple& operator=(const blocktriple&) noexcept = default;
		blocktriple& operator=(blocktriple&&) noexcept = default;

		constexpr blocktriple() noexcept : _block{ 0 } {}
		constexpr blocktriple(int iv) noexcept : _block{ 0 } { *this = iv; }
		constexpr blocktriple& operator=(int rhs) noexcept {
			return convert_signed_integer(rhs);
		}
		template<typename Ty>
		constexpr blocktriple& convert_signed_integer(const Ty& rhs) noexcept {
			//_scale = 1;
			return *this;
		}
	private:
		bt _block[nrBlocks];

		// template parameters need names different from class template parameters (for gcc and clang)
		template<size_t sbits, typename bbt>
		friend std::ostream& operator<< (std::ostream& ostr, const blocktriple<sbits, bbt>& a);

	};

	template<size_t significantbits, typename bt>
	inline std::ostream& operator<<(std::ostream& ostr, const blocktriple<significantbits, bt>& a) {
		ostr << a._scale;
		return ostr;
	}
} // namespace sw::experiment

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::cout << "blocktriple constexpr tests\n";
	
	{
		// decorated constructors
		{
			// this will round-up to 16 due to the fact that we only have 3 bits of significant
			CONSTEXPRESSION blocktriple<3> a(15);  // signed long
			std::cout << "constexpr constructor for type 'int'                 " << a << '\n';
		}
		{
			// this will stay 15 due to the fact that we have 4 bits of significant
			CONSTEXPRESSION blocktriple<4> a(15);  // signed long
			std::cout << "constexpr constructor for type 'int'                 " << a << '\n';
		}
		{
			CONSTEXPRESSION blocktriple<32> a(2);  // signed long
			std::cout << "constexpr constructor for type 'int'                 " << a << '\n';
		}
		{
			CONSTEXPRESSION blocktriple<32> a(4ll);  // long long
			std::cout << "constexpr constructor for type 'long long'           " << a << '\n';
		}
		{
			CONSTEXPRESSION blocktriple<32> a(8ul);  // unsigned long
			std::cout << "constexpr constructor for type 'unsigned long'       " << a << '\n';
		}
		{
			CONSTEXPRESSION blocktriple<32> a(16ull);  // unsigned long
			std::cout << "constexpr constructor for type 'unsigned long long'  " << a << '\n';
		}
#if BIT_CAST_IS_CONSTEXPR
		{
			CONSTEXPRESSION blocktriple<32> a(1.125f);  // float
			std::cout << "constexpr constructor for type 'float'               " << a << '\n';
		}
		{
			CONSTEXPRESSION blocktriple<32> a(1.0625);   // double
			std::cout << "constexpr constructor for type 'double'              " << a << '\n';
		}
#if LONG_DOUBLE_SUPPORT
		{
			blocktriple<32> a(1.03125l);  // long double
			std::cout << "constexpr constructor for type 'long double'         " << a << '\n';
		}
#endif

#endif // BIT_CAST_IS_CONSTEXPR
	}

	{
		// assignment operators
		{
			CONSTEXPRESSION blocktriple<32> a = 1l;  // signed long
			std::cout << a << '\n';
		}
		{
			CONSTEXPRESSION blocktriple<32> a = 1ul;  // unsigned long
			std::cout << a << '\n';
		}
#if BIT_CAST_IS_CONSTEXPR
		{
			CONSTEXPRESSION blocktriple<32> a = 1.0f;  // float
			std::cout << a << '\n';
		}
		{
			CONSTEXPRESSION blocktriple<32> a = 1.0;   // double
			std::cout << a << '\n';
		}
#if LONG_DOUBLE_SUPPORT
		{
			blocktriple<32> a = 1.0l;  // long double
			std::cout << a << '\n';
		}
#endif

#endif // BIT_CAST_IS_CONSTEXPR
	}

	if (nrOfFailedTestCases > 0) {
		std::cout << "FAIL\n";
	}
	else {
		std::cout << "PASS\n";
	}
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
