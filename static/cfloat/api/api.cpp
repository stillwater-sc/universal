// api.cpp: application programming interface tests for cfloat number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the cfloat template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "cfloat<> Application Programming Interface demonstration";
	int nrOfFailedTestCases = 0;

	// important behavioral traits
	{
		using TestType = cfloat<8, 2>;
		ReportTrivialityOfType<TestType>();
	}

	// construction, initialization, and copy construction
	{
		quarter q{ 1.0f };  // literal float to fp8
		single s(1.0);      // literal double to single
		half h(s);          // variable single to half precision
		fp8e5m2 fp8e5 = h;  // half to custom fp8e5m2 precision
		std::cout << "IEEE-754 fp8     : " << to_binary(q) << " : " << q << '\n';
		std::cout << "IEEE-754 fp16    : " << to_binary(h) << " : " << h << '\n';
		std::cout << "IEEE-754 fp32    : " << to_binary(s) << " : " << s << '\n';
		std::cout << "custom   fp8e5m2 : " << to_binary(fp8e5) << " : " << fp8e5 << '\n';
	}

	// default behavior
	std::cout << "+---------    Default cfloat has no subnormals, no supernormals and is not saturating\n";
	{
		constexpr size_t nbits = 8;
		constexpr size_t es = 3;
		using Real = cfloat<nbits, es>;  // bt = uint8_t, hasSubnormals = false, hasSupernormals = false, isSaturating = false

		Real a(1.0f), b(0.5f);
		ArithmeticOperators(a, b);
	}

	// explicit configuration
	std::cout << "+---------    Explicit configuration of a cfloat\n";
	{
		constexpr size_t nbits = 8;
		constexpr size_t es = 3;
		using bt = uint8_t;
		constexpr bool hasSubnormals   = true;
		constexpr bool hasSupernormals = true;
		constexpr bool isSaturating    = false;
		using Real = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;

		Real a(1.0f), b(0.5f);
		ArithmeticOperators(a, b);
	}

	// report on the dynamic range of some standard configurations
	std::cout << "+---------    Dynamic ranges of some standard cfloat<> configurations   --------+\n";
	{
		// quarter, half, single, duble, quad, and octo precision IEEE-754 style floating-point
		std::cout << "quarter  precision: " << cfloat_range<quarter>() << '\n';
		std::cout << "half     precision: " << cfloat_range<half>() << '\n';
		std::cout << "single   precision: " << cfloat_range<single>() << '\n';
		std::cout << "double   precision: " << cfloat_range<duble>() << '\n';
		std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
		std::cout << "performance of printing of quad and larger precision needs to improve to be practical\n";
//		cfloat_range<quad>(std::cout);
//		cfloat_range<octo>(std::cout);

		std::cout << "---\n";

		quarter q; // uninitialized
		q.setbits(0x01);  // smallest subnormal
		std::cout << "minpos  cfloat<8,2> : " << to_binary(q) << " : " << q << '\n';
		q.setbits(0x5f);  // max normal
		std::cout << "maxnorm cfloat<8,2> : " << to_binary(q) << " : " << q << '\n';
		q.setbits(0x7d);  // max supernormal
		std::cout << "maxpos  cfloat<8,2> : " << to_binary(q) << " : " << q << '\n';

		half h; // uninitialized
		h.setbits(0x0001); // smallest subnormal
		std::cout << "minpos  cfloat<16,5>: " << to_binary(h) << " : " << h << '\n';
		h.setbits(0x7bff);  // max normal
		std::cout << "maxnorm cfloat<16,5>: " << to_binary(h) << " : " << h << '\n';
		h.setbits(0x7ffd);  // max supernormal
		std::cout << "maxpos  cfloat<16,5>: " << to_binary(h) << " : " << h << '\n';

		using QuarterNormal = cfloat<  8, 2, uint8_t, false, false, false>; // no sub or supernormals
		QuarterNormal qn; // uninitialized
		qn.minpos();
		std::cout << "minpos quarterNormal: " << to_binary(qn) << " : " << qn << '\n';
		qn.maxpos();
		std::cout << "maxpos quarterNormal: " << to_binary(qn) << " : " << qn << '\n';

		using halfNormal = cfloat< 16, 5, uint16_t, false, false, false>; // no sub or supernormals
		halfNormal hn; // uninitialized
		hn.minpos();
		std::cout << "minpos halfNormal   : " << to_binary(hn) << " : " << hn << '\n';
		hn.maxpos();
		std::cout << "maxpos halfNormal   : " << to_binary(hn) << " : " << hn << '\n';

		std::cout << "---\n";
	}

	// use type aliases of standard configurations
	std::cout << "+---------    Type aliases for some industry standard float configurations   --------+\n";
	{
		float f1, f2, f3;
		f1 = 1.0f;
		f2 = 1.0e-3f;
		f3 = f1 / f2;
		std::cout << "float32  : " << type_tag(f3) << '\n';
		std::cout << f1 << " / " << f2 << " = " << f3 << " : " << to_binary(f3) << '\n';

		sw::universal::bfloat_t b1(f1), b2(f2), b3;
		b3 = b1 / b2;
		std::cout << "bfloat16 : " << type_tag(b3) << '\n';
		std::cout << b1 << " / " << b2 << " = " << b3 << " : " << to_binary(b3) << '\n';

		sw::universal::half h1(f1), h2(f2), h3;
		h3 = h1 / h2;
		std::cout << "half     : " << type_tag(h3) << '\n';
		std::cout << h1 << " / " << h2 << " = " << h3 << " : " << to_binary(h3) << '\n';
	}

	// constexpr and specific values
	std::cout << "+---------    constexpr and specific values   --------+\n";
	{
		constexpr size_t nbits = 10;
		constexpr size_t es = 3;
		using Real = cfloat<nbits, es>;  // bt = uint8_t, hasSubnormals = false, hasSupernormals = false, isSaturating = false

		CONSTEXPRESSION Real a{}; // zero constexpr
		std::cout << type_tag(a) << '\n';

		CONSTEXPRESSION Real b(1.0f);  // constexpr of a native type conversion
		std::cout << to_binary(b) << " : " << b << '\n';

		CONSTEXPRESSION Real c(SpecificValue::minpos);  // constexpr of a special value in the encoding
		std::cout << to_binary(c) << " : " << c << " == minpos" << '\n';

		CONSTEXPRESSION Real d(SpecificValue::maxpos);  // constexpr of a special value in the encoding
		std::cout << to_binary(d) << " : " << d << " == maxpos" << '\n';
	}

	// set bit patterns
	std::cout << "+---------    set bit patterns API   --------+\n";
	{
		constexpr size_t nbits = 16;
		constexpr size_t es = 5;
		using Real = cfloat<nbits, es>;  // bt = uint8_t, hasSubnormals = false, hasSupernormals = false, isSaturating = false

		Real a; // uninitialized
		std::cout << type_tag(a) << '\n';

		a.setbits(0x0000);
		std::cout << to_binary(a) << " : " << a << '\n';

		a.setbits(0xAAAA);
		std::cout << to_binary(a) << " : " << a << '\n';

		a.assign(std::string("0b1.01010.1010'1010'10"));
		std::cout << to_binary(a) << " : " << a << '\n';

		a.assign(std::string("0b1.01010.10'1010'1010"));
		std::cout << to_binary(a) << " : " << a << '\n';
	}

	std::cout << "+---------    set specific values of interest   --------+\n";
	{
		cfloat<8, 2> a; // uninitialized
		std::cout << "maxpos : " << a.maxpos() << " : " << scale(a) << '\n';
		std::cout << "minpos : " << a.minpos() << " : " << scale(a) << '\n';
		std::cout << "zero   : " << a.zero() << " : " << scale(a) << '\n';
		std::cout << "minneg : " << a.minneg() << " : " << scale(a) << '\n';
		std::cout << "maxneg : " << a.maxneg() << " : " << scale(a) << '\n';
		std::cout << dynamic_range(a) << std::endl;
	}

	std::cout << "+---------    cfloat<16, 5, uint32_t, hasSubnormals, noSupernormals, notSaturating>         half-precision subnormals   --------+\n";
	{
		constexpr size_t nbits = 16;
		constexpr size_t es = 5;
		using BlockType = uint32_t;
		using Cfloat = cfloat<nbits, es, BlockType, true>;
		constexpr size_t fbits = Cfloat::fbits;
		Cfloat a, b; // uninitialized

		// enumerate the subnormals
		uint32_t pattern = 1ul;
		std::streamsize precision = std::cout.precision();
		std::cout << std::setw(nbits) << "binary" << " : " << std::setw(nbits) << "native" << " : " << std::setw(nbits) << "conversion\n";
//		std::cout << std::setprecision(3);
		std::cout << std::fixed;
		for (unsigned i = 0; i < fbits; ++i) {
			a.setbits(pattern);
			std::cout << color_print(a) << " : " << std::setw(nbits) << a << " : " << std::setw(nbits) << float(a) << '\n';
			pattern <<= 1;
		}
		// enumerate the normals
		a.setbits(0x0400);
		for (size_t i = 0; i < 30; ++i) {
			std::cout << color_print(a) << " : " << std::setw(nbits) << a << " : " << std::setw(nbits) << float(a) << " + 1ULP ";
			b = a; ++b;
			std::cout << color_print(b) << " : " << std::setw(nbits) << b << " : " << std::setw(nbits) << float(b) << '\n';
			a *= 2;
		}
		std::cout << std::setprecision(precision);
		std::cout << std::scientific;
	}
	std::cout << "+---------    cfloat<32, 8, uint32_t, hasSubnormals, noSupernormals, notSaturating>         IEEE-754 float subnormals   --------+\n";
	{
		using BlockType = uint32_t;
		float subnormal = std::nextafter(0.0f, 1.0f);
		using Cfloat = cfloat<32, 8, BlockType, true>;
		Cfloat a; // uninitialized
		blockbinary<a.fhbits, BlockType> significant;

		std::streamsize precision = std::cout.precision();
		std::cout << std::setprecision(8);
		uint32_t pattern = 0x00000001ul;
		for (unsigned i = 0; i < 24; ++i) {
			a.setbits(pattern);
			std::cout << to_binary(a, true) << " : " << a << ": ";
			pattern <<= 1;
			std::cout << color_print(subnormal) << " : " << subnormal << std::endl;
			subnormal *= 2.0f;

			if (i < 23) { // the last iteration is a normal encoding
				constexpr bool isNormal = false;
				int scale_offset = static_cast<int>(a.significant(significant, isNormal)); // significant will be in leading 1 format, so not interesting unless you are doing arithmetic
				int check = a.MIN_EXP_NORMAL - scale_offset;
				if (check != a.scale()) std::cout << a.MIN_EXP_NORMAL << " - " << scale_offset << " = (" << check << ") should be equal to " << a.scale() << std::endl;
			}
		}
		std::cout << std::setprecision(precision);
	}

	std::cout << "+---------    Subnormal exponent values   --------+\n";
	{
		// we are not using element [0] as es = 0 is not supported in the cfloat spec
		int exponents[] = {
			0, 1, 0, -2, -6, -14, -30, -62, -126, -254, -510, -1022
		};
		for (int i = 1; i < 12; ++i) {
			std::cout << "es = " << std::setw(2) << i << " = " 
				<< std::setw(5) << exponents[i] << " : " 
				<< std::setprecision(17) << subnormal_exponent[i] << std::endl;
		}
	}

	std::cout << "+---------    human-readable output for large cfloats   --------+\n";
	{
		using sp   = cfloat< 32,  8, uint32_t, true, false, false>;  // single precision
		using dp   = cfloat< 64, 11, uint32_t, true, false, false>;  // double precision
		using ep   = cfloat< 80, 11, uint32_t, true, false, false>;  // extended precision
		using qp   = cfloat<128, 15, uint8_t, true, false, false>;   // quad precision
//		using octo = cfloat<256, 18, uint8_t, true, false, false>;   // octo precision

		auto precision = std::cout.precision();

		{
			std::cout << std::setprecision(std::numeric_limits<sp>::max_digits10);
			ReportValue(sp(SpecificValue::minpos), "single precision  ");
		}

		{
			std::cout << std::setprecision(std::numeric_limits<dp>::max_digits10);
			ReportValue(dp(SpecificValue::minpos), "double precision  ");
		}
	
		// TBD: the conversion algorithm is too slow, so currently, we are casting to double
		// using the native conversion function which doesn't support precisions beyond double
		{
			std::cout << std::setprecision(std::numeric_limits<ep>::max_digits10);
			ReportValue(ep(SpecificValue::minpos), "extended precision");
		}

		{
			std::cout << std::setprecision(std::numeric_limits<qp>::max_digits10);
			ReportValue(qp(SpecificValue::minpos), "quad precision    ");
		}

		// this demonstrates that our conversion is WAY TOO SLOW: takes 4 minutes to create the representation: ETLO 1/23
//		octo o(SpecificValue::maxpos);
//		std::cout << std::fixed << o << std::scientific << '\n';
		std::cout << std::setprecision(precision);
	}

	std::cout << "+---------    special value properties cfloat vs IEEE754   --------+\n";
	{
		float fa;
		fa = NAN;
		std::cout << "qNAN   : " << to_binary(NAN) << '\n';
		std::cout << "sNAN   : " << to_binary(-NAN) << '\n';
		if (fa < 0.0f && fa > 0.0f && fa != 0.0f) {
			std::cout << "IEEE-754 is incorrectly implemented\n";
		}
		else {
			std::cout << "IEEE-754 NAN has no sign\n";
		}

		single a(fa);
		if ((a < 0.0f && a > 0.0f && a != 0.0f) || a.isneg()) {
			std::cout << "cfloat is incorrectly implemented\n";
			++nrOfFailedTestCases;
		}
		else {
			std::cout << "cfloat NAN has no sign\n";
		}
	}

	{
		using cfloat = sw::universal::cfloat<32, 8, uint32_t, true, false, false>;

		std::cout << "cfloat(INFINITY): " << cfloat(INFINITY) << "\n";
		std::cout << "cfloat(-INFINITY): " << cfloat(-INFINITY) << "\n";

		std::cout << "cfloat(std::numeric_limits<float>::infinity())  : " << cfloat(std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "cfloat(-std::numeric_limits<float>::infinity()) : " << cfloat(-std::numeric_limits<float>::infinity()) << "\n";

		std::cout << " 2 * std::numeric_limits<float>::infinity()  : " << 2 * std::numeric_limits<float>::infinity() << "\n";
		std::cout << " 2 * std::numeric_limits<cfloat>::infinity() : " << 2 * std::numeric_limits<cfloat>::infinity() << "\n";
		std::cout << "-2 * std::numeric_limits<cfloat>::infinity() : " << -2 * std::numeric_limits<cfloat>::infinity() << "\n";

		std::cout << "sw::universal::nextafter(cfloat(0), std::numeric_limits<cfloat>::infinity())  : " << sw::universal::nextafter(cfloat(-0), std::numeric_limits<cfloat>::infinity()) << "\n";
		std::cout << "std::nextafter(float(0), std::numeric_limits<float>::infinity())              : " << std::nextafter(float(-0), std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "sw::universal::nextafter(cfloat(0), -std::numeric_limits<cfloat>::infinity()) : " << sw::universal::nextafter(cfloat(0), -std::numeric_limits<cfloat>::infinity()) << "\n";
		std::cout << "std::nextafter(float(0), -std::numeric_limits<float>::infinity())             : " << std::nextafter(float(0), -std::numeric_limits<float>::infinity()) << "\n";

		std::cout << "cfloat(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_QUIET)      : " << cfloat(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_QUIET) << "\n";
		std::cout << "cfloat(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) : " << cfloat(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) << "\n";
	}

	// serialization
	{
		half h(0.5), hi(0);
		std::vector<half> v;
		unsigned N = 10;
		for (unsigned i = 0; i < N; ++i) {
			ReportValue(h, "half precision");
			v.push_back(h);
			h *= 0.5f;
		}
		std::stringstream s;
		for (auto h : v) {
			s << h << ' ';
		}
		for (unsigned i = 0; i < N; ++i) {
			s >> hi;
			ReportValue(hi, "half precision");
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
