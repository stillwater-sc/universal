#pragma once
// table.hpp: generate a table of encoding and values for fixed-size arbitrary configuration double base numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// generate a full binary representation table for a given dbns configuration
template<size_t nbits, size_t fbbits, typename BlockType = std::uint8_t, auto... xtra>
void GenerateDbnsTable(std::ostream& ostr, bool csvFormat = false) {
	const size_t size = (1 << nbits);
	sw::universal::dbns<nbits, fbbits, BlockType, xtra...> v;
	if (csvFormat) {
		ostr << "\"Generate Value table for an DBNS<" << nbits << "," << fbbits << "> in CSV format\"" << std::endl;
		ostr << "#, Binary, sign, scale, value\n";
		for (size_t i = 0; i < size; i++) {
			v.setbits(i);
			ostr << i << ","
				<< to_binary(v) << ","
				<< v.sign() << ","
				<< v.scale() << ","
				<< v
				<< '\n';
		}
		ostr << std::endl;
	}
	else {
		ostr << "Generate Value table for an DBNS<" << nbits << "," << fbbits << "> in TXT format" << std::endl;

		const size_t index_column = 5;
		const size_t bin_column = 16;
		const size_t sign_column = 8;
		const size_t scale_column = 8;
		const size_t value_column = 30;
		const size_t format_column = 16;

		ostr << std::setw(index_column) << " # "
			<< std::setw(bin_column) << "Binary"
			<< std::setw(sign_column) << "sign"
			<< std::setw(scale_column) << "scale"
			<< std::setw(value_column) << "value"
			<< std::setw(format_column) << "format"
			<< std::endl;
		for (size_t i = 0; i < size; i++) {
			v.setbits(i);
			ostr << std::setw(4) << i << ": "
				<< std::setw(bin_column) << to_binary(v)
				<< std::setw(sign_column) << v.sign()
				<< std::setw(scale_column) << v.scale()
				<< std::setw(value_column) << v << " "
				<< std::setw(format_column) << std::right << v
				<< std::endl;
		}
	}
}

}} // namespace sw::universal
