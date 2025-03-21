#include <cstdint>
#include <universal/native/ieee754.hpp>

int main()
try {
	using namespace sw::universal;

	double value = -160849543.8637974;

	std::cout << std::setprecision(12);
	std::cout << "double value = " << value << '\n';
	std::cout << to_binary(value) << " : " << value << '\n';

	std::cout << "explicit conversions\n";
	std::cout << "uint32_t(" << value << ") = " << uint32_t(value) << '\n';
	std::cout << " int32_t(" << value << ") = " <<  int32_t(value) << '\n';
	std::cout << "double cast uint32_t(int32_t(" << value << ") = " << uint32_t(int32_t(value)) << '\n';

	unsigned i = unsigned(value);
	std::cout << "unsigned = " << i << " : " << to_binary(i) << '\n';
	unsigned long long ull = unsigned(value);
	std::cout << "unsigned = " << ull << " : " << to_binary(ull) << '\n';



}
catch (...) {
	std::cerr << "Caught unexpected exception" << std::endl;
}
