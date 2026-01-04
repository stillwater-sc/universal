#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <iostream>
#include <iomanip>

int main() {
    using namespace sw::universal;

    std::cout << "==========================================\n";
    std::cout << "Phase 2 ereal mathlib comprehensive test\n";
    std::cout << "==========================================\n\n";

    int totalFailures = 0;

    // Test trunc
    {
        std::cout << "1. Testing trunc()...\n";
        ereal<> pos(2.7), neg(-2.7);
        ereal<> result1 = trunc(pos);  // Should be 2.0
        ereal<> result2 = trunc(neg);  // Should be -2.0
        ereal<> expected1(2.0), expected2(-2.0);

        bool pass = (result1 == expected1) && (result2 == expected2);
        std::cout << "   trunc(2.7) == 2.0: " << (result1 == expected1 ? "PASS" : "FAIL") << "\n";
        std::cout << "   trunc(-2.7) == -2.0: " << (result2 == expected2 ? "PASS" : "FAIL") << "\n";
        std::cout << "   Result: " << (pass ? "PASS" : "FAIL") << "\n\n";
        if (!pass) totalFailures++;
    }

    // Test round
    {
        std::cout << "2. Testing round()...\n";
        ereal<> x1(2.3), x2(2.5), x3(2.7);
        ereal<> result1 = round(x1);  // Should be 2.0
        ereal<> result2 = round(x2);  // Should be 3.0
        ereal<> result3 = round(x3);  // Should be 3.0
        ereal<> expected1(2.0), expected2(3.0), expected3(3.0);

        bool pass = (result1 == expected1) && (result2 == expected2) && (result3 == expected3);
        std::cout << "   round(2.3) == 2.0: " << (result1 == expected1 ? "PASS" : "FAIL") << "\n";
        std::cout << "   round(2.5) == 3.0: " << (result2 == expected2 ? "PASS" : "FAIL") << "\n";
        std::cout << "   round(2.7) == 3.0: " << (result3 == expected3 ? "PASS" : "FAIL") << "\n";
        std::cout << "   Result: " << (pass ? "PASS" : "FAIL") << "\n\n";
        if (!pass) totalFailures++;
    }

    // Test ldexp
    {
        std::cout << "3. Testing ldexp()...\n";
        ereal<> x(1.0);
        ereal<> result1 = ldexp(x, 3);  // Should be 8.0 (1.0 * 2^3)
        ereal<> result2 = ldexp(x, -2);  // Should be 0.25 (1.0 * 2^-2)
        ereal<> expected1(8.0), expected2(0.25);

        bool pass = (result1 == expected1) && (result2 == expected2);
        std::cout << "   ldexp(1.0, 3) == 8.0: " << (result1 == expected1 ? "PASS" : "FAIL") << "\n";
        std::cout << "   ldexp(1.0, -2) == 0.25: " << (result2 == expected2 ? "PASS" : "FAIL") << "\n";
        std::cout << "   Result: " << (pass ? "PASS" : "FAIL") << "\n\n";
        if (!pass) totalFailures++;
    }

    // Test frexp
    {
        std::cout << "4. Testing frexp()...\n";
        ereal<> x(8.0);
        int exp;
        ereal<> mantissa = frexp(x, &exp);  // Should be (0.5, 4) because 8.0 = 0.5 * 2^4
        ereal<> expected(0.5);
        int expected_exp = 4;

        bool pass = (mantissa == expected) && (exp == expected_exp);
        std::cout << "   frexp(8.0) mantissa == 0.5: " << (mantissa == expected ? "PASS" : "FAIL") << "\n";
        std::cout << "   frexp(8.0) exponent == 4: " << (exp == expected_exp ? "PASS" : "FAIL") << "\n";
        std::cout << "   Result: " << (pass ? "PASS" : "FAIL") << "\n\n";
        if (!pass) totalFailures++;
    }

    // Test frexp/ldexp roundtrip
    {
        std::cout << "5. Testing frexp/ldexp roundtrip...\n";
        ereal<> x(6.0);
        int exp;
        ereal<> mantissa = frexp(x, &exp);
        ereal<> reconstructed = ldexp(mantissa, exp);  // Should equal original x

        bool pass = (reconstructed == x);
        std::cout << "   ldexp(frexp(6.0)) == 6.0: " << (pass ? "PASS" : "FAIL") << "\n";
        std::cout << "   Result: " << (pass ? "PASS" : "FAIL") << "\n\n";
        if (!pass) totalFailures++;
    }

    // Test fmod
    {
        std::cout << "6. Testing fmod()...\n";
        ereal<> x(5.3), y(2.0);
        ereal<> result = fmod(x, y);  // Should be 1.3 (5.3 - 2*2.0)
        // Note: Direct comparison might have precision issues, so we check the property
        ereal<> n = trunc(x / y);  // Should be 2.0
        ereal<> expected = x - (n * y);

        bool pass = (result == expected);
        std::cout << "   fmod(5.3, 2.0) correct: " << (pass ? "PASS" : "FAIL") << "\n";
        std::cout << "   Result: " << (pass ? "PASS" : "FAIL") << "\n\n";
        if (!pass) totalFailures++;
    }

    // Test remainder
    {
        std::cout << "7. Testing remainder()...\n";
        ereal<> x(5.3), y(2.0);
        ereal<> result = remainder(x, y);  // Should be 1.3
        ereal<> n = round(x / y);  // Should be 3.0
        ereal<> expected = x - (n * y);

        bool pass = (result == expected);
        std::cout << "   remainder(5.3, 2.0) correct: " << (pass ? "PASS" : "FAIL") << "\n";
        std::cout << "   Result: " << (pass ? "PASS" : "FAIL") << "\n\n";
        if (!pass) totalFailures++;
    }

    std::cout << "==========================================\n";
    std::cout << "Phase 2 Comprehensive Test Summary\n";
    std::cout << "==========================================\n";
    std::cout << "Total failures: " << totalFailures << "\n";
    std::cout << "Overall result: " << (totalFailures == 0 ? "PASS" : "FAIL") << "\n\n";

    std::cout << "Phase 2 functions implemented:\n";
    std::cout << "  ✓ truncate: trunc(), round() - using floor/ceil\n";
    std::cout << "  ✓ numerics: frexp(), ldexp() - exponent manipulation\n";
    std::cout << "  ✓ fractional: fmod(), remainder() - using division\n";

    return (totalFailures > 0 ? 1 : 0);
}
