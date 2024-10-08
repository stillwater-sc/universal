#include <iostream>
#include <iomanip>

#include <universal/number/edecimal/edecimal.hpp>

int main(int argc, char** argv)
try {
	using namespace sw::universal;
	edecimal d, e, f;

	e = 1.0f;
	f = 0xFFFF'FFFF'FFFF'FFFFull;
	d = e + f;

    std::cout << "one                      e : " << e << '\n';
    std::cout << "max unsigned long long   f : " << f << '\n';
	std::cout << "                         d : " << d << std::endl;

	return EXIT_SUCCESS;

}
catch (const char* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Unprocessed universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Unprocessed universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
