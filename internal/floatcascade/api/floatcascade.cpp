// api.cpp: application programming interface tests for double-double (dd) number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <universal/internal/floatcascade/floatcascade.hpp>

using namespace sw::universal;

class FloatCascadeTestSuite {
private:
    int tests_run = 0;
    int tests_passed = 0;

    void assert_test(bool condition, const std::string& test_name) {
        tests_run++;
        if (condition) {
            tests_passed++;
            std::cout << "OK  " << test_name << std::endl;
        }
        else {
            std::cout << "NOT" << test_name << " FAILED\n";
        }
    }

    template<typename T>
    bool nearly_equal(T a, T b, T epsilon = 1e-15) {
        return std::abs(a - b) <= epsilon * std::max(std::abs(a), std::abs(b));
    }

public:
    void run_all_tests() {
        std::cout << "Running FloatCascade Unit Tests\n";
        std::cout << "================================\n";

        test_construction();
        test_component_access();
        test_conversion_operations();
        test_ordering_verification();
        test_sign_detection();
        test_zero_detection();
        test_expansion_ops();
        test_edge_cases();
        test_precision_characteristics();

        std::cout << "\nTest Summary: " << tests_passed << "/" << tests_run << " passed\n";
        if (tests_passed == tests_run) {
            std::cout << "All tests PASSED!\n";
        }
        else {
            std::cout << (tests_run - tests_passed) << " tests FAILED!\n";
        }
    }

    void test_construction() {
        std::cout << "\n--- Construction Tests ---\n";

        // Default constructor
        floatcascade<3> fc1;
        assert_test(fc1.iszero(), "Default constructor creates zero");
        assert_test(fc1[0] == 0.0 && fc1[1] == 0.0 && fc1[2] == 0.0,
            "Default constructor zeros all components");

        // Single double constructor
        floatcascade<3> fc2(1.5);
        assert_test(fc2[0] == 1.5, "Single double constructor sets [0] component");
        assert_test(fc2[1] == 0.0 && fc2[2] == 0.0, "Single double constructor zeros other components");
        assert_test(fc2.to_double() == 1.5, "Single double constructor preserves value");

        // Array constructor
        std::array<double, 3> arr = { 1.0, 0.1, 0.01 };
        floatcascade<3> fc3(arr);
        assert_test(fc3[0] == 1.0 && fc3[1] == 0.1 && fc3[2] == 0.01,
            "Array constructor preserves order");
        assert_test(nearly_equal(fc3.to_double(), 1.11), "Array constructor sums correctly");

        // Copy constructor
        floatcascade<3> fc4(fc3);
        assert_test(fc4[0] == fc3[0] && fc4[1] == fc3[1] && fc4[2] == fc3[2],
            "Copy constructor works");
    }

    void test_component_access() {
        std::cout << "\n--- Component Access Tests ---\n";

        floatcascade<4> fc;

        // Write access
        fc[0] = 10.0;
        fc[1] = 1.0;
        fc[2] = 0.1;
        fc[3] = 0.01;

        assert_test(fc[0] == 10.0, "Component write/read [0]");
        assert_test(fc[1] == 1.0, "Component write/read [1]");
        assert_test(fc[2] == 0.1, "Component write/read [2]");
        assert_test(fc[3] == 0.01, "Component write/read [3]");

        // Size
        assert_test(fc.size() == 4, "Size method returns correct value");

        // Data access
        const auto& data = fc.data();
        assert_test(data[0] == 10.0 && data[3] == 0.01, "Data access works");

        // Set method
        fc.set(42.0);
        assert_test(fc[0] == 42.0 && fc[1] == 0.0, "Set method works");
    }

    void test_conversion_operations() {
        std::cout << "\n--- Conversion Tests ---\n";

        // to_double()
        floatcascade<3> fc;
        fc[0] = 1.0;
        fc[1] = 0.5;
        fc[2] = 0.25;

        assert_test(fc.to_double() == 1.75, "to_double() sums components");

        // Test with mixed signs
        fc[1] = -0.1;
        assert_test(nearly_equal(fc.to_double(), 1.15), "to_double() handles mixed signs");

        // Test with zero components
        fc[2] = 0.0;
        assert_test(nearly_equal(fc.to_double(), 0.9), "to_double() handles zeros");
    }

