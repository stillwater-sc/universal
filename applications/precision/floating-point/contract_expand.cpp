// contract_expand.cpp: evaluation of contractions and expansions of posit number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
//#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>

template<typename Scalar>
void ContractionExpansion(int depth) {
	using namespace sw::universal;

	int columnWidth = 20;
	Scalar seed = 2.0;
	std::cout << "Contraction/Expansion sequence sqrt(sqrt(sqrt(...sqrt(x))))))^depth => seed with seed = " << seed << '\n';
	std::cout << std::setw(3) << "#"
		<< std::setw(columnWidth) << "contraction"
		<< std::setw(columnWidth) << "expansion"
		<< std::setw(columnWidth) << "error"
		<< '\n';
	for (int i = 1; i < depth; ++i) {
		Scalar x = seed;
		for (int k = 1; k < i; ++k) {
			x = sqrt(x);
		}
		Scalar contraction = x;
		for (int k = 1; k < i; ++k) {
			x = exp2(x);
		}
		Scalar expansion = x;
		std::cout << std::setw(3) << i << " "
			<< std::setw(columnWidth) << contraction << " "
			<< std::setw(columnWidth) << expansion << " "
			<< std::setw(columnWidth) << expansion - seed
			<< '\n';
	}
}

// build a table of values for x, sqrt(x), pow(x,2)
template<typename Scalar>
void RangeTable(std::ostream& ostr) {
	using namespace sw::universal;
	constexpr unsigned nbits = Scalar::nbits;
	static_assert(nbits < 16, "size of the table is constrained to nbits < 16");

	unsigned COLUMN_WIDTH = 10;
	size_t NR_SAMPLES = (1ull << (nbits - 1)); // ignore negative values
	Scalar x;
	ostr << std::setw(COLUMN_WIDTH) << "x" << ','
		<< std::setw(COLUMN_WIDTH) << "y = x" << ','
		<< std::setw(COLUMN_WIDTH) << "y = sqrt(x)" << ','
		<< std::setw(COLUMN_WIDTH) << "y = x^2" << ','
		<< std::setw(COLUMN_WIDTH) << "y = sqrt(x^2)" << ','
		<< std::setw(COLUMN_WIDTH) << "y = sqrt(x)^2"
		<< '\n';
	for (unsigned i = 0; i < NR_SAMPLES; ++i) {
		x.setbits(i);
		Scalar sqrt_x = pow(x, 0.5);
		Scalar x_sqr = pow(x, 2.0);
		Scalar x_t1 = pow(pow(x, 2.0), 0.5);
		Scalar x_t2 = pow(pow(x, 0.5), 2.0);
		ostr << std::setw(COLUMN_WIDTH) << x << ','
			<< std::setw(COLUMN_WIDTH) << x << ','
			<< std::setw(COLUMN_WIDTH) << sqrt_x << ','
			<< std::setw(COLUMN_WIDTH) << x_sqr << ','
			<< std::setw(COLUMN_WIDTH) << x_t1 << ','
			<< std::setw(COLUMN_WIDTH) << x_t2
			<< '\n';
	}
}

