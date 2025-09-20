#include <iostream>
#include <universal/internal/blockbinary/blockbinary.hpp>

using namespace sw::universal;

int main() {
    std::cout << "Testing blockbinary decimal converter\n";
    std::cout << "====================================\n\n";

    // Test 32-bit signed blockbinary
    {
        std::cout << "32-bit signed blockbinary tests:\n";
        blockbinary<32, uint8_t, BinaryNumberType::Signed> a(0);
        std::cout << "0: " << a << std::endl;

        a = 123;
        std::cout << "123: " << a << std::endl;

        a = -456;
        std::cout << "-456: " << a << std::endl;

        a = 2147483647; // max positive 32-bit signed
        std::cout << "2147483647: " << a << std::endl;

        a = -2147483648LL; // max negative 32-bit signed
        std::cout << "-2147483648: " << a << std::endl;
        std::cout << std::endl;
    }

    // Test 32-bit unsigned blockbinary
    {
        std::cout << "32-bit unsigned blockbinary tests:\n";
        blockbinary<32, uint8_t, BinaryNumberType::Unsigned> b(0);
        std::cout << "0: " << b << std::endl;

        b = 123;
        std::cout << "123: " << b << std::endl;

        b = 4294967295UL; // max 32-bit unsigned
        std::cout << "4294967295: " << b << std::endl;
        std::cout << std::endl;
    }

    // Test 64-bit signed blockbinary
    {
        std::cout << "64-bit signed blockbinary tests:\n";
        blockbinary<64, uint8_t, BinaryNumberType::Signed> c(0);
        std::cout << "0: " << c << std::endl;

        c = 9223372036854775807LL; // max positive 64-bit signed
        std::cout << "9223372036854775807: " << c << std::endl;

        c = -9223372036854775807LL - 1; // max negative 64-bit signed
        std::cout << "-9223372036854775808: " << c << std::endl;
        std::cout << std::endl;
    }

    // Test 128-bit blockbinary (larger than 64 bits)
    {
        std::cout << "128-bit signed blockbinary tests:\n";
        blockbinary<128, uint8_t, BinaryNumberType::Signed> d(0);
        std::cout << "0: " << d << std::endl;

        d = 123456789;
        std::cout << "123456789: " << d << std::endl;

        d = -987654321;
        std::cout << "-987654321: " << d << std::endl;
        std::cout << std::endl;
    }

    std::cout << "All tests completed.\n";
    return 0;
}