#pragma once
// numeric_limits_utility.hpp: utility functions for working with numeric_limits<> 
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace unum {

// a standard table format for numeric_limits<> of type Scalar
template<typename Scalar, size_t ColumnWidth = 20>
void numberTraits(std::ostream& ostr) {
	using namespace std;

	ostr << "std::numeric_limits< " << typeid(Scalar).name() << " >\n";
	ostr << "min exponent       " << setw(ColumnWidth) << numeric_limits<Scalar>::min_exponent << '\n';
	ostr << "max exponent       " << setw(ColumnWidth) << numeric_limits<Scalar>::max_exponent << '\n';
	ostr << "radix              " << setw(ColumnWidth) << numeric_limits<Scalar>::radix << '\n';
	ostr << "radix digits       " << setw(ColumnWidth) << numeric_limits<Scalar>::digits << '\n';
	ostr << "min                " << setw(ColumnWidth) << numeric_limits<Scalar>::min() << '\n';
	ostr << "max                " << setw(ColumnWidth) << numeric_limits<Scalar>::max() << '\n';
	ostr << "lowest             " << setw(ColumnWidth) << numeric_limits<Scalar>::lowest() << '\n';
	ostr << "epsilon (1+1ULP-1) " << setw(ColumnWidth) << numeric_limits<Scalar>::epsilon() << '\n';
	ostr << "round_error        " << setw(ColumnWidth) << numeric_limits<Scalar>::round_error() << '\n';
	ostr << "denorm_min         " << setw(ColumnWidth) << numeric_limits<Scalar>::denorm_min() << '\n';
	ostr << "infinity           " << setw(ColumnWidth) << numeric_limits<Scalar>::infinity() << '\n';
	ostr << "quiet_NAN          " << setw(ColumnWidth) << numeric_limits<Scalar>::quiet_NaN() << '\n';
	ostr << "signaling_NAN      " << setw(ColumnWidth) << numeric_limits<Scalar>::signaling_NaN() << '\n';
}

// compare numeric_limits of two Real types
template<typename Type1, typename Type2, size_t ColumnWidth = 20>
void compareNumberTraits(std::ostream& ostr) {
	using namespace std;

	ostr << "comparing numeric_limits between " << typeid(Type1).name() << " and " << typeid(Type2).name() << '\n';
	ostr << "                " << setw(ColumnWidth) << typeid(Type1).name() << " vs " << setw(ColumnWidth) << typeid(Type2).name() << '\n';
	ostr << "min             " << setw(ColumnWidth) << numeric_limits< Type1 >::min_exponent << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::min_exponent << '\n';
	ostr << "max             " << setw(ColumnWidth) << numeric_limits< Type1 >::max_exponent << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::max_exponent << '\n';
	ostr << "lowest          " << setw(ColumnWidth) << numeric_limits< Type1 >::radix << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::radix << '\n';
	ostr << "epsilon         " << setw(ColumnWidth) << numeric_limits< Type1 >::digits << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::digits << '\n';
	ostr << "min             " << setw(ColumnWidth) << numeric_limits< Type1 >::min() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::min() << '\n';
	ostr << "max             " << setw(ColumnWidth) << numeric_limits< Type1 >::max() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::max() << '\n';
	ostr << "lowest          " << setw(ColumnWidth) << numeric_limits< Type1 >::lowest() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::lowest() << '\n';
	ostr << "epsilon         " << setw(ColumnWidth) << numeric_limits< Type1 >::epsilon() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::epsilon() << '\n';
	ostr << "round_error     " << setw(ColumnWidth) << numeric_limits< Type1 >::round_error() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::round_error() << '\n';
	ostr << "denorm_min      " << setw(ColumnWidth) << numeric_limits< Type1 >::denorm_min() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::denorm_min() << '\n';
	ostr << "infinity        " << setw(ColumnWidth) << numeric_limits< Type1 >::infinity() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::infinity() << '\n';
	ostr << "quiet_NAN       " << setw(ColumnWidth) << numeric_limits< Type1 >::quiet_NaN() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::quiet_NaN() << '\n';
	ostr << "signaling_NAN   " << setw(ColumnWidth) << numeric_limits< Type1 >::signaling_NaN() << " vs " << setw(ColumnWidth) << numeric_limits< Type2 >::signaling_NaN() << '\n';
}


} }  // namespace sw::unum