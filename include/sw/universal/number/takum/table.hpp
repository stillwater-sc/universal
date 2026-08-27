#pragma once
// table.hpp: generate a table of encoding and values for takum configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cstdint>       // std::uint8_t
#include <ostream>       // std::ostream
#include <iomanip>       // std::setw
namespace sw { namespace universal {

// Generate a full binary representation table for a takum or takum_log
// configuration.
//
// The two variants share the (S, D, R, C, M) layout down to the bit, so the
// table shares every column but the last.  A linear takum's value is
// (1 + f) * 2^c, and its base-2 scale is the natural companion column.  A
// logarithmic takum's value is sqrt(e)^l, where l is what the encoding actually
// carries; its scale() is a converted quantity and would be the less honest
// column of the two, so the logarithmic value is printed instead.
template<typename TakumType>
void GenerateTakumVariantTable(std::ostream& ostr, bool csvFormat = false) {
	constexpr bool logarithmic = is_takum_log<TakumType>;
	constexpr unsigned nbits = TakumType::nbits;
	constexpr unsigned rbits = TakumType::rbits;
	const char* typeName = (logarithmic ? "takum_log<" : "takum<");
	const char* lastLabel = (logarithmic ? "l" : "scale");

	// if constexpr, not a ternary: both arms of a ternary are compiled, and only
	// one of the two variants has each accessor.
	auto lastColumn = [](const TakumType& x) -> double {
		if constexpr (logarithmic) return x.logarithmic_value();
		else return static_cast<double>(x.scale());
	};

	const unsigned size = (1u << nbits);
	TakumType v;

	if (csvFormat) {
		ostr << "\"Generate Value table for a " << typeName << nbits << "," << rbits << "> in CSV format\"" << '\n';
		ostr << "#, Binary, sign, direction, regime, characteristic, " << lastLabel << ", value\n";
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
					<< lastColumn(v) << ","
					<< v
					<< '\n';
			}
		}
		ostr << '\n';
	}
	else {
		ostr << "Generate Value table for a " << typeName << nbits << "," << rbits << "> in TXT format" << '\n';

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
			<< std::setw(scale_column) << lastLabel
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
					<< std::setw(scale_column) << lastColumn(v)
					<< std::setw(value_column) << v
					<< '\n';
			}
		}
	}
}

// The original entry point, unchanged for every existing caller.
template<unsigned nbits, unsigned rbits = 3, typename BlockType = std::uint8_t>
void GenerateTakumTable(std::ostream& ostr, bool csvFormat = false) {
	GenerateTakumVariantTable< takum<nbits, rbits, BlockType> >(ostr, csvFormat);
}

// ... and its logarithmic counterpart.
template<unsigned nbits, unsigned rbits = 3, typename BlockType = std::uint8_t>
void GenerateTakumLogTable(std::ostream& ostr, bool csvFormat = false) {
	GenerateTakumVariantTable< takum_log<nbits, rbits, BlockType> >(ostr, csvFormat);
}

}} // namespace sw::universal
