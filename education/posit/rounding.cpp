// rounding.cpp: examples of rounding (projecting) values with/to posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
#include <posit>

using namespace std;
using namespace sw::unum;

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_sub
template<size_t nbits, size_t es>
void GenerateTestCase(const posit<nbits,es>& pa, const posit<nbits,es>& pb, const posit<nbits, es>& psecondary) {
	double da, db, dref;
	posit<nbits, es> pref, pdif;
	da = (double)pa;
	db = double(pb);
	dref = da - db;
	pref = dref;
	pdif = pa - pb;
	cout << setprecision(17);
	cout << "0x" << to_hex(pa.get()) << " - 0x" << to_hex(pb.get()) << "             decimal " << pa << " - " << pb << endl;
	cout << "dref   " << setw(20) << dref << endl;
	cout << "pref   " << setw(20) << pref << "  " << pref.get() << endl;
	cout << "actual " << setw(20) << pdif << "  " << pdif.get() << endl;
	cout << "second " << setw(20) << psecondary << "  " << psecondary.get() << endl;
	cout << setprecision(5);
}


/*
oparand a     op	    operand b	Theo's code	John's Mathematica code
0x9368de2d	 minus (-)	0x75bd5593	89fc9c28	0x7573e376
0xaddfa756	 minus (-)	0x51215708	a65f2827	0xc80fe5e0
0xe556134f	 minus (-)	0x42ff7483	bccb3c17	0x42ca251d
0xf7d37f28	 minus (-)	0x6301e2a4	9cfe1903	0x6301de4b
0x59f71c3c	 minus (-)	0x4df90e86	54f1b135	0x5d755fde
0xd8ce471f	 minus (-)	0x6fbd0a92	90420252	0x6fbc1776
0x18f27112	 minus (-)	0x4f5ccac7	b0b6fefd	0x4f70948b
*/
void RunHardwareValidationFailures() {
	const size_t nbits = 32;
	const size_t es = 2;
	posit<nbits, es> pa, pb, pmathematica;
	pa.set_raw_bits(0x9368de2d);	pb.set_raw_bits(0x75bd5593);	pmathematica.set_raw_bits(0x7573e376);	GenerateTestCase<nbits, es>(-pa, pb, pmathematica);
	pa.set_raw_bits(0xaddfa756);	pb.set_raw_bits(0x51215708);	pmathematica.set_raw_bits(0xc80fe5e0);	GenerateTestCase<nbits, es>(-pa, pb, pmathematica);
	pa.set_raw_bits(0xe556134f);	pb.set_raw_bits(0x42ff7483);	pmathematica.set_raw_bits(0x42ca251d);	GenerateTestCase<nbits, es>(-pa, pb, pmathematica);
	pa.set_raw_bits(0xf7d37f28);	pb.set_raw_bits(0x6301e2a4);	pmathematica.set_raw_bits(0x6301de4b);	GenerateTestCase<nbits, es>(-pa, pb, pmathematica);
	pa.set_raw_bits(0x59f71c3c);	pb.set_raw_bits(0x4df90e86);	pmathematica.set_raw_bits(0x5d755fde);	GenerateTestCase<nbits, es>(-pa, pb, pmathematica);
	pa.set_raw_bits(0xd8ce471f);	pb.set_raw_bits(0x6fbd0a92);	pmathematica.set_raw_bits(0x6fbc1776);	GenerateTestCase<nbits, es>(-pa, pb, pmathematica);
	pa.set_raw_bits(0x18f27112);	pb.set_raw_bits(0x4f5ccac7);	pmathematica.set_raw_bits(0x4f70948b);	GenerateTestCase<nbits, es>(-pa, pb, pmathematica);
}

int main(int argc, char** argv)
try {
	const size_t nbits = 32;
	const size_t es = 2;

	int nrOfFailedTestCases = 0;

	long double ld = 1.234567890123456789;
	posit<64, 2> p(ld);

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
catch (...) {
	cerr << "Caught unknown exception" << endl;
	return EXIT_FAILURE;
}