    void test_ordering_verification() {
        std::cout << "\n--- Ordering Verification Tests ---\n";

        // Test that we're using decreasing magnitude order (most significant first)
        floatcascade<3> fc;
        fc[0] = 1.0;      // Most significant
        fc[1] = 1e-8;     // Medium 
        fc[2] = 1e-16;    // Least significant

        // The first component should be the primary approximation
        assert_test(fc[0] == 1.0, "First component is most significant");

        // For typical usage, |fc[0]| >= |fc[1]| >= |fc[2]|
        assert_test(std::abs(fc[0]) >= std::abs(fc[1]), "Component 0 >= Component 1 in magnitude");
        assert_test(std::abs(fc[1]) >= std::abs(fc[2]), "Component 1 >= Component 2 in magnitude");

        // Sign should come from first non-zero component (which should be [0] in typical case)
        assert_test(fc.sign() == 1, "Positive sign detected from first component");

        fc[0] = -2.0;
        assert_test(fc.sign() == -1, "Negative sign detected from first component");
    }

    void test_sign_detection() {
        std::cout << "\n--- Sign Detection Tests ---\n";

        floatcascade<3> fc;

        // Zero case
        assert_test(fc.sign() == 0, "Zero cascade has zero sign");

        // Positive cases
        fc[0] = 1.0;
        assert_test(fc.sign() == 1, "Positive first component");

        fc[0] = 0.0;
        fc[1] = 0.5;
        assert_test(fc.sign() == 1, "Positive second component with zero first");

        fc[1] = 0.0;
        fc[2] = 0.1;
        assert_test(fc.sign() == 1, "Positive third component with zero others");

        // Negative cases
        fc.clear();
        fc[0] = -1.0;
        assert_test(fc.sign() == -1, "Negative first component");

        fc[0] = 0.0;
        fc[1] = -0.5;
        assert_test(fc.sign() == -1, "Negative second component with zero first");

        // Mixed signs - first non-zero wins
        fc[0] = 1.0;
        fc[1] = -2.0;
        fc[2] = -3.0;
        assert_test(fc.sign() == 1, "First non-zero component determines sign");
    }

    void test_zero_detection() {
        std::cout << "\n--- Zero Detection Tests ---\n";

        floatcascade<3> fc;
        assert_test(fc.iszero(), "Default constructed cascade is zero");

        fc[1] = 1.0;
        assert_test(!fc.iszero(), "Non-zero cascade detected");

        fc.clear();
        assert_test(fc.iszero(), "Clear() makes cascade zero");

        // Test with very small numbers (should not be considered zero)
        fc[0] = 1e-100;
        assert_test(!fc.iszero(), "Very small number is not zero");
    }

    void test_expansion_ops() {
        std::cout << "\n--- Expansion Operations Tests ---\n";

        // Test two_sum
        double a = 1.0, b = 1e-16;
        double x, y;
        expansion_ops::two_sum(a, b, x, y);

        assert_test(x == 1.0, "two_sum main result");
        assert_test(y == 1e-16, "two_sum error term");
        assert_test(nearly_equal(x + y, a + b), "two_sum exactness");

        // Test fast_two_sum (requires |a| >= |b|)
        expansion_ops::fast_two_sum(a, b, x, y);
        assert_test(nearly_equal(x + y, a + b), "fast_two_sum exactness");

        // Test grow_expansion
        floatcascade<2> fc2;
        fc2[0] = 1.0;
        fc2[1] = 1e-8;

        floatcascade<3> fc3 = expansion_ops::grow_expansion(fc2, 1e-16);
        assert_test(fc3.size() == 3, "grow_expansion increases size");
        assert_test(nearly_equal(fc3.to_double(), fc2.to_double() + 1e-16),
            "grow_expansion preserves value");
    }

