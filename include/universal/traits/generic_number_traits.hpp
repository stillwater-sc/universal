#pragma once
//  generic_number_traits.hpp : base type of number system traits
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { 
	
namespace internal {

	// default implementation of digits10(), based on numeric_limits if specialized,
	// 0 for integer types, and log10(epsilon()) otherwise.
	template< typename ScalarType,
		bool use_numeric_limits = std::numeric_limits<ScalarType>::is_specialized,
		bool is_integer = unum::number_traits<ScalarType>::is_integer>
		struct default_digits10_impl
	{
		static int run() { return std::numeric_limits<T>::digits10; }
	};

	template<typename T>
	struct default_digits10_impl<T, false, false> // Floating point
	{
		static int run() {
			using std::log10;
			using std::ceil;
			typedef typename NumTraits<T>::Real Real;
			return int(ceil(-log10(NumTraits<Real>::epsilon())));
		}
	};

	template<typename T>
	struct default_digits10_impl<T, false, true> // Integer
	{
		static int run() { return 0; }
	};

} // namespace internal

namespace unum {

	template<typename ScalarType>
	struct generic_number_traits {
		enum {
			is_integer = std::numeric_limits<ScalarType>::is_integer,
			is_signed = std::numeric_limits<ScalarType>::is_signed,
			is_complex = 0,
			needs_init = internal::is_arithmetic<ScalarType>::value ? 0 : 1,
		};
		static inline ScalarType epsilon() {
			return numext::numeric_limits<ScalarType>::epsilon();
		}
		static inline int digits10() {
			return internal::default_digits10_impl<T>::run();
		}

		static inline ScalarType maxpos() {
			return (numext::numeric_limits<ScalarType>::max)();
		}
	
		static inline ScalarType minpos() {
			return (numext::numeric_limits<ScalarType>::min)();
		}

		static inline ScalarType infinity() {
			return numext::numeric_limits<ScalarType>::infinity();
		}

		static inline ScalarType quiet_NaN() {
			return numext::numeric_limits<ScalarType>::quiet_NaN();
		}
	};

	template<typename ScalarType>
	struct number_traits : public generic_number_traits<ScalarType>
	{};

	template<> struct number_traits<float>
		: generic_number_traits<float>
	{
		//UNUM_DEVICE_FUNC
		static inline float dummy_precision() { return 1e-5f; }
	};

	template<> struct number_traits<double> 
		: generic_number_traits<double>
	{
		//UNUM_DEVICE_FUNC
		static inline double dummy_precision() { return 1e-12; }
	};

} // namespace unum

} // namespace sw
