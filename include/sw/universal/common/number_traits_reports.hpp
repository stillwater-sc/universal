#pragma once
// number_traits.hpp: utility functions for working with numeric_limits<> 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// a standard table format for numeric_limits<> of type Scalar
template<typename Scalar, size_t ColumnWidth = 40>
void numberTraits(std::ostream& ostr) {
	using namespace std;

	auto defaultPrecision = ostr.precision();
	ostr << std::scientific << std::setprecision(numeric_limits<Scalar>::max_digits10);
	ostr << "std::numeric_limits< " << type_tag(Scalar()) << " >\n";
	ostr << "min exponent       " << setw(ColumnWidth) << numeric_limits<Scalar>::min_exponent << '\n';
	ostr << "max exponent       " << setw(ColumnWidth) << numeric_limits<Scalar>::max_exponent << '\n';
	ostr << "radix              " << setw(ColumnWidth) << numeric_limits<Scalar>::radix << '\n';
	ostr << "radix digits       " << setw(ColumnWidth) << numeric_limits<Scalar>::digits << '\n';
	ostr << "min                " << setw(ColumnWidth) << numeric_limits<Scalar>::min() << '\n';
	ostr << "max                " << setw(ColumnWidth) << numeric_limits<Scalar>::max() << '\n';
	ostr << "lowest             " << setw(ColumnWidth) << numeric_limits<Scalar>::lowest() << '\n';
	ostr << "epsilon ==ulp(1.0) " << setw(ColumnWidth) << numeric_limits<Scalar>::epsilon() << '\n';
	ostr << "round_error        " << setw(ColumnWidth) << numeric_limits<Scalar>::round_error() << '\n';
	ostr << "smallest value     " << setw(ColumnWidth) << numeric_limits<Scalar>::denorm_min() << '\n';
	ostr << "infinity           " << setw(ColumnWidth) << numeric_limits<Scalar>::infinity() << '\n';
	ostr << "quiet_NAN          " << setw(ColumnWidth) << numeric_limits<Scalar>::quiet_NaN() << '\n';
	ostr << "signaling_NAN      " << setw(ColumnWidth) << numeric_limits<Scalar>::signaling_NaN() << '\n';
	ostr << '\n';
	ostr << std::setprecision(defaultPrecision);
}

// compare numeric_limits of two Real types
template<typename Type1, typename Type2, size_t ColumnWidth = 30>
void compareNumberTraits(std::ostream& ostr) {
	using namespace std;

	auto defaultPrecision = ostr.precision();
	ostr << std::scientific << std::setprecision(numeric_limits<Type1>::max_digits10);
	ostr << "comparing numeric_limits between " << type_tag(Type1()) << " and " << type_tag(Type2()) << '\n';
	ostr << "                " << setw(ColumnWidth) << typeid(Type1).name() << " vs " << setw(ColumnWidth) << typeid(Type2).name() << '\n';
	ostr << "min exponent    " << setw(ColumnWidth) << numeric_limits< Type1 >::min_exponent << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::min_exponent << '\n';
	ostr << "max exponent    " << setw(ColumnWidth) << numeric_limits< Type1 >::max_exponent << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::max_exponent << '\n';
	ostr << "radix           " << setw(ColumnWidth) << numeric_limits< Type1 >::radix << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::radix << '\n';
	ostr << "radix digits    " << setw(ColumnWidth) << numeric_limits< Type1 >::digits << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::digits << '\n';
	ostr << "min             " << setw(ColumnWidth) << numeric_limits< Type1 >::min() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::min() << '\n';
	ostr << "max             " << setw(ColumnWidth) << numeric_limits< Type1 >::max() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::max() << '\n';
	ostr << "lowest          " << setw(ColumnWidth) << numeric_limits< Type1 >::lowest() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::lowest() << '\n';
	ostr << "epsilon         " << setw(ColumnWidth) << numeric_limits< Type1 >::epsilon() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::epsilon() << '\n';
	ostr << "round_error     " << setw(ColumnWidth) << numeric_limits< Type1 >::round_error() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::round_error() << '\n';
	ostr << "smallest value  " << setw(ColumnWidth) << numeric_limits< Type1 >::denorm_min() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::denorm_min() << '\n';
	ostr << "infinity        " << setw(ColumnWidth) << numeric_limits< Type1 >::infinity() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::infinity() << '\n';
	ostr << "quiet_NAN       " << setw(ColumnWidth) << numeric_limits< Type1 >::quiet_NaN() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::quiet_NaN() << '\n';
	ostr << "signaling_NAN   " << setw(ColumnWidth) << numeric_limits< Type1 >::signaling_NaN() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::signaling_NaN() << '\n';
	ostr << '\n';
	ostr << std::setprecision(defaultPrecision);
}

