#pragma once
// table.hpp: generate a table for an classic floating-point configuration
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>  // for typeid()
#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/number/cfloat/manipulators.hpp>  // hex_print and the like

namespace sw { namespace universal {

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
	TestType v{};

	if (csvFormat) {
		ostr << "\"Generate Lookup table for a " << type_tag(v) << " in CSV format\"" << std::endl;
		ostr << "#, Binary, sign, scale, exponent, fraction, value, hex, unsigned, signed\n";
		for (size_t i = 0; i < NR_VALUES; i++) {
			v.setbits(i);
			blockbinary<nbits, bt> signedValue;
			v.bits(signedValue);  // blockbinary is a 2's complement integer
			blockbinary<nbits + 1, bt > unsignedValue; // keep MSB 0 to reflect an unsigned value
			v.bits(unsignedValue);
			bool s{ false };
			blockbinary<es, bt> e;
			blockbinary<fbits, bt> f;
			decode(v, s, e, f);
			ostr << i << ','
				<< to_binary(v) << ','
				<< s << ','
				<< scale(v) << ','
				<< std::right << to_binary(e) << ','
				<< std::right << to_binary(f) << ','
				<< v << ','
				<< hex_print(v) << ','
				<< signedValue << ','
				<< unsignedValue
				<< '\n';
		}
		ostr << std::endl;
	}
	else {
		ostr << "Generate table for a " << type_tag(v) << " in TXT format" << std::endl;

		const size_t index_column = 5;
		const size_t bin_column = 16;
		const size_t sign_column = 8;
		const size_t scale_column = 8;
		const size_t exponent_column = 16;
		const size_t fraction_column = 16;
		const size_t value_column = 30;
		const size_t hex_format_column = 16;
		const size_t integer_column = 10;

		ostr << std::setw(index_column) << " # "
			<< std::setw(bin_column) << "Binary"
			<< std::setw(sign_column) << "sign"
			<< std::setw(scale_column) << "scale"
			<< std::setw(exponent_column) << "exponent"
			<< std::setw(fraction_column) << "fraction"
			<< std::setw(value_column) << "value"
			<< std::setw(hex_format_column) << "hex_format"
			<< std::setw(integer_column) << "signed"
			<< std::setw(integer_column) << "positive"
			<< std::setw(integer_column) << "unsigned"
			<< std::endl;
		for (size_t i = 0; i < NR_VALUES; i++) {
			v.setbits(i);
			blockbinary<nbits, bt> signedValue, positiveProjection;
			v.bits(signedValue);  // blockbinary is a 2's complement integer
			positiveProjection = signedValue;
			if (signedValue.isneg()) {
				positiveProjection.twosComplement();
			}
			blockbinary<nbits + 1, bt > unsignedValue; // keep MSB 0 to reflect an unsigned value
			v.bits(unsignedValue);
			bool s{ false };
			blockbinary<es, bt> e;
			blockbinary<fbits, bt> f;
			decode(v, s, e, f);
			ostr << std::setw(4) << i << ": "
				<< std::setw(bin_column) << to_binary(v)
				<< std::setw(sign_column) << s
				<< std::setw(scale_column) << scale(v)
				<< std::setw(exponent_column) << std::right << to_binary(e, true)
				<< std::setw(fraction_column) << std::right << to_binary(f, true)
				<< std::setw(value_column) << v
				<< std::setw(hex_format_column) << std::right << hex_print(v)
				<< std::setw(integer_column) << std::right << signedValue
				<< std::setw(integer_column) << std::right << positiveProjection
				<< std::setw(integer_column) << std::right << unsignedValue
				<< std::endl;
		}
	}
}

/// <summary>
/// generate a table of cfloat exponent bounds up till es=20
/// </summary>
void GenerateCfloatExponentBounds()
{
	// max exp values as a function of es
	constexpr int WIDTH = 15;
	std::cout <<
		std::setw(WIDTH) << "es" <<
		std::setw(WIDTH) << "RAW_MAX_EXP" <<
		std::setw(WIDTH) << "EXP_BIAS" <<
		std::setw(WIDTH) << "MAX_EXP" <<
		std::setw(WIDTH) << "MIN_EXP_NORMAL" <<
		std::setw(WIDTH) << "MIN_NORMAL"
		<< '\n';
	size_t nrSubnormals = sizeof(subnormal_reciprocal_shift)/sizeof(subnormal_reciprocal_shift[0]);
	for (size_t es = 1; es < nrSubnormals; ++es) {
		int EXP_BIAS = ((1l << (es - 1ull)) - 1l);
		int RAW_MAX_EXP = (es == 1) ? 1 : ((1l << es) - 1);
		int MAX_EXP = (es == 1) ? 1 : ((1l << es) - EXP_BIAS - 1);
		int MIN_EXP_NORMAL = 1 - EXP_BIAS;
		double MIN_NORMAL = std::pow(2.0, MIN_EXP_NORMAL);
		// MIN_EXP_SUBNORMAL = 1 - EXP_BIAS - int(fbits); // the scale of smallest ULP
		std::cout <<
			std::setw(WIDTH) << es <<
			std::setw(WIDTH) << RAW_MAX_EXP <<
			std::setw(WIDTH) << EXP_BIAS <<
			std::setw(WIDTH) << MAX_EXP <<
			std::setw(WIDTH) << MIN_EXP_NORMAL <<
			std::setw(WIDTH) << MIN_NORMAL <<
			'\n';
	}
}

}} // namespace sw::universal

