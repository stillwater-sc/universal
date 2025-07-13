#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <cmath>
#include <tuple>
#include <algorithm>
#include <numeric>
#include <iomanip>

#include <universal/number/erational/erational.hpp>

/*
This application demonstrates several interesting aspects of rational number arithmetic :

Musical Scale Generation :
    Creates just-intonation scales where all intervals are represented as exact ratios, showing how they differ from equal temperament.

Harmonic Series :
    Generates the harmonic series for any fundamental frequency, demonstrating how natural harmonics form rational relationships.

Just Interval Discovery :
    Finds all possible simple frequency ratios within given limits, useful for exploring microtonal music and alternative tuning systems.

Key features :

- Exact representation of musical intervals using rational numbers
- Conversion between frequency ratios and cents, a logarithmic measure of musical intervals
- Calculation of deviation from equal temperament
- Support for arbitrary base frequencies
- Reduction of ratios to simplest form

The output shows :

- Pure frequency ratios for common musical intervals
- Actual frequencies in Hz
- Deviation from equal temperament in cents

This application is particularly interesting because :

- It shows how rational numbers can represent musical relationships exactly
- It demonstrates why certain intervals sound "pure" (simple ratios) while others sound dissonant
- It helps explain historical tuning systems and why equal temperament was developed
- It can be used to explore microtonality and alternative tuning systems
*/

// Forward declarations
class Rational;
template<typename RationalType> class Note;
template<typename RationalType> class HarmonicsCalculator;

// Rational number class for exact representation
class Rational {
private:
    int64_t num;
    int64_t den;

    void reduce() {
        if (den < 0) {
            num = -num;
            den = -den;
        }
        int64_t g = std::gcd(std::abs(num), den);
        num /= g;
        den /= g;
    }

public:
    explicit operator double() const noexcept { return toDouble(); }

    Rational(int64_t n = 0, int64_t d = 1) : num(n), den(d) {
        if (d == 0) throw std::invalid_argument("Denominator cannot be zero");
        reduce();
    }

    Rational operator*(const Rational& other) const {
        return Rational(num * other.num, den * other.den);
    }

    double toDouble() const {
        return static_cast<double>(num) / den;
    }

    std::pair<int64_t, int64_t> toPair() const {
        return {num, den};
    }

    std::string toString() const {
        return std::to_string(num) + "/" + std::to_string(den);
    }
};

std::ostream& operator<<(std::ostream& ostr, const Rational& r) {
    return ostr << r.toString();
}

// Musical note class
template<typename RationalType>
class Note {
public:
    std::string name;
    RationalType frequency;
    double cents;

    Note(const std::string& n, const RationalType& f, double c)
        : name(n), frequency(f), cents(c) {}
};

template<typename RationalType>
class HarmonicsCalculator {
private:
    double baseFrequency;
    std::map<std::string, RationalType> perfectRatios;

    static double ratioToCents(const RationalType& ratio) {
        return 1200 * std::log2(double(ratio));
    }

public:
    HarmonicsCalculator(double base = 440.0) : baseFrequency(base) {
        perfectRatios = {
            {"unison", RationalType(1, 1)},
            {"minor_second", RationalType(16, 15)},
            {"major_second", RationalType(9, 8)},
            {"minor_third", RationalType(6, 5)},
            {"major_third", RationalType(5, 4)},
            {"perfect_fourth", RationalType(4, 3)},
            {"tritone", RationalType(45, 32)},
            {"perfect_fifth", RationalType(3, 2)},
            {"minor_sixth", RationalType(8, 5)},
            {"major_sixth", RationalType(5, 3)},
            {"minor_seventh", RationalType(9, 5)},
            {"major_seventh", RationalType(15, 8)},
            {"octave", RationalType(2, 1)}
        };
    }

    std::vector<Note<RationalType>> generateScale(const RationalType& rootRatio,
                                    const std::vector<std::string>& intervals) {
        std::vector<Note<RationalType>> scale;
        RationalType currentRatio = rootRatio;

        for (size_t i = 0; i < intervals.size(); ++i) {
            const auto& interval = intervals[i];
            RationalType ratio = perfectRatios[interval];
            currentRatio = rootRatio * ratio;
            double cents = ratioToCents(currentRatio);
            cents = std::fmod(cents, 1200.0);
            double etCents = (i + 1) * 100.0;
            double centsDeviation = cents - etCents;

            scale.emplace_back(interval, currentRatio, centsDeviation);
        }

        return scale;
    }

