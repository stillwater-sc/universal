// epsilons for different number systems
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/lns/lns.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <limits>
 

/*
// Example: precision_info template
template<typename T>
void precision_info();
*/

template<typename Scalar>
void precision_info(const std::string& tag, const size_t nbits, const size_t precision) {
	std::cout << "| " << tag << " | " << nbits << " | "; 
	if constexpr (sw::universal::is_posit<Scalar>) {
		std::cout << "up to " << precision << " | ";
	} else {
		std::cout << precision << " | ";
	}
	std::cout << std::numeric_limits<Scalar>::epsilon() << " | ";
    std::cout << std::numeric_limits<Scalar>::min() << " | ";
    std::cout << std::numeric_limits<Scalar>::max() << " |\n";
}

int main()
{
	using namespace sw::universal;

    std::cout << "| Format | Bits | Significand | ε (relative step at 1) | Smallest normal | Largest |\n";
    std::cout << "|---|---|---|---|---|---|\n";

    precision_info<cfloat<64,11>>("binary64 (cfloat<64,11> or double)", 64, 53);
	precision_info<cfloat<48,8>>("binary48 (cfloat<48,8>)", 48, 40);
	precision_info<cfloat<40,8>>("binary40 (cfloat<40,8>)", 40, 32);
	precision_info<cfloat<32,8>>("binary32 (cfloat<32,8> or float)", 32, 24);
	precision_info<cfloat<24,8>>("binary24 (cfloat<24,8>)", 24, 16);
	precision_info<cfloat<16,8>>("bfloat16 (cfloat<16,8> or bfloat)", 16, 8);
	precision_info<cfloat<16,5>>("binary16 (cfloat<16,5> or FP16)", 16, 11);
	precision_info<posit<48,2>>("posit<48,2>", 48, 44);
    precision_info<posit<32,2>>("posit<32,2>", 32, 28);
	precision_info<posit<24,2>>("posit<24,2>", 24, 20);
	precision_info<posit<16,2>>("posit<16,2>", 16, 12);
	precision_info<posit<16,1>>("posit<16,1>", 16, 13);
	precision_info<lns<32, 24>>("lns<32,24>", 32, 24);
	precision_info<lns<16, 8>>("lns<16,8>", 16, 8);
	precision_info<fixpnt<32, 16>>("fixpnt<32,16>", 32, 16);
	precision_info<fixpnt<16, 8>>("fixpnt<16,8>", 16, 8);

	return EXIT_SUCCESS;	


}