    void test_edge_cases() {
        std::cout << "\n--- Edge Cases Tests ---\n";

        // Very large numbers
        floatcascade<2> fc_large(1e100);
        assert_test(fc_large[0] == 1e100, "Large number storage");
        assert_test(fc_large.to_double() == 1e100, "Large number conversion");

        // Very small numbers
        floatcascade<2> fc_small(1e-100);
        assert_test(fc_small[0] == 1e-100, "Small number storage");
        assert_test(fc_small.to_double() == 1e-100, "Small number conversion");

        // Infinity
        floatcascade<2> fc_inf(std::numeric_limits<double>::infinity());
        assert_test(std::isinf(fc_inf[0]), "Infinity storage");
        assert_test(std::isinf(fc_inf.to_double()), "Infinity conversion");

        // NaN
        floatcascade<2> fc_nan(std::numeric_limits<double>::quiet_NaN());
        assert_test(std::isnan(fc_nan[0]), "NaN storage");
        assert_test(std::isnan(fc_nan.to_double()), "NaN conversion");
    }

    void test_precision_characteristics() {
        std::cout << "\n--- Precision Characteristics Tests ---\n";

        // Test that we can represent 1 + epsilon where epsilon < machine epsilon
        double epsilon = std::numeric_limits<double>::epsilon();
        floatcascade<2> fc;
        fc[0] = 1.0;
        fc[1] = epsilon / 2.0;  // Smaller than machine epsilon

        double sum = fc.to_double();
        assert_test(sum == 1.0 + epsilon / 2.0, "Can represent sub-epsilon precision");

        // Test typical cascade pattern: each component ~1 ULP of previous
        floatcascade<3> precise;
        precise[0] = 1.0;
        precise[1] = epsilon;           // 1 ULP at scale of 1.0
        precise[2] = epsilon * epsilon; // 1 ULP at scale of epsilon

        assert_test(precise.to_double() > 1.0, "Cascade increases precision");
        assert_test(precise[0] > precise[1], "Decreasing magnitude property");
        assert_test(std::abs(precise[1]) > std::abs(precise[2]), "Decreasing magnitude property");
    }
};

// Demo function to show floatcascade in action
void demonstrate_floatcascade() {
    std::cout << "\n\nFloatCascade Demonstration\n";
    std::cout << "==========================\n";
    std::cout << std::setprecision(17);

    // Show the classic problem: 1.0 + machine_epsilon/2
    double eps = std::numeric_limits<double>::epsilon();
    double regular_sum = 1.0 + eps / 2.0;

    floatcascade<2> cascade_sum;
    cascade_sum[0] = 1.0;
    cascade_sum[1] = eps / 2.0;

    std::cout << "Machine epsilon: " << eps << std::endl;
    std::cout << "Regular double 1.0 + eps/2: " << regular_sum << std::endl;
    std::cout << "FloatCascade 1.0 + eps/2: " << cascade_sum.to_double() << std::endl;
    std::cout << "Cascade components: [" << cascade_sum[0] << ", " << cascade_sum[1] << "]" << std::endl;

    // Show component ordering
    floatcascade<4> demo;
    demo[0] = 1.234567890123456;     // Most significant
    demo[1] = 9.876543210987654e-9;  // High precision correction
    demo[2] = 1.111111111111111e-17; // Ultra precision correction  
    demo[3] = 5.555555555555555e-26; // Maximum precision correction

    std::cout << "\nComponent ordering demonstration:" << std::endl;
    std::cout << demo << std::endl;
    std::cout << "As double: " << demo.to_double() << std::endl;

    // Show what happens when we lose precision
    double truncated = demo[0];  // Just the first component
    std::cout << "First component only: " << truncated << std::endl;
    std::cout << "Precision gain: " << (demo.to_double() - truncated) << std::endl;
}

int main() {
    using namespace sw::universal;
    
    size_t N = 5;
    for (int i = N - 1; i >= 0; --i) {
        std::cout << i << '\n';
	}
    for (size_t i = N; i-- > 0; ) {
        std::cout << i << '\n';
    }
    return 0;

    FloatCascadeTestSuite tests;
    tests.run_all_tests();

    demonstrate_floatcascade();
        
    return EXIT_SUCCESS;
}
