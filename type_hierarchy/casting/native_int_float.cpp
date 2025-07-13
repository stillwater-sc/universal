#include <cstdint>
#include <universal/native/ieee754.hpp>

int main()
try {
	double value = -160849543.8637974;

	std::cout << std::setprecision(2);
	std::cout << "uint32_t(" << value << ") = " << uint32_t(value) << '\n';
	std::cout << " int32_t(" << value << ") = " <<  int32_t(value) << '\n';
	std::cout << "double cast uint32_t(int32_t(" << value << ") = " << uint32_t(int32_t(value)) << '\n';
}
catch (...) {
	std::cerr << "Caught unexpected exception" << std::endl;
}
