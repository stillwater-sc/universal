// api.cpp: test suite runner for class interface tests of the decimal fixed-point dfixpnt type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the dfixpnt template environment
#define DFIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/dfixpnt/dfixpnt.hpp>

#include <universal/verification/test_suite.hpp>


int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "dfixpnt decimal fixed-point API";
	std::string test_tag    = "dfixpnt API";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

    // triviality check
    ReportTrivialityOfType<dfixpnt<8, 3>>();

    using Dfp = dfixpnt<8, 3>;

    // default construction
    {
        Dfp a;
        (void)a; // no value guarantee for trivially constructed type
    }

    // SpecificValue construction
    {
        Dfp z(SpecificValue::zero);
        if (!z.iszero()) {
            ++nrOfFailedTestCases;
            if (reportTestCases) ReportTestResult(1, "SpecificValue::zero", test_tag);
        }
    }

    // type_tag
    {
        Dfp a;
        std::string tag = type_tag(a);
        if (tag.find("dfixpnt") == std::string::npos) {
            ++nrOfFailedTestCases;
            if (reportTestCases) ReportTestResult(1, "type_tag", test_tag);
        }
    }

    // assign
    {
        using Dfp = dfixpnt<8, 3>;
        Dfp a;
        std::cout << "type tag: " << type_tag(a) << '\n';
        std::cout << "type field: " << type_field(a) << '\n';

        a = 123;
        std::cout << "a = 123 : " << a << '\n';

        a.assign("456.789");
        std::cout << "a = 456.789 : " << a << '\n';
        std::cout << "binary: " << to_binary(a) << '\n';
    }

    // integer construction and to_string
    {
        Dfp a(42);
        if (a.to_string() != "42.000") {
            ++nrOfFailedTestCases;
            if (reportTestCases) ReportTestResult(1, "int construction 42", test_tag);
        }
    }
    {
        Dfp a(-7);
        if (a.to_string() != "-7.000") {
            ++nrOfFailedTestCases;
            if (reportTestCases) ReportTestResult(1, "int construction -7", test_tag);
        }
    }

    // double construction
    {
        Dfp a(3.14);
        if (a.to_string() != "3.140") {
            ++nrOfFailedTestCases;
            if (reportTestCases) ReportTestResult(1, "double construction 3.14", test_tag);
        }
    }

    // string assign
    {
        Dfp a;
        a.assign("99.125");
        if (a.to_string() != "99.125") {
            ++nrOfFailedTestCases;
            if (reportTestCases) ReportTestResult(1, "string assign 99.125", test_tag);
        }
    }

    // stream I/O roundtrip
    {
        Dfp a;
        a.assign("12.345");
        std::stringstream ss;
        ss << a;
        Dfp b;
        ss >> b;
        if (a != b) {
            ++nrOfFailedTestCases;
            if (reportTestCases) ReportTestResult(1, "stream I/O roundtrip", test_tag);
        }
    }

    // digit access
    {
        Dfp a;
        a.assign("456.789");
        // digit(0) = 9, digit(1) = 8, digit(2) = 7, digit(3) = 6, digit(4) = 5, digit(5) = 4
        if (a.digit(0) != 9 || a.digit(1) != 8 || a.digit(2) != 7) {
            ++nrOfFailedTestCases;
            if (reportTestCases) ReportTestResult(1, "digit access fractional", test_tag);
        }
        if (a.digit(3) != 6 || a.digit(4) != 5 || a.digit(5) != 4) {
            ++nrOfFailedTestCases;
            if (reportTestCases) ReportTestResult(1, "digit access integer", test_tag);
        }
    }

    // +0 == -0
    {
        Dfp pos_zero(SpecificValue::zero);
        Dfp neg_zero(SpecificValue::zero);
        neg_zero.setsign(true);
        if (pos_zero != neg_zero) {
            ++nrOfFailedTestCases;
            if (reportTestCases) ReportTestResult(1, "+0 == -0", test_tag);
        }
    }

    // BCD/BID/DPD encoding: all three encodings produce the same results
    {
        using BCD8 = dfixpnt<8, 3, DecimalEncoding::BCD>;
        BCD8 bcd(123);
        std::cout << "BCD  : " << to_binary(bcd) << " : " << bcd << '\n';
        if (bcd.to_string() != "123.000") {
            ++nrOfFailedTestCases;
            if (reportTestCases) ReportTestResult(1, "BCD encoding", test_tag);
        }
        using BID8 = dfixpnt<8, 3, DecimalEncoding::BID>;
        BID8 bid(123);
        std::cout << "BID  : " << to_binary(bid) << " : " << bid << '\n';
        if (bid.to_string() != "123.000") {
            ++nrOfFailedTestCases;
            if (reportTestCases) ReportTestResult(1, "BID encoding", test_tag);
        }
        using DPD8 = dfixpnt<8, 3, DecimalEncoding::DPD>;
        DPD8 dpd(123);
        std::cout << "DPD  : " << to_binary(dpd) << " : " << dpd << '\n';
        if (dpd.to_string() != "123.000") {
            ++nrOfFailedTestCases;
            if (reportTestCases) ReportTestResult(1, "DPD encoding", test_tag);
        }
    }

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
