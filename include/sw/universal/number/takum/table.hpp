#pragma once
// table.hpp: generate a table of encoding and values for takum configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// generate a full binary representation table for a given takum configuration
template<unsigned nbits, unsigned rbits = 3, typename BlockType = std::uint8_t>
void GenerateTakumTable(std::ostream& ostr, bool csvFormat = false) {
	const unsigned size = (1u << nbits);
	sw::universal::takum<nbits, rbits, BlockType> v;

	if (csvFormat) {
		ostr << "\"Generate Value table for a takum<" << nbits << "," << rbits << "> in CSV format\"" << '\n';
		ostr << "#, Binary, sign, direction, regime, characteristic, scale, value\n";
		for (unsigned i = 0; i < size; i++) {
			v.setbits(i);
			if (v.isnar()) {
				ostr << i << ","
					<< to_binary(v) << ","
					<< v.sign() << ","
					<< "," << "," << ","
					<< "NaR"
					<< '\n';
			}
			else {
				ostr << i << ","
					<< to_binary(v) << ","
					<< v.sign() << ","
					<< v.direct() << ","
					<< v.regime() << ","
					<< v.characteristic() << ","
					<< v.scale() << ","
					<< v
					<< '\n';
			}
		}
		ostr << '\n';
	}
	else {
		ostr << "Generate Value table for a takum<" << nbits << "," << rbits << "> in TXT format" << '\n';

		const unsigned index_column = 5;
		const unsigned bin_column = nbits + 8;  // accommodate dots in to_binary
		const unsigned sign_column = 6;
		const unsigned dir_column = 6;
		const unsigned regime_column = 8;
		const unsigned char_column = 8;
		const unsigned scale_column = 8;
		const unsigned value_column = 30;

		ostr << std::setw(index_column) << " # "
			<< std::setw(bin_column) << "Binary"
			<< std::setw(sign_column) << "sign"
			<< std::setw(dir_column) << "dir"
			<< std::setw(regime_column) << "regime"
			<< std::setw(char_column) << "char"
			<< std::setw(scale_column) << "scale"
			<< std::setw(value_column) << "value"
			<< '\n';
		for (unsigned i = 0; i < size; i++) {
			v.setbits(i);
			if (v.isnar()) {
				ostr << std::setw(4) << i << ": "
					<< std::setw(bin_column) << to_binary(v)
					<< std::setw(sign_column) << v.sign()
					<< std::setw(dir_column) << " "
					<< std::setw(regime_column) << " "
					<< std::setw(char_column) << " "
					<< std::setw(scale_column) << " "
					<< std::setw(value_column) << "NaR"
					<< '\n';
			}
			else {
				ostr << std::setw(4) << i << ": "
					<< std::setw(bin_column) << to_binary(v)
					<< std::setw(sign_column) << v.sign()
					<< std::setw(dir_column) << v.direct()
					<< std::setw(regime_column) << v.regime()
					<< std::setw(char_column) << v.characteristic()
					<< std::setw(scale_column) << v.scale()
					<< std::setw(value_column) << v
					<< '\n';
			}
		}
	}
}

}} // namespace sw::universal
