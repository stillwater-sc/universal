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
	template< typename T,
		bool use_numeric_limits = std::numeric_limits<T>::is_specialized,
		bool is_integer = NumTraits<T>::IsInteger>
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

} // namespace unum

} // namespace sw