    std::vector<Note<RationalType>> findHarmonics(const RationalType& fundamental, int maxOrder = 8) {
        std::vector<Note<RationalType>> harmonics;
        for (int i = 1; i <= maxOrder; ++i) {
            auto [num, den] = fundamental.toPair();
            RationalType ratio(i * num, den);
            double cents = std::fmod(ratioToCents(ratio), 1200.0);
            double etCents = std::round(cents / 100.0) * 100.0;
            double centsDeviation = cents - etCents;

            harmonics.emplace_back(
                "Harmonic " + std::to_string(i),
                ratio,
                centsDeviation
            );
        }
        return harmonics;
    }

    std::vector<Note<RationalType>> findJustIntervals(int maxNumerator = 16, int maxDenominator = 16) {
        std::vector<Note<RationalType>> intervals;
        std::set<std::pair<int64_t, int64_t>> seenRatios;

        for (int num = 1; num <= maxNumerator; ++num) {
            for (int den = 1; den <= maxDenominator; ++den) {
                if (std::gcd(num, den) == 1) {
                    RationalType ratio(num, den);
                    double cents = std::fmod(ratioToCents(ratio), 1200.0);

                    if (cents > 0 && cents < 1200) {
                        auto pair = ratio.toPair();
                        if (seenRatios.insert(pair).second) {
                            double etCents = std::round(cents / 100.0) * 100.0;
                            double centsDeviation = cents - etCents;

                            intervals.emplace_back(
                                std::to_string(num) + ":" + std::to_string(den),
                                ratio,
                                centsDeviation
                            );
                        }
                    }
                }
            }
        }

        std::sort(intervals.begin(), intervals.end(),
                 [this](const Note<RationalType>& a, const Note<RationalType>& b) {
                     return ratioToCents(a.frequency) < ratioToCents(b.frequency);
                 });

        return intervals;
    }
};

template<typename RationalType>
void demonstrateCapabilities() {
    HarmonicsCalculator<RationalType> calc;

    // 1. Generate just-intonation major scale
    std::cout << "Just Intonation Major Scale:\n";
    std::vector<std::string> majorScale = {
        "unison", "major_second", "major_third", "perfect_fourth",
        "perfect_fifth", "major_sixth", "major_seventh"
    };

    auto scale = calc.generateScale(RationalType(1, 1), majorScale);
    for (const auto& note : scale) {
        double freq = double(note.frequency) * 440.0;
        std::cout << std::fixed << std::setprecision(2)
                  << note.name << ": " << note.frequency
                  << " (" << freq << " Hz), deviation: "
                  << std::showpos << note.cents << " cents\n";
    }
    std::cout << '\n';

    // 2. Generate harmonic series
    std::cout << "First 8 Harmonics of A4 (440 Hz):\n";
    auto harmonics = calc.findHarmonics(RationalType(1, 1));
    for (const auto& harmonic : harmonics) {
        double freq = double(harmonic.frequency) * 440.0;
        std::cout << std::fixed << std::setprecision(2)
                  << harmonic.name << ": " << harmonic.frequency
                  << " (" << freq << " Hz), deviation: "
                  << std::showpos << harmonic.cents << " cents\n";
    }
    std::cout << '\n';

    // 3. Find simple just intervals
    std::cout << "Simple Just Intervals (up to 5:4):\n";
    auto intervals = calc.findJustIntervals(5, 4);
    for (const auto& interval : intervals) {
        double cents = std::log2(double(interval.frequency)) * 1200.0;
        std::cout << std::fixed << std::setprecision(2)
                  << "Ratio " << interval.name << ": "
                  << cents << " cents, deviation: "
                  << std::showpos << interval.cents << " cents\n";
    }
}

int main()
try {
    demonstrateCapabilities<Rational>();

    demonstrateCapabilities<sw::universal::erational>();
    return 0;
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
}