// compare numeric_limits of three Real types
template<typename Type1, typename Type2, typename Type3, size_t ColumnWidth = 30>
void threeWayCompareNumberTraits(std::ostream& ostr) {
	using namespace std;

	auto defaultPrecision = ostr.precision();
	ostr << std::scientific << std::setprecision(numeric_limits<Type1>::max_digits10);
	ostr << "comparing numeric_limits between " << type_tag(Type1()) << " and " << type_tag(Type2()) << " and " << type_tag(Type3()) << '\n';
	ostr << "                " << setw(ColumnWidth) << typeid(Type1).name() << " vs " << setw(ColumnWidth) << typeid(Type2).name() << " vs " << setw(ColumnWidth) << typeid(Type3).name() << '\n';
	ostr << "min exponent    " << setw(ColumnWidth) << numeric_limits< Type1 >::min_exponent    << setw(ColumnWidth) << numeric_limits< Type2 >::min_exponent    << setw(ColumnWidth) << numeric_limits< Type3 >::min_exponent << '\n';
	ostr << "max exponent    " << setw(ColumnWidth) << numeric_limits< Type1 >::max_exponent    << setw(ColumnWidth) << numeric_limits< Type2 >::max_exponent    << setw(ColumnWidth) << numeric_limits< Type3 >::max_exponent << '\n';
	ostr << "radix           " << setw(ColumnWidth) << numeric_limits< Type1 >::radix           << setw(ColumnWidth) << numeric_limits< Type2 >::radix           << setw(ColumnWidth) << numeric_limits< Type3 >::radix << '\n';
	ostr << "radix digits    " << setw(ColumnWidth) << numeric_limits< Type1 >::digits          << setw(ColumnWidth) << numeric_limits< Type2 >::digits          << setw(ColumnWidth) << numeric_limits< Type3 >::digits << '\n';
	ostr << "min             " << setw(ColumnWidth) << numeric_limits< Type1 >::min()           << setw(ColumnWidth) << numeric_limits< Type2 >::min()           << setw(ColumnWidth) << numeric_limits< Type3 >::min() << '\n';
	ostr << "max             " << setw(ColumnWidth) << numeric_limits< Type1 >::max()           << setw(ColumnWidth) << numeric_limits< Type2 >::max()           << setw(ColumnWidth) << numeric_limits< Type3 >::max() << '\n';
	ostr << "lowest          " << setw(ColumnWidth) << numeric_limits< Type1 >::lowest()        << setw(ColumnWidth) << numeric_limits< Type2 >::lowest()        << setw(ColumnWidth) << numeric_limits< Type3 >::lowest() << '\n';
	ostr << "epsilon         " << setw(ColumnWidth) << numeric_limits< Type1 >::epsilon()       << setw(ColumnWidth) << numeric_limits< Type2 >::epsilon()       << setw(ColumnWidth) << numeric_limits< Type3 >::epsilon() << '\n';
	ostr << "round_error     " << setw(ColumnWidth) << numeric_limits< Type1 >::round_error()   << setw(ColumnWidth) << numeric_limits< Type2 >::round_error()   << setw(ColumnWidth) << numeric_limits< Type3 >::round_error() << '\n';
	ostr << "smallest value  " << setw(ColumnWidth) << numeric_limits< Type1 >::denorm_min()    << setw(ColumnWidth) << numeric_limits< Type2 >::denorm_min()    << setw(ColumnWidth) << numeric_limits< Type3 >::denorm_min() << '\n';
	ostr << "infinity        " << setw(ColumnWidth) << numeric_limits< Type1 >::infinity()      << setw(ColumnWidth) << numeric_limits< Type2 >::infinity()      << setw(ColumnWidth) << numeric_limits< Type3 >::infinity() << '\n';
	ostr << "quiet_NAN       " << setw(ColumnWidth) << numeric_limits< Type1 >::quiet_NaN()     << setw(ColumnWidth) << numeric_limits< Type2 >::quiet_NaN()     << setw(ColumnWidth) << numeric_limits< Type3 >::quiet_NaN() << '\n';
	ostr << "signaling_NAN   " << setw(ColumnWidth) << numeric_limits< Type1 >::signaling_NaN() << setw(ColumnWidth) << numeric_limits< Type2 >::signaling_NaN() << setw(ColumnWidth) << numeric_limits< Type3 >::signaling_NaN() << '\n';
	ostr << '\n';
	ostr << std::setprecision(defaultPrecision);
}

}} // namespace sw::universal
