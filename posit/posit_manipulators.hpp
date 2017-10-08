#pragma once
// posit_manipulators.hpp: definitions of helper functions for posit type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <iomanip>
#include <cmath>  // for frexp/frexpf

// This file contains functions that use the posit type.
// If you have helper functions that the posit type could use, but does not depend on 
// the posit type, you can add them to the file posit_helpers.hpp.

// generate a full binary representation table for a given posit configuration
template<size_t nbits, size_t es>
void GeneratePositTable(std::ostream& ostr) 
{
	ostr << "Generate Posit Lookup table for a POSIT<" << nbits << "," << es << ">" << std::endl;

	const size_t size = (1 << nbits);
	posit<nbits, es>	myPosit;

	const size_t index_column = 5;
	const size_t bin_column = 16;
	const size_t k_column = 16;
	const size_t sign_column = 16;
	const size_t regime_column = 30;
	const size_t exponent_column = 16;
	const size_t fraction_column = 16;
	const size_t value_column = 30;

	ostr << std::setw(index_column) << " # "
		<< std::setw(bin_column) << " Binary"
		<< std::setw(bin_column) << " Decoded"
		<< std::setw(k_column) << " k-value"
		<< std::setw(sign_column) << "sign"
		<< std::setw(regime_column) << " regime"
		<< std::setw(exponent_column) << " exponent"
		<< std::setw(fraction_column) << " fraction"
		<< std::setw(value_column) << " value" << std::endl;
	for (int i = 0; i < size; i++) {
		myPosit.set_raw_bits(i);
		regime<nbits,es>   r = myPosit.get_regime();
		exponent<nbits,es> e = myPosit.get_exponent();
		fraction<nbits,es> f = myPosit.get_fraction();
		ostr << std::setw(4) << i << ": "
			<< std::setw(bin_column) << myPosit.get()
			<< std::setw(bin_column) << myPosit.get_decoded()
			<< std::setw(k_column) << myPosit.regime_k()
			<< std::setw(sign_column) << myPosit.sign_value()
			<< std::setw(regime_column) << std::setprecision(22) << r.value() << std::setprecision(0)
			<< std::setw(exponent_column) << std::right << e 
			<< std::setw(fraction_column) << std::right << f
			<< std::setw(value_column) << std::setprecision(22) << myPosit.to_double() << std::setprecision(0)
			<< std::endl;
	}
}
