// numbers.cpp: example program to use C++20 <numbers> high precision constants
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/compiler.hpp>
#include <cstddef>
#include <stdexcept>
#include <cstring>
#include <ostream>
#include <limits>
#include <numbers>    // high-precision constants

// select the number systems we would like to compare
#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

#ifndef _MSC_VER
#  if __cplusplus < 201103
#    define CONSTEXPR11_TN
#    define CONSTEXPR14_TN
#    define CONSTEXPR17_TN
#    define NOEXCEPT_TN
#  elif __cplusplus < 201402
#    define CONSTEXPR11_TN constexpr
#    define CONSTEXPR14_TN
#    define CONSTEXPR17_TN
#    define NOEXCEPT_TN noexcept
#  else
#    define CONSTEXPR11_TN constexpr
#    define CONSTEXPR14_TN constexpr
#    define CONSTEXPR17_TN constexpr
#    define NOEXCEPT_TN noexcept
#  endif
#else  // _MSC_VER
#  if _MSC_VER < 1900
#    define CONSTEXPR11_TN
#    define CONSTEXPR14_TN
#    define NOEXCEPT_TN
#  elif _MSC_VER < 2000
#    define CONSTEXPR11_TN constexpr
#    define CONSTEXPR14_TN
#    define NOEXCEPT_TN noexcept
#  else
#    define CONSTEXPR11_TN constexpr
#    define CONSTEXPR14_TN constexpr
#    define NOEXCEPT_TN noexcept
#  endif
#endif  // _MSC_VER

class static_string
{
	const char* const p_;
	const std::size_t sz_;

public:
	typedef const char* const_iterator;

	template <std::size_t N>
	CONSTEXPR11_TN static_string(const char(&a)[N]) NOEXCEPT_TN
		: p_(a)
		, sz_(N - 1)
	{}

	CONSTEXPR11_TN static_string(const char* p, std::size_t N) NOEXCEPT_TN
		: p_(p)
		, sz_(N)
	{}

	CONSTEXPR11_TN const char* data() const NOEXCEPT_TN { return p_; }
	CONSTEXPR11_TN std::size_t size() const NOEXCEPT_TN { return sz_; }

	CONSTEXPR11_TN const_iterator begin() const NOEXCEPT_TN { return p_; }
	CONSTEXPR11_TN const_iterator end()   const NOEXCEPT_TN { return p_ + sz_; }

	CONSTEXPR11_TN char operator[](std::size_t n) const
	{
		return n < sz_ ? p_[n] : throw std::out_of_range("static_string");
	}
};

inline
std::ostream&
operator<<(std::ostream& os, static_string const& s)
{
	return os.write(s.data(), static_cast<std::streamsize>(s.size()));
}

template <class T>
CONSTEXPR14_TN
static_string
type_name()
{
#ifdef __clang__
	static_string p = __PRETTY_FUNCTION__;
	return static_string(p.data() + 31, p.size() - 31 - 1);
#elif defined(__GNUC__)
	static_string p = __PRETTY_FUNCTION__;
#  if __cplusplus < 201402
	return static_string(p.data() + 36, p.size() - 36 - 1);
#  else
	return static_string(p.data() + 46, p.size() - 46 - 1);
#  endif
#elif defined(_MSC_VER)
	static_string p = __FUNCSIG__;
	return static_string(p.data() + 38, p.size() - 38 - 7);
#endif
}


int main()
try {
	using namespace sw::universal;

	std::cout << "high-precision constants\n";

	report_compiler();

	using int32 = integer<32, std::uint32_t>;
	using fixpnt32 = fixpnt<32, 16, Modulo, std::uint32_t>;
	using cfloat32 = cfloat<32, 8, std::uint32_t, true, true, false>;
	using posit32 = posit<32, 2>;
	using areal32 = areal<32, 8, std::uint32_t>;
	using lns32 = lns<32, 8, std::uint32_t>;

	// report on precision and dynamic range of the number system

	std::streamsize precision = std::cout.precision();

	constexpr size_t columnWidth = 30;
	numberTraits<int32, columnWidth>(std::cout);
	numberTraits<fixpnt32, columnWidth>(std::cout);
	numberTraits<float, columnWidth>(std::cout);
	numberTraits<cfloat32, columnWidth>(std::cout);
	numberTraits<areal32, columnWidth>(std::cout);
	numberTraits<posit32, columnWidth>(std::cout);
	numberTraits<lns32, columnWidth>(std::cout);

	std::cout << "a better type name: " << type_tag(posit32()) << '\n';

	std::cout << minmax_range<float>() << '\n';
	std::cout << minmax_range<posit32>() << '\n';
	std::cout << minmax_range<lns32>() << '\n';

	std::cout << dynamic_range<float>() << '\n';
	std::cout << dynamic_range<posit32>() << '\n';
	std::cout << dynamic_range<lns32>() << '\n';

	std::cout << symmetry_range<float>() << '\n';
	std::cout << symmetry_range<posit32>() << '\n';
	std::cout << symmetry_range<lns32>() << '\n';

	compareNumberTraits<float, cfloat32>(std::cout);
	compareNumberTraits<float, posit32>(std::cout);
	compareNumberTraits<float, lns32>(std::cout);

#if (__cplusplus == 202003L) || (_MSVC_LANG == 202003L)
	constexpr long double pi     = 3.14159265358979323846;
	//constexpr long double e      = 2.71828182845904523536;
	//constexpr long double log_2e = 1.44269504088896340736;

	cout << std::setprecision(50);
	cout << "my pi             " << pi << endl;
	cout << "numbers::pi       " << std::numbers::pi << endl;   // a double constexpr
	cout << "pi_v<float>       " << std::numbers::pi_v<float> << endl;
	cout << "pi_v<double>      " << std::numbers::pi_v<double> << endl;
	cout << "pi_v<long double> " << std::numbers::pi_v<long double> << endl;
#endif

	std::cout << std::setprecision(precision);
	std::cout << std::endl;
	
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