void SquareRootSquared(std::ostream& ostr) {
	constexpr size_t nbits = 8;  // the sampling 
	unsigned COLUMN_WIDTH = 10;
	size_t NR_SAMPLES = (1ull << (nbits - 1)); // ignore negative values
	using c8_2  = sw::universal::cfloat< 8, 2, uint8_t, true, true, false>;
	using c10_2 = sw::universal::cfloat<10, 2, uint8_t, true, true, false> ;
	using c12_2 = sw::universal::cfloat<12, 2, uint8_t, true, true, false> ;
	using c14_2 = sw::universal::cfloat<14, 2, uint8_t, true, true, false> ;
	using c16_2 = sw::universal::cfloat<16, 2, uint8_t, true, true, false> ;
	ostr << std::setw(COLUMN_WIDTH) << "x" << ','
		<< std::setw(COLUMN_WIDTH) << "y = x" << ','
		<< std::setw(COLUMN_WIDTH) << "cfloat<8,2>" << ','
		<< std::setw(COLUMN_WIDTH) << "cfloat<10,2>" << ','
		<< std::setw(COLUMN_WIDTH) << "cfloat<12,2>" << ','
		<< std::setw(COLUMN_WIDTH) << "cfloat<14,2>" << ','
		<< std::setw(COLUMN_WIDTH) << "cfloat<16,2>" << ','
		<< '\n';
	c8_2 x;
	for (unsigned i = 0; i < NR_SAMPLES; ++i) {
		x.setbits(i);
		float v = float(x);
		c8_2  v8(v);
		c10_2 v10(v);
		c12_2 v12(v);
		c14_2 v14(v);
		c16_2 v16(v);
		ostr << std::setw(COLUMN_WIDTH) << x << ','
			<< std::setw(COLUMN_WIDTH) << v8 << ','
			<< std::setw(COLUMN_WIDTH) << pow(pow(v8, 0.5), 2.0) << ','
			<< std::setw(COLUMN_WIDTH) << pow(pow(v10, 0.5), 2.0) << ','
			<< std::setw(COLUMN_WIDTH) << pow(pow(v12, 0.5), 2.0) << ','
			<< std::setw(COLUMN_WIDTH) << pow(pow(v14, 0.5), 2.0) << ','
			<< std::setw(COLUMN_WIDTH) << pow(pow(v16, 0.5), 2.0)
			<< '\n';
	}

}

void SquareRootSquared2(std::ostream& ostr) {
	constexpr unsigned nbits = 8;  // the sampling 
	unsigned COLUMN_WIDTH = 10;
	size_t NR_SAMPLES = (1ull << (nbits - 1)); // ignore negative values
	using c8_2 = sw::universal::cfloat<8, 2>;
	using c10_2 = sw::universal::cfloat<10, 2>;
	using c12_3 = sw::universal::cfloat<12, 3>;
	using c14_4 = sw::universal::cfloat<14, 4>;
	using c16_5 = sw::universal::cfloat<16, 5>;
	ostr << std::setw(COLUMN_WIDTH) << "x" << ','
		<< std::setw(COLUMN_WIDTH) << "y = x" << ','
		<< std::setw(COLUMN_WIDTH) << "cfloat<8,2>" << ','
		<< std::setw(COLUMN_WIDTH) << "cfloat<10,2>" << ','
		<< std::setw(COLUMN_WIDTH) << "cfloat<12,3>" << ','
		<< std::setw(COLUMN_WIDTH) << "cfloat<14,4>" << ','
		<< std::setw(COLUMN_WIDTH) << "cfloat<16,5>" << ','
		<< '\n';
	c8_2 x;
	for (unsigned i = 0; i < NR_SAMPLES; ++i) {
		x.setbits(i);
		float v = float(x);
		c8_2  v8(v);
		c10_2 v10(v);
		c12_3 v12(v);
		c14_4 v14(v);
		c16_5 v16(v);
		ostr << std::setw(COLUMN_WIDTH) << x << ','
			<< std::setw(COLUMN_WIDTH) << v8 << ','
			<< std::setw(COLUMN_WIDTH) << pow(pow(v8, 0.5), 2.0) << ','
			<< std::setw(COLUMN_WIDTH) << pow(pow(v10, 0.5), 2.0) << ','
			<< std::setw(COLUMN_WIDTH) << pow(pow(v12, 0.5), 2.0) << ','
			<< std::setw(COLUMN_WIDTH) << pow(pow(v14, 0.5), 2.0) << ','
			<< std::setw(COLUMN_WIDTH) << pow(pow(v16, 0.5), 2.0)
			<< '\n';
	}
}

int main()
try {
	using namespace sw::universal;

	// print detailed bit-level computational intermediate results
	// bool verbose = false;

	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << std::setprecision(12);

	/*
	{
		constexpr size_t nbits = 32;
		constexpr size_t es = 2;
		using Posit = posit<nbits, es>;
		ContractionExpansion<Posit>(10);
	}
	*/

	{
		using Real = cfloat<8, 2, uint8_t, false, false, false>;
		RangeTable<Real>(std::cout);
	}


	// SquareRootSquared(std::cout);

	// restore the previous ostream precision
	std::cout << std::setprecision(precision);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
