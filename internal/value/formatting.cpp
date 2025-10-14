// Test program for value<> native decimal formatter
#include <iostream>
#include <iomanip>
#include <universal/internal/value/value.hpp>

using namespace sw::universal::internal;

int main() {
	std::cout << "=== Testing value<> native decimal formatter ===" << std::endl;
	std::cout << std::endl;

	// Test with value<23> (similar to float precision)
	value<23> v1;
	v1 = 3.14159265358979;

	value<23> v2;
	v2 = -2.71828;

	value<23> v3;
	v3 = 0.0;

	value<23> v4;
	v4 = 0.000123456;

	value<23> v5;
	v5 = 12345.6789;

	value<23> v_large;
	v_large = 1.23456e20;

	value<23> v_small;
	v_small = 1.23456e-20;

	std::cout << "Default formatting:" << std::endl;
	std::cout << "  v1 (pi) = " << v1 << std::endl;
	std::cout << "  v2 (e)  = " << v2 << std::endl;
	std::cout << "  v3 (0)  = " << v3 << std::endl;
	std::cout << std::endl;

	// Test precision
	std::cout << "Precision tests:" << std::endl;
	std::cout << "  v1 (prec=3): " << std::setprecision(3) << v1 << std::endl;
	std::cout << "  v1 (prec=10): " << std::setprecision(10) << v1 << std::endl;
	std::cout << std::endl;

	// Test fixed format
	std::cout << "Fixed format:" << std::endl;
	std::cout << "  v1: " << std::fixed << std::setprecision(6) << v1 << std::endl;
	std::cout << "  v4: " << v4 << std::endl;
	std::cout << "  v5: " << v5 << std::endl;
	std::cout << std::endl;

	// Test scientific format
	std::cout << "Scientific format:" << std::endl;
	std::cout << "  v1: " << std::scientific << std::setprecision(4) << v1 << std::endl;
	std::cout << "  v4: " << v4 << std::endl;
	std::cout << "  v5: " << v5 << std::endl;
	std::cout << "  v_large: " << v_large << std::endl;
	std::cout << "  v_small: " << v_small << std::endl;
	std::cout << std::endl;

	// Test showpos
	std::cout << "Show positive sign:" << std::endl;
	std::cout << "  v1: " << std::defaultfloat << std::showpos << v1 << std::endl;
	std::cout << "  v2: " << v2 << std::endl;
	std::cout << "  v3: " << v3 << std::endl;
	std::cout << std::noshowpos << std::endl;

	// Test width and alignment
	std::cout << "Width and alignment:" << std::endl;
	std::cout << "  Right (default): |" << std::defaultfloat << std::setw(20) << v1 << "|" << std::endl;
	std::cout << "  Left:            |" << std::left << std::setw(20) << v1 << "|" << std::endl;
	std::cout << "  Internal:        |" << std::internal << std::showpos << std::setw(20) << v1 << "|" << std::endl;
	std::cout << std::noshowpos << std::right << std::endl;

	// Test fill character
	std::cout << "Fill character:" << std::endl;
	std::cout << "  Stars: |" << std::setfill('*') << std::setw(20) << v1 << "|" << std::endl;
	std::cout << "  Zeros: |" << std::setfill('0') << std::internal << std::showpos << std::setw(20) << v2 << "|" << std::endl;
	std::cout << std::setfill(' ') << std::noshowpos << std::right << std::endl;

	// Test special values
	value<23> v_inf;
	v_inf.setinf();
	value<23> v_nan;
	v_nan.setnan();

	std::cout << "Special values:" << std::endl;
	std::cout << "  inf: " << v_inf << std::endl;
	std::cout << "  nan: " << v_nan << std::endl;
	std::cout << std::endl;

	// Test with higher precision value<52> (double precision)
	std::cout << "=== Testing with value<52> (double precision) ===" << std::endl;
	value<52> vd1;
	vd1 = 3.141592653589793238;

	std::cout << "  Default: " << std::defaultfloat << vd1 << std::endl;
	std::cout << "  Prec 15: " << std::setprecision(15) << vd1 << std::endl;
	std::cout << "  Fixed 12: " << std::fixed << std::setprecision(12) << vd1 << std::endl;
	std::cout << "  Scientific 10: " << std::scientific << std::setprecision(10) << vd1 << std::endl;

	std::cout << std::endl;
	std::cout << "=== All value<> decimal formatting tests completed ===" << std::endl;

	return 0;
}
