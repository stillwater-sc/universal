#pragma once
//  metaprogramming.hpp : meta-programming patterns
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

namespace sw { 
	
namespace internal {

	struct true_type { enum { value = 1 }; };
	struct false_type { enum { value = 0 }; };

	template<bool Condition, typename Then, typename Else>
	struct conditional { typedef Then type; };

	template<typename Then, typename Else>
	struct conditional <false, Then, Else> { typedef Else type; };

	template<typename T, typename U> struct is_same { enum { value = 0 }; };
	template<typename T> struct is_same<T, T> { enum { value = 1 }; };

	template<typename T> struct remove_reference { typedef T type; };
	template<typename T> struct remove_reference<T&> { typedef T type; };

	template<typename T> struct remove_pointer { typedef T type; };
	template<typename T> struct remove_pointer<T*> { typedef T type; };
	template<typename T> struct remove_pointer<T*const> { typedef T type; };

	template <class T> struct remove_const { typedef T type; };
	template <class T> struct remove_const<const T> { typedef T type; };
	template <class T> struct remove_const<const T[]> { typedef T type[]; };
	template <class T, unsigned int Size> struct remove_const<const T[Size]> { typedef T type[Size]; };

	template<typename T> struct remove_all { typedef T type; };
	template<typename T> struct remove_all<const T> { typedef typename remove_all<T>::type type; };
	template<typename T> struct remove_all<T const&> { typedef typename remove_all<T>::type type; };
	template<typename T> struct remove_all<T&> { typedef typename remove_all<T>::type type; };
	template<typename T> struct remove_all<T const*> { typedef typename remove_all<T>::type type; };
	template<typename T> struct remove_all<T*> { typedef typename remove_all<T>::type type; };

	template<typename T> struct is_arithmetic { enum { value = false }; };
	template<> struct is_arithmetic<float> { enum { value = true }; };
	template<> struct is_arithmetic<double> { enum { value = true }; };
	template<> struct is_arithmetic<long double> { enum { value = true }; };
	template<> struct is_arithmetic<bool> { enum { value = true }; };
	template<> struct is_arithmetic<char> { enum { value = true }; };
	template<> struct is_arithmetic<signed char> { enum { value = true }; };
	template<> struct is_arithmetic<unsigned char> { enum { value = true }; };
	template<> struct is_arithmetic<signed short> { enum { value = true }; };
	template<> struct is_arithmetic<unsigned short> { enum { value = true }; };
	template<> struct is_arithmetic<signed int> { enum { value = true }; };
	template<> struct is_arithmetic<unsigned int> { enum { value = true }; };
	template<> struct is_arithmetic<signed long> { enum { value = true }; };
	template<> struct is_arithmetic<unsigned long> { enum { value = true }; };

	template<typename T> struct is_integral { enum { value = false }; };
	template<> struct is_integral<bool> { enum { value = true }; };
	template<> struct is_integral<char> { enum { value = true }; };
	template<> struct is_integral<signed char> { enum { value = true }; };
	template<> struct is_integral<unsigned char> { enum { value = true }; };
	template<> struct is_integral<signed short> { enum { value = true }; };
	template<> struct is_integral<unsigned short> { enum { value = true }; };
	template<> struct is_integral<signed int> { enum { value = true }; };
	template<> struct is_integral<unsigned int> { enum { value = true }; };
	template<> struct is_integral<signed long> { enum { value = true }; };
	template<> struct is_integral<unsigned long> { enum { value = true }; };

	template <typename T> struct add_const { typedef const T type; };
	template <typename T> struct add_const<T&> { typedef T& type; };

	template <typename T> struct is_const { enum { value = 0 }; };
	template <typename T> struct is_const<T const> { enum { value = 1 }; };

	template<typename T> struct add_const_on_value_type { typedef const T type; };
	template<typename T> struct add_const_on_value_type<T&> { typedef T const& type; };
	template<typename T> struct add_const_on_value_type<T*> { typedef T const* type; };
	template<typename T> struct add_const_on_value_type<T* const> { typedef T const* const type; };
	template<typename T> struct add_const_on_value_type<T const* const> { typedef T const* const type; };

#if defined(__CUDA_ARCH__)
#if !defined(__FLT_EPSILON__)
#define __FLT_EPSILON__ FLT_EPSILON
#define __DBL_EPSILON__ DBL_EPSILON
#endif

namespace device {

