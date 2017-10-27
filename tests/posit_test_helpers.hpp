//  posit_test_helpers.cpp : functions to aid in testing and test reporting on posit types.
// Needs to be included after posit type is declared.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

template<size_t nbits, size_t es>
void ReportUnaryArithmeticError(std::string test_case, std::string op, const posit<nbits, es>& lhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
	std::cerr << test_case
		<< " " << op << " "	
		<< std::setw(10) << lhs
		<< " != "
		<< std::setw(10) << pref << " instead it yielded "
		<< std::setw(10) << presult
		<< " " << components_to_string(presult) << std::endl;
}

template<size_t nbits, size_t es>
void ReportBinaryArithmeticError(std::string test_case, std::string op, const posit<nbits, es>& lhs, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
	std::cerr << test_case
		<< std::setw(10) << lhs
		<< " " << op << " "
		<< std::setw(10) << rhs
		<< " != "
		<< std::setw(10) << pref <<    " instead it yielded "
		<< std::setw(10) << presult
		<< " " << components_to_string(presult) << std::endl;
}

template<size_t nbits, size_t es>
void ReportBinaryArithmeticSuccess(std::string test_case, std::string op, const posit<nbits, es>& lhs, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
	std::cerr << test_case
		<< std::setw(10) << lhs
		<< " " << op << " "
		<< std::setw(10) << rhs
		<< " == "
		<< std::setw(10) << presult << " reference value is "
		<< std::setw(10) << pref
		<< " " << components_to_string(presult) << std::endl;
}

template<size_t nbits, size_t es>
void ReportDecodeError(std::string test_case, const posit<nbits, es>& actual, double golden_value) {
	std::cerr << test_case << " actual " << actual << " required " << golden_value << std::endl;
}