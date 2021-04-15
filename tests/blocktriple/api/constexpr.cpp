// constexpr.cpp: compile time tests for blocktriple constexpr
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/directives.hpp>
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

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	int nrOfFailedTestCases = 0;

	cout << "blocktriple constexpr tests" << endl;
	
	{
		// decorated constructors
		{
			// this will round-up to 16 due to the fact that we only have 3 bits of significant
			CONSTEXPRESSION blocktriple<3> a(15);  // signed long
			cout << "constexpr constructor for type 'int'                 " << a << endl;
		}
		{
			// this will stay 15 due to the fact that we have 4 bits of significant
			CONSTEXPRESSION blocktriple<4> a(15);  // signed long
			cout << "constexpr constructor for type 'int'                 " << a << endl;
		}
		{
			CONSTEXPRESSION blocktriple<32> a(2);  // signed long
			cout << "constexpr constructor for type 'int'                 " << a << endl;
		}
		{
			CONSTEXPRESSION blocktriple<32> a(4ll);  // long long
			cout << "constexpr constructor for type 'long long'           " << a << endl;
		}
		{
			CONSTEXPRESSION blocktriple<32> a(8ul);  // unsigned long
			cout << "constexpr constructor for type 'unsigned long'       " << a << endl;
		}
		{
			CONSTEXPRESSION blocktriple<32> a(16ull);  // unsigned long
			cout << "constexpr constructor for type 'unsigned long long'  " << a << endl;
		}
#if BIT_CAST_SUPPORT
		{
			CONSTEXPRESSION blocktriple<32> a(1.125f);  // float
			cout << "constexpr constructor for type 'float'               " << a << endl;
		}
		{
			CONSTEXPRESSION blocktriple<32> a(1.0625);   // double
			cout << "constexpr constructor for type 'double'              " << a << endl;
		}
		{
			CONSTEXPRESSION blocktriple<32> a(1.03125l);  // long double
			cout << "constexpr constructor for type 'long double'         " << a << endl;
		}
#endif // BIT_CAST_SUPPORT
	}

	{
		// assignment operators
		{
			CONSTEXPRESSION blocktriple<32> a = 1l;  // signed long
			cout << a << endl;
		}
		{
			CONSTEXPRESSION blocktriple<32> a = 1ul;  // unsigned long
			cout << a << endl;
		}
#if BIT_CAST_SUPPORT
		{
			CONSTEXPRESSION blocktriple<32> a = 1.0f;  // float
			cout << a << endl;
		}
		{
			CONSTEXPRESSION blocktriple<32> a = 1.0;   // double
			cout << a << endl;
		}
		{
			CONSTEXPRESSION blocktriple<32> a = 1.0l;  // long double
			cout << a << endl;
		}
#endif // BIT_CAST_SUPPORT
	}



	if (nrOfFailedTestCases > 0) {
		cout << "FAIL" << endl;
	}
	else {
		cout << "PASS" << endl;
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
