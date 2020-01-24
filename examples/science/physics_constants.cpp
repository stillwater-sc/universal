// physics_constants.cpp: experiments with posit representations of important constants in physics 
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/posit/posit>

/*
The 2019 redefinition of the SI base units came into force on 20 May 2019,[1][2] the 144th anniversary 
of the Metre Convention. In the redefinition, four of the seven SI base units 
           – the kilogram, ampere, kelvin, and mole –
were redefined by setting exact numerical values for 
   - the Planck constant (h), 
   - the elementary electric charge (e), 
   - the Boltzmann constant (k), and 
   - the Avogadro constant (NA), 
 respectively. 
 
 The second, metre, and candela were already defined by physical constants and were subject to correction 
 to their definitions. The new definitions aimed to improve the SI without changing the value of any units, 
 ensuring continuity with existing measurements.[3][4] In November 2018, the 26th General Conference on 
 Weights and Measures (CGPM) unanimously approved these changes,[5][6] which the International Committee 
 for Weights and Measures (CIPM) had proposed earlier that year after determining that previously agreed 
 conditions for the change had been met.[7]:23 These conditions were satisfied by a series of experiments 
 that measured the constants to high accuracy relative to the old SI definitions, and were the culmination 
 of decades of research.

 Following the successful 1983 redefinition of the metre in terms of an exact numerical value for the 
 speed of light, the BIPM's Consultative Committee for Units (CCU) recommended and the BIPM proposed that 
 four further constants of nature should be defined to have exact values. These are:

The Planck constant h is exactly 6.62607015×10−34 joule-second (J⋅s).
The elementary charge e is exactly 1.602176634×10−19 coulomb (C).
The Boltzmann constant k is exactly 1.380649×10−23 joule per kelvin (J⋅K−1).
The Avogadro constant NA is exactly 6.02214076×1023 reciprocal mole (mol−1).

These constants are described in the 2006 version of the SI manual but in that version, the latter three 
are defined as "constants to be obtained by experiment" rather than as "defining constants". The redefinition 
retains unchanged the numerical values associated with the following constants of nature:

The speed of light c is exactly 299792458 metres per second (m⋅s−1);
The ground state hyperfine structure transition frequency of the caesium-133 atom ΔνCs is exactly 9192631770 hertz (Hz);
The luminous efficacy Kcd of monochromatic radiation of frequency 540×1012 Hz (540 THz) – a frequency 
of green-colored light at approximately the peak sensitivity of the human eye – is exactly 683 lumens per watt (lm⋅W−1).
The seven definitions above are rewritten below with the derived units (joule, coulomb, hertz, lumen, and watt) expressed 
in terms of the seven base units; second, metre, kilogram, ampere, kelvin, mole, and candela, according to the 9th SI Brochure. 
In the list that follows, the symbol sr stands for the dimensionless unit steradian.

ΔνCs = Δν(133Cs)hfs = 9192631770 s−1
c = 299792458 m⋅s−1
h = 6.62607015×10−34 kg⋅m2⋅s−1
e = 1.602176634×10−19 A⋅s
k = 1.380649×10−23 kg⋅m2⋅K−1⋅s−2
NA = 6.02214076×1023 mol−1
Kcd = 683 cd⋅sr⋅s3⋅kg−1⋅m−2
As part of the redefinition, the international prototype kilogram was retired and definitions of the kilogram, 
the ampere, and the kelvin were replaced. The definition of the mole was revised. These changes have the effect 
of redefining the SI base units, though the definitions of the SI derived units in terms of the base units remain the same.
 */

template<typename Scalar>
void Represent(std::ostream& ostr, Scalar s, std::streamsize precision = 17) {
	using namespace std;
	// preserve the existing ostream precision
	auto old_precision = cout.precision();
	ostr << setprecision(precision);

	ostr << s << endl;

	// restore the previous ostream precision
	ostr << setprecision(old_precision);
}

void Sample(std::ostream& ostr, long double constant) {
	using namespace sw::unum;
	Represent(ostr << minmax_range< long double  >() << " : ", constant, 23);
	Represent(ostr << minmax_range< double       >() << " : ", double(constant), 15);
	Represent(ostr << minmax_range< float        >() << " : ", float(constant), 6);
	Represent(ostr << minmax_range< posit<32, 2> >() << " : ", posit<32, 2>(constant));
	Represent(ostr << minmax_range< posit<32, 3> >() << " : ", posit<32, 3>(constant));
	Represent(ostr << minmax_range< posit<40, 3> >() << " : ", posit<40, 3>(constant));
	Represent(ostr << minmax_range< posit<48, 3> >() << " : ", posit<48, 3>(constant));
	Represent(ostr << minmax_range< posit<64, 3> >() << " : ", posit<64, 3>(constant));
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	//constexpr size_t nbits = 32;
	//constexpr size_t es = 2;
	//using Posit = posit<nbits,es>;

	// print detailed bit-level computational intermediate results
	// bool verbose = false;


	long double h = 6.62607015e-34;  // (J⋅s)
	long double e = 1.602176634e-19; // (C)
	long double k = 1.380649e-23;    // (J⋅K−1)
	long double NA = 6.02214076e23;  // (mol−1)


	cout << "The Planck constant h is exactly 6.62607015*10^-34 joule - second.\n";
	Sample(cout, h);
	cout << endl;

	cout << "The elementary charge e is exactly 1.602176634*10^-19 coulomb.\n";
	Sample(cout, e);
	cout << endl;

	cout << "The Boltzmann constant k is exactly 1.380649*10^-23 joule per kelvin.\n";
	Sample(cout, k);
	cout << endl;

	cout << "The Avogadro constant NA is exactly 6.02214076*10^+23 reciprocal mole.\n";
	Sample(cout, NA);
	cout << endl;

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