	template<typename T> struct numeric_limits {
		UNUM_DEVICE_FUNC
			static T epsilon() { return 0; }
		static T(max)() { assert(false && "Highest not supported for this type"); }
		static T(min)() { assert(false && "Lowest not supported for this type"); }
		static T infinity() { assert(false && "Infinity not supported for this type"); }
		static T quiet_NaN() { assert(false && "quiet_NaN not supported for this type"); }
	};
	template<> struct numeric_limits<float>	{
		UNUM_DEVICE_FUNC
			static float epsilon() { return __FLT_EPSILON__; }
		UNUM_DEVICE_FUNC
			static float (max)() { return CUDART_MAX_NORMAL_F; }
		UNUM_DEVICE_FUNC
			static float (min)() { return FLT_MIN; }
		UNUM_DEVICE_FUNC
			static float infinity() { return CUDART_INF_F; }
		UNUM_DEVICE_FUNC
			static float quiet_NaN() { return CUDART_NAN_F; }
	};
	template<> struct numeric_limits<double> {
		UNUM_DEVICE_FUNC
			static double epsilon() { return __DBL_EPSILON__; }
		UNUM_DEVICE_FUNC
			static double (max)() { return DBL_MAX; }
		UNUM_DEVICE_FUNC
			static double (min)() { return DBL_MIN; }
		UNUM_DEVICE_FUNC
			static double infinity() { return CUDART_INF; }
		UNUM_DEVICE_FUNC
			static double quiet_NaN() { return CUDART_NAN; }
	};
	template<> struct numeric_limits<int> {
		UNUM_DEVICE_FUNC
			static int epsilon() { return 0; }
		UNUM_DEVICE_FUNC
			static int (max)() { return INT_MAX; }
		UNUM_DEVICE_FUNC
			static int (min)() { return INT_MIN; }
	};
	template<> struct numeric_limits<unsigned int> {
		UNUM_DEVICE_FUNC
			static unsigned int epsilon() { return 0; }
		UNUM_DEVICE_FUNC
			static unsigned int (max)() { return UINT_MAX; }
		UNUM_DEVICE_FUNC
			static unsigned int (min)() { return 0; }
	};
	template<> struct numeric_limits<long> {
		UNUM_DEVICE_FUNC
			static long epsilon() { return 0; }
		UNUM_DEVICE_FUNC
			static long (max)() { return LONG_MAX; }
		UNUM_DEVICE_FUNC
			static long (min)() { return LONG_MIN; }
	};
	template<> struct numeric_limits<unsigned long>	{
		UNUM_DEVICE_FUNC
			static unsigned long epsilon() { return 0; }
		UNUM_DEVICE_FUNC
			static unsigned long (max)() { return ULONG_MAX; }
		UNUM_DEVICE_FUNC
			static unsigned long (min)() { return 0; }
	};
	template<> struct numeric_limits<long long>	{
		UNUM_DEVICE_FUNC
			static long long epsilon() { return 0; }
		UNUM_DEVICE_FUNC
			static long long (max)() { return LLONG_MAX; }
		UNUM_DEVICE_FUNC
			static long long (min)() { return LLONG_MIN; }
	};
	template<> struct numeric_limits<unsigned long long> {
		UNUM_DEVICE_FUNC
			static unsigned long long epsilon() { return 0; }
		UNUM_DEVICE_FUNC
			static unsigned long long (max)() { return ULLONG_MAX; }
		UNUM_DEVICE_FUNC
			static unsigned long long (min)() { return 0; }
	};
#endif // __CUDA_ARCH__
} // namespace device

namespace numext {

#if defined(__CUDA_ARCH__)
	using internal::device::numeric_limits;
#else
	using std::numeric_limits;
#endif

}

} // namespace sw
