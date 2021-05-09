#pragma once
// table.hpp: generate a posit table
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <iomanip>
#include <cmath>  // for frexp/frexpf
#include <typeinfo>  // for typeid()

namespace sw::universal {

// generate a full binary representation table for a given posit configuration
template<size_t nbits, size_t es>
void GeneratePositTable(std::ostream& ostr, bool csvFormat = false)	{
	static constexpr size_t fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
	const size_t size = (1 << nbits);
	posit<nbits, es>	p;
	if (csvFormat) {
		ostr << "\"Generate Posit Lookup table for a POSIT<" << nbits << "," << es << "> in CSV format\"" << std::endl;
		ostr << "#, Binary, Decoded, k, sign, scale, regime, exponent, fraction, value, posit\n";
		for (size_t i = 0; i < size; i++) {
			p.setBits(i);
			bool		     	 s;
			regime<nbits, es>    r;
			exponent<nbits, es>  e;
			fraction<fbits>      f;
			decode(p.get(), s, r, e, f);
			ostr << i << ","
				<< p.get() << ","
				<< decoded(p) << ","
				<< r.regime_k() << ","
				<< s << ","
				<< scale(p) << ","
				<< std::right << r << ","
				<< std::right << e << ","
				<< std::right << f << ","
				<< to_string(p, 22) << ","
				<< p
				<< '\n';
		}
		ostr << std::endl;
	}
	else {
		ostr << "Generate Posit Lookup table for a POSIT<" << nbits << "," << es << "> in TXT format" << std::endl;

		const size_t index_column = 5;
		const size_t bin_column = 16;
		const size_t k_column = 8;
		const size_t sign_column = 8;
		const size_t scale_column = 8;
		const size_t regime_column = 16;
		const size_t exponent_column = 16;
		const size_t fraction_column = 16;
		const size_t value_column = 30;
		const size_t posit_format_column = 16;

		ostr << std::setw(index_column) << " # "
			<< std::setw(bin_column) << "Binary"
			<< std::setw(bin_column) << "Decoded"
			<< std::setw(k_column) << "k"
			<< std::setw(sign_column) << "sign"
			<< std::setw(scale_column) << "scale"
			<< std::setw(regime_column) << "regime"
			<< std::setw(exponent_column) << "exponent"
			<< std::setw(fraction_column) << "fraction"
			<< std::setw(value_column) << "value"
			<< std::setw(posit_format_column) << "posit_format"
			<< std::endl;
		for (size_t i = 0; i < size; i++) {
			p.setBits(i);
			bool		     	 s;
			regime<nbits, es>    r;
			exponent<nbits, es>  e;
			fraction<fbits>      f;
			decode(p.get(), s, r, e, f);
			ostr << std::setw(4) << i << ": "
				<< std::setw(bin_column) << p.get()
				<< std::setw(bin_column) << decoded(p)
				<< std::setw(k_column) << r.regime_k()
				<< std::setw(sign_column) << s
				<< std::setw(scale_column) << scale(p)
				<< std::setw(regime_column) << std::right << to_string(r)
				<< std::setw(exponent_column) << std::right << to_string(e)
				<< std::setw(fraction_column) << std::right << to_string(f)
				<< std::setw(value_column) << to_string(p, 22) << " "
				<< std::setw(posit_format_column) << std::right << p
				<< std::endl;
		}
	}
}

}  // namespace sw::universal

