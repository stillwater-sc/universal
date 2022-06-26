// lns.cpp: generates encoding tables of lns configurations
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the lns template environment
// first: enable general or specialized lns configurations
#define LNS_FAST_SPECIALIZATION
// second: enable/disable lns arithmetic exceptions
#define LNS_THROW_ARITHMETIC_EXCEPTION 1

#include <universal/number/lns/lns.hpp>
//#include <universal/verification/test_suite.hpp>

// generate a full binary representation table for a given posit configuration
template<size_t nbits, size_t rbits>
void GenerateLnsTable(std::ostream& ostr, bool csvFormat = false) {
	const size_t size = (1 << nbits);
	sw::universal::lns<nbits, rbits> v;
	if (csvFormat) {
		ostr << "\"Generate Value table for an LNS<" << nbits << "," << rbits << "> in CSV format\"" << std::endl;
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
		ostr << "Generate Value table for an LNS<" << nbits << "," << rbits << "> in TXT format" << std::endl;

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

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// Usage: edu_tables_lns [-csv]
	bool csv = false;
	if (argc == 2) {
		if (std::string(argv[1]) == std::string("-csv")) csv = true;
	}
	std::cout << "Generate value tables for logarithmic number system configurations\n";

	GenerateLnsTable<4, 0>(std::cout, csv);
	GenerateLnsTable<4, 1>(std::cout, csv);
	GenerateLnsTable<4, 2>(std::cout, csv);
	GenerateLnsTable<4, 3>(std::cout, csv);

	GenerateLnsTable<5, 0>(std::cout, csv);
	GenerateLnsTable<5, 1>(std::cout, csv);
	GenerateLnsTable<5, 2>(std::cout, csv);
	GenerateLnsTable<5, 3>(std::cout, csv);
	GenerateLnsTable<5, 4>(std::cout, csv);

	GenerateLnsTable<6, 0>(std::cout, csv);
	GenerateLnsTable<6, 1>(std::cout, csv);
	GenerateLnsTable<6, 2>(std::cout, csv);
	GenerateLnsTable<6, 3>(std::cout, csv);
	GenerateLnsTable<6, 4>(std::cout, csv);
	GenerateLnsTable<6, 5>(std::cout, csv);

	GenerateLnsTable<8, 0>(std::cout, csv);
	GenerateLnsTable<8, 1>(std::cout, csv);
	GenerateLnsTable<8, 2>(std::cout, csv);
	GenerateLnsTable<8, 3>(std::cout, csv);
	GenerateLnsTable<8, 4>(std::cout, csv);
	GenerateLnsTable<8, 5>(std::cout, csv);
	GenerateLnsTable<8, 6>(std::cout, csv);
	GenerateLnsTable<8, 7>(std::cout, csv);

	// edge case, where sign and exponent msb are in different limbs
//	GenerateLnsTable<9, 8>(std::cout, csv);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
