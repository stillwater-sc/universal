// Test program for posit operator<< formatting 
#include <iostream>
#include <iomanip>
#include <sstream>
#include <universal/number/posit1/posit1.hpp>

using namespace sw::universal;

int main() {
	using Posit32 = posit<32, 2>;

	Posit32 p(3.14159265358979323846);
	Posit32 neg(-2.71828);
	Posit32 nar; nar.setnar();
	Posit32 zero(0.0);
	Posit32 small(0.000123456);
	Posit32 large(12345.6789);

	std::cout << "=== Testing posit operator<< format flags ===" << std::endl;
	std::cout << std::endl;

	// Test default formatting
	std::cout << "Default formatting:" << std::endl;
	std::cout << "  p = " << p << std::endl;
	std::cout << "  neg = " << neg << std::endl;
	std::cout << "  nar = " << nar << std::endl;
	std::cout << std::endl;

	// Test precision
	std::cout << "Precision tests:" << std::endl;
	std::cout << "  p (prec=3): " << std::setprecision(3) << p << std::endl;
	std::cout << "  p (prec=10): " << std::setprecision(10) << p << std::endl;
	std::cout << std::endl;

	// Test fixed format
	std::cout << "Fixed format:" << std::endl;
	std::cout << "  p: " << std::fixed << std::setprecision(6) << p << std::endl;
	std::cout << "  small: " << small << std::endl;
	std::cout << "  large: " << large << std::endl;
	std::cout << std::endl;

	// Test scientific format
	std::cout << "Scientific format:" << std::endl;
	std::cout << "  p: " << std::scientific << std::setprecision(4) << p << std::endl;
	std::cout << "  small: " << small << std::endl;
	std::cout << "  large: " << large << std::endl;
	std::cout << std::endl;

	// Test hexfloat format
	std::cout << "Hexfloat format:" << std::endl;
	std::cout << "  p: " << std::hexfloat << p << std::endl;
	std::cout << "  neg: " << neg << std::endl;
	std::cout << std::endl;

	// Test showpos
	std::cout << "Show positive sign:" << std::endl;
	std::cout << "  p: " << std::defaultfloat << std::showpos << p << std::endl;
	std::cout << "  neg: " << neg << std::endl;
	std::cout << "  zero: " << zero << std::endl;
	std::cout << std::noshowpos << std::endl;

	// Test showpoint
	std::cout << "Show decimal point:" << std::endl;
	std::cout << "  zero: " << std::showpoint << zero << std::endl;
	std::cout << "  large: " << std::fixed << std::setprecision(0) << large << std::endl;
	std::cout << std::noshowpoint << std::endl;

	// Test uppercase
	std::cout << "Uppercase (scientific):" << std::endl;
	std::cout << "  small: " << std::scientific << std::uppercase << small << std::endl;
	std::cout << "  p (hex): " << std::hexfloat << p << std::endl;
	std::cout << std::nouppercase << std::endl;

	// Test width and alignment
	std::cout << "Width and alignment:" << std::endl;
	std::cout << "  Right (default): |" << std::defaultfloat << std::setw(15) << p << "|" << std::endl;
	std::cout << "  Left:            |" << std::left << std::setw(15) << p << "|" << std::endl;
	std::cout << "  Internal:        |" << std::internal << std::showpos << std::setw(15) << p << "|" << std::endl;
	std::cout << std::noshowpos << std::right << std::endl;

	// Test fill character
	std::cout << "Fill character:" << std::endl;
	std::cout << "  Stars: |" << std::setfill('*') << std::setw(15) << p << "|" << std::endl;
	std::cout << "  Zeros: |" << std::setfill('0') << std::internal << std::showpos << std::setw(15) << neg << "|" << std::endl;
	std::cout << std::setfill(' ') << std::noshowpos << std::right << std::endl;

	// Test NaR formatting
	std::cout << "NaR alignment:" << std::endl;
	std::cout << "  Right: |" << std::setw(10) << nar << "|" << std::endl;
	std::cout << "  Left:  |" << std::left << std::setw(10) << nar << "|" << std::endl;
	std::cout << "  Stars: |" << std::right << std::setfill('*') << std::setw(10) << nar << "|" << std::endl;

	std::cout << std::endl;
	std::cout << "=== All formatting tests completed ===" << std::endl;

	return 0;
}
