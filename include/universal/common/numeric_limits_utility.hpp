#pragma once
// numeric_limits_utility.hpp: utility functions for working with numeric_limits<> 
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace unum {

// a standard table format for numeric_limits<> of type Real
template<typename Scalar>
void numeric_limits_table(std::ostream& ostr) {
	using namespace std;

	constexpr unsigned int COLUMN_WIDTH = 20;
	ostr << "std::numeric_limits< " << typeid(Scalar).name() << " >\n";
	ostr << "min             " << setw(COLUMN_WIDTH) << numeric_limits<Scalar>::min() << '\n';
	ostr << "max             " << setw(COLUMN_WIDTH) << numeric_limits<Scalar>::max() << '\n';
	ostr << "lowest          " << setw(COLUMN_WIDTH) << numeric_limits<Scalar>::lowest() << '\n';
	ostr << "epsilon         " << setw(COLUMN_WIDTH) << numeric_limits<Scalar>::epsilon() << '\n';
	ostr << "round_error     " << setw(COLUMN_WIDTH) << numeric_limits<Scalar>::round_error() << '\n';
	ostr << "denorm_min      " << setw(COLUMN_WIDTH) << numeric_limits<Scalar>::denorm_min() << '\n';
	ostr << "infinity        " << setw(COLUMN_WIDTH) << numeric_limits<Scalar>::infinity() << '\n';
	ostr << "quiet_NAN       " << setw(COLUMN_WIDTH) << numeric_limits<Scalar>::quiet_NaN() << '\n';
	ostr << "signaling_NAN   " << setw(COLUMN_WIDTH) << numeric_limits<Scalar>::signaling_NaN() << '\n';
}

// compare numeric_limits of two Real types
template<typename Type1, typename Type2>
void compare_numeric_limits(std::ostream& ostr) {
	using namespace std;

	constexpr unsigned int COLUMN_WIDTH = 20;
	ostr << "comparing numeric_limits between float and posit<32,2>\n";
	ostr << "                " << setw(COLUMN_WIDTH) << "IEEE float" << " vs " << setw(COLUMN_WIDTH) << "posit<32,2>" << '\n';
	ostr << "min             " << setw(COLUMN_WIDTH) << numeric_limits< Type1 >::min() << " vs " << setw(COLUMN_WIDTH) << numeric_limits< Type2 >::min() << '\n';
	ostr << "max             " << setw(COLUMN_WIDTH) << numeric_limits< Type1 >::max() << " vs " << setw(COLUMN_WIDTH) << numeric_limits< Type2 >::max() << '\n';
	ostr << "lowest          " << setw(COLUMN_WIDTH) << numeric_limits< Type1 >::lowest() << " vs " << setw(COLUMN_WIDTH) << numeric_limits< Type2 >::lowest() << '\n';
	ostr << "epsilon         " << setw(COLUMN_WIDTH) << numeric_limits< Type1 >::epsilon() << " vs " << setw(COLUMN_WIDTH) << numeric_limits< Type2 >::epsilon() << '\n';
	ostr << "round_error     " << setw(COLUMN_WIDTH) << numeric_limits< Type1 >::round_error() << " vs " << setw(COLUMN_WIDTH) << numeric_limits< Type2 >::round_error() << '\n';
	ostr << "denorm_min      " << setw(COLUMN_WIDTH) << numeric_limits< Type1 >::denorm_min() << " vs " << setw(COLUMN_WIDTH) << numeric_limits< Type2 >::denorm_min() << '\n';
	ostr << "infinity        " << setw(COLUMN_WIDTH) << numeric_limits< Type1 >::infinity() << " vs " << setw(COLUMN_WIDTH) << numeric_limits< Type2 >::infinity() << '\n';
	ostr << "quiet_NAN       " << setw(COLUMN_WIDTH) << numeric_limits< Type1 >::quiet_NaN() << " vs " << setw(COLUMN_WIDTH) << numeric_limits< Type2 >::quiet_NaN() << '\n';
	ostr << "signaling_NAN   " << setw(COLUMN_WIDTH) << numeric_limits< Type1 >::signaling_NaN() << " vs " << setw(COLUMN_WIDTH) << numeric_limits< Type2 >::signaling_NaN() << '\n';
}
} }