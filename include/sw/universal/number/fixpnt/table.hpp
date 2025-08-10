// table.hpp: generate a table of encoding and values for fixed-size arbitrary fixed-point configurations
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// generate a full binary representation table for a given posit configuration
template<size_t nbits, size_t rbits>
void GenerateFixedPointTable(std::ostream& ostr, bool csvFormat = false) {
	const size_t size = (1 << nbits);
	sw::universal::fixpnt<nbits, rbits>	p;
	if (csvFormat) {
		ostr << "\"Generate Fixed-Point Lookup table for a FIXPNT<" << nbits << "," << rbits << "> in CSV format\"" << std::endl;
		ostr << "#, Binary, sign, scale, value\n";
		for (size_t i = 0; i < size; i++) {
			p.setbits(i);
			ostr << i << ","
				<< to_binary(p) << ","
				<< p.sign() << ","
				<< scale(p) << ","
				<< p
				<< '\n';
		}
		ostr << std::endl;
	}
	else {
		ostr << "Generate Fixed-Point Lookup table for a FIXPNT<" << nbits << "," << rbits << "> in TXT format" << std::endl;

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
			p.setbits(i);
			ostr << std::setw(4) << i << ": "
				<< std::setw(bin_column) << to_binary(p)
				<< std::setw(sign_column) << p.sign()
				<< std::setw(scale_column) << scale(p)
				<< std::setw(value_column) << p << " "
				<< std::setw(format_column) << std::right << p
				<< std::endl;
		}
	}
}

}}  // namespace sw::universal
