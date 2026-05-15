#include <iostream>
#include <iomanip>
#include <cmath>
#include <array>
#include <math/constexpr_math.hpp>

// Standalone trace of the same algorithm as CORDICAddSub<lns<32,24>>::sb_add(0)
int main() {
    constexpr unsigned N = 24;
    constexpr double LN2 = 0.6931471805599453;
    constexpr double INV_LN2 = 1.4426950408889634;

    auto is_repeat = [](unsigned i) {
        unsigned r = 4u;
        while (r <= 60u) {
            if (r == i) return true;
            r = 3u * r + 1u;
        }
        return false;
    };

    // atanh table
    std::array<double, N + 2> ATANH{};
    for (unsigned i = 1; i <= N + 1; ++i) {
        double x = std::exp2(-double(i));
        ATANH[i] = 0.5 * std::log((1.0 + x) / (1.0 - x));
    }

    auto cordic_ln_unit = [&](double w) {
        double x = w + 1.0;
        double y = w - 1.0;
        double z = 0.0;
        std::cout << std::setprecision(10);
        std::cout << "init: x=" << x << " y=" << y << " z=" << z << '\n';
        for (unsigned i = 1; i <= N; ++i) {
            double shift = std::exp2(-double(i));
            double sigma = (y < 0.0) ? 1.0 : -1.0;
            double nx = x + sigma * y * shift;
            double ny = y + sigma * x * shift;
            double nz = z - sigma * ATANH[i];
            x = nx; y = ny; z = nz;
            std::cout << "iter " << i << ": sigma=" << sigma << " atanh=" << ATANH[i]
                      << " => x=" << x << " y=" << y << " z=" << z << '\n';
            if (is_repeat(i)) {
                sigma = (y < 0.0) ? 1.0 : -1.0;
                double rx = x + sigma * y * shift;
                double ry = y + sigma * x * shift;
                double rz = z - sigma * ATANH[i];
                x = rx; y = ry; z = rz;
                std::cout << "iter " << i << " R: sigma=" << sigma
                          << " => x=" << x << " y=" << y << " z=" << z << '\n';
            }
        }
        return 2.0 * z;
    };

    double w = 2.0;
    double truth = std::log(w);
    double got = cordic_ln_unit(w);
    std::cout << "\nFinal: cordic_ln_unit(" << w << ") = " << got << " truth=ln(2)=" << truth
              << " diff=" << (got - truth) << '\n';
    return 0;
}
