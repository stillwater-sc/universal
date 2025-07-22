#pragma once
// table.hpp: generate a table of encoding and values for fixed-size arbitrary configuration posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// generate a full binary representation table for a given posit configuration
template<unsigned nbits, unsigned es>
void GeneratePositTable(std::ostream& ostr, bool csvFormat = false)	{
	static constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
	const unsigned size = (1 << nbits);
	posito<nbits, es>	p;
	if (csvFormat) {
		ostr << "\"Generate Posit Lookup table for a POSIT<" << nbits << "," << es << "> in CSV format\"" << std::endl;
		ostr << "#, Binary, Decoded, k, sign, scale, regime, exponent, fraction, value, posit\n";
		for (unsigned i = 0; i < size; i++) {
			p.setbits(i);
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

		const unsigned index_column = 5;
		const unsigned bin_column = 16;
		const unsigned k_column = 8;
		const unsigned sign_column = 8;
		const unsigned scale_column = 8;
		const unsigned regime_column = 16;
		const unsigned exponent_column = 16;
		const unsigned fraction_column = 16;
		const unsigned value_column = 30;
		const unsigned posit_format_column = 16;

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
		for (unsigned i = 0; i < size; i++) {
			p.setbits(i);
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

}} // namespace sw::universal

