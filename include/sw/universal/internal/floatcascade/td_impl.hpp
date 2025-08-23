#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <vector>
#include <universal/internal/floatcascade/floatcascade.hpp>

namespace sw::universal {

// Triple-Double (td) number system using floatcascade<3>
class td {
private:
    floatcascade<3> cascade;

public:
    // Constructors
    td() : cascade() {}
    explicit td(double x) : cascade(x) {}
    explicit td(const floatcascade<3>& fc) : cascade(fc) {}
    
    // Constructor from dd (zero-extends to 3 components)
    explicit td(const dd& d) : cascade() {
        floatcascade<2> dd_cascade = d.get_cascade();
        cascade[0] = dd_cascade[0];
        cascade[1] = dd_cascade[1];
        cascade[2] = 0.0;
    }

    // Assignment from floatcascade
    td& operator=(const floatcascade<3>& fc) {
        cascade = fc;
        return *this;
    }
    
    // Extract floatcascade
    const floatcascade<3>& get_cascade() const { return cascade; }
    operator floatcascade<3>() const { return cascade; }

    // Conversion to other types
    explicit operator double() const { return cascade.to_double(); }
    
    // Assignment from dd
    td& operator=(const dd& other) {
        floatcascade<2> other_cascade = other.get_cascade();
        cascade[0] = other_cascade[0];
        cascade[1] = other_cascade[1];
        cascade[2] = 0.0;
        return *this;
    }

    // Arithmetic operations
    td operator+(const td& other) const {
        auto result = expansion_ops::add_cascades(cascade, other.cascade);
        // Compress to 3 components
        floatcascade<3> compressed;
        compressed[0] = result[0];
        compressed[1] = result[1];
        compressed[2] = result[2] + result[3] + result[4] + result[5]; // Simple compression
        return td(compressed);
    }

    td operator-(const td& other) const {
        floatcascade<3> neg_other;
        neg_other[0] = -other.cascade[0];
        neg_other[1] = -other.cascade[1];
        neg_other[2] = -other.cascade[2];
        
        auto result = expansion_ops::add_cascades(cascade, neg_other);
        floatcascade<3> compressed;
        compressed[0] = result[0];
        compressed[1] = result[1];
        compressed[2] = result[2] + result[3] + result[4] + result[5];
        return td(compressed);
    }

    td operator-() const {
        floatcascade<3> neg;
        neg[0] = -cascade[0];
        neg[1] = -cascade[1];
        neg[2] = -cascade[2];
        return td(neg);
    }

    // Properties
    bool is_zero() const { return cascade.is_zero(); }
    int sign() const { return cascade.sign(); }

    // Stream output
    friend std::ostream& operator<<(std::ostream& os, const td& t) {
        os << "td(" << t.cascade << ")";
        return os;
    }
};

} // namespace sw::universal
