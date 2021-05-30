#pragma once
// table.hpp: generate a table for an bfloat<> configuration
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>  // for typeid()
#include <universal/number/bfloat/bfloat.hpp>
#include <universal/number/bfloat/manipulators.hpp>  // hex_print and the like

namespace sw::universal {

/// <summary>
/// generate a full binary representation table for a given bfloat configuration
/// </summary>
/// <typeparam name="bt">type of the storage block used to represent the bfloat</typeparam>
/// <param name="ostr">ostream reference to write to</param>
/// <param name="uncertainty">if true output certain and uncertain values, otherwise only certain values</param>
/// <param name="csvFormat">if true present as a comma separated value format, text otherwise</param>
template<typename TestType>
void GenerateTable(std::ostream& ostr, bool csvFormat = false)	{
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t es    = TestType::es;
	constexpr size_t fbits = TestType::fbits;
	using bt = typename TestType::BlockType;
	constexpr size_t NR_VALUES = (1 << nbits);
	TestType v;

	if (csvFormat) {
		ostr << "\"Generate Lookup table for a " << typeid(v).name() << " in CSV format\"" << std::endl;
		ostr << "#, Binary, sign, scale, exponent, fraction, value, hex\n";
		for (size_t i = 0; i < NR_VALUES; i++) {
			v.setbits(i);
			bool s{ false };
			blockbinary<es, bt> e;
			blockbinary<fbits, bt> f;
			sw::universal::decode<nbits, es, fbits, bt>(v, s, e, f);
			ostr << i << ','
				<< to_binary(v) << ','
				<< s << ','
				<< scale(v) << ','
				<< std::right << to_binary(e) << ','
				<< std::right << to_binary(f) << ','
				<< v
				<< '\n';
		}
		ostr << std::endl;
	}
	else {
		ostr << "Generate table for a " << typeid(v).name() << " in TXT format" << std::endl;

		const size_t index_column = 5;
		const size_t bin_column = 16;
		const size_t sign_column = 8;
		const size_t scale_column = 8;
		const size_t exponent_column = 16;
		const size_t fraction_column = 16;
		const size_t value_column = 30;
		const size_t hex_format_column = 16;

		ostr << std::setw(index_column) << " # "
			<< std::setw(bin_column) << "Binary"
			<< std::setw(sign_column) << "sign"
			<< std::setw(scale_column) << "scale"
			<< std::setw(exponent_column) << "exponent"
			<< std::setw(fraction_column) << "fraction"
			<< std::setw(value_column) << "value"
			<< std::setw(hex_format_column) << "hex_format"
			<< std::endl;
		for (size_t i = 0; i < NR_VALUES; i++) {
			v.setbits(i);
			bool s{ false };
			blockbinary<es, bt> e;
			blockbinary<fbits, bt> f;
			sw::universal::decode<nbits, es, fbits, bt>(v, s, e, f);
			ostr << std::setw(4) << i << ": "
				<< std::setw(bin_column) << to_binary(v)
				<< std::setw(sign_column) << s
				<< std::setw(scale_column) << scale(v)
				<< std::setw(exponent_column) << std::right << to_binary(e, true)
				<< std::setw(fraction_column) << std::right << to_binary(f, true)
				<< std::setw(value_column) << v
				<< std::setw(hex_format_column) << std::right << hex_print(v)
				<< std::endl;
		}
	}
}

}  // namespace sw::universal

