#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <vector>
#include <universal/internal/floatcascade/floatcascade.hpp>

namespace sw::universal {

// Double-Double (dd) number system using floatcascade<2>
class dd {
private:
    floatcascade<2> cascade;

public:
    // Constructors
    dd() : cascade() {}
    explicit dd(double x) : cascade(x) {}
    explicit dd(const floatcascade<2>& fc) : cascade(fc) {}
    
    // Assignment from floatcascade
    dd& operator=(const floatcascade<2>& fc) {
        cascade = fc;
        return *this;
    }
    
    // Extract floatcascade
    const floatcascade<2>& get_cascade() const { return cascade; }
    operator floatcascade<2>() const { return cascade; }

    // Conversion to other types
    explicit operator double() const { return cascade.to_double(); }
    
    // Assignment from td (triple-double) - extracts first 2 components
    template<size_t N>
    dd& operator=(const floatcascade<N>& other) {
        static_assert(N >= 2, "Cannot assign from smaller cascade");
        cascade[0] = other[0];
        cascade[1] = other[1];
        return *this;
    }

    // Arithmetic operations
    dd operator+(const dd& other) const {
        auto result = expansion_ops::add_cascades(cascade, other.cascade);
        // Compress to 2 components (this is where precision is lost)
        floatcascade<2> compressed;
        compressed[0] = result[0];
        compressed[1] = result[1] + result[2] + result[3]; // Simple compression
        return dd(compressed);
    }

    dd operator-(const dd& other) const {
        floatcascade<2> neg_other;
        neg_other[0] = -other.cascade[0];
        neg_other[1] = -other.cascade[1];
        
        auto result = expansion_ops::add_cascades(cascade, neg_other);
        floatcascade<2> compressed;
        compressed[0] = result[0];
        compressed[1] = result[1] + result[2] + result[3];
        return dd(compressed);
    }

    dd operator-() const {
        floatcascade<2> neg;
        neg[0] = -cascade[0];
        neg[1] = -cascade[1];
        return dd(neg);
    }

    // Properties
    bool is_zero() const { return cascade.iszero(); }
    int sign() const { return cascade.sign(); }

    // Stream output
    friend std::ostream& operator<<(std::ostream& os, const dd& d) {
        os << "dd(" << d.cascade << ")";
        return os;
    }
};

} // namespace sw::universal
