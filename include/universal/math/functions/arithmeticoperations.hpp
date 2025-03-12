#ifndef UNIVERSAL_MATH_FUNCTIONS_ARITHMETICOPERATIONS_HPP
#define UNIVERSAL_MATH_FUNCTIONS_ARITHMETICOPERATIONS_HPP

#include <universal/number/posit/posit.hpp>
#include <string_view>

namespace sw::universal {

template<typename NumberType, char Op>
struct OperationStruc {
    static constexpr char getOperationChar() { return Op; }

    static constexpr std::string_view getOperationString() {
        if (Op == '+') return "addition";
        if (Op == '-') return "subtraction";
        if (Op == '*') return "multiplication";
        if (Op == '/') return "division";
        return "unknown";
    }

    static NumberType primary(NumberType a, NumberType b) {
        if (Op == '+') return a + b;
        if (Op == '-') return a - b;
        if (Op == '*') return a * b;
        if (Op == '/') return a / b;
        return NumberType(0);
    }

    static NumberType inverse(NumberType a, NumberType b) {
        if (Op == '+') return a - b;
        if (Op == '-') return a + b;
        if (Op == '*') return a / b;
        if (Op == '/') return a * b;
        return NumberType(0);
    }

    NumberType executeOperation(NumberType a, NumberType b) const {
        return primary(a, b);
    }

    NumberType executeInverseOperation(NumberType a, NumberType b) const {
        return inverse(a, b);
    }
};

} // namespace sw::universal

#endif