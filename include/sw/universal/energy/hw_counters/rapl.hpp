#pragma once
// rapl.hpp: Intel RAPL energy measurement via Linux powercap sysfs
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// RAPL (Running Average Power Limit) provides hardware energy counters
// on Intel (and some AMD) processors. This implementation uses the
// Linux powercap sysfs interface which requires no external dependencies.
//
// Supported platforms: Linux only
// Requirements:
//   - Linux kernel >= 3.13 with powercap support
//   - Read access to /sys/class/powercap/intel-rapl/
//   - Intel or AMD processor with RAPL support
//
// Usage:
//   #include <universal/energy/hw_counters/rapl.hpp>
//
//   using namespace sw::universal::energy;
//
//   if (RaplReader::isAvailable()) {
//       RaplReader rapl;
//       rapl.start();
//       // ... computation ...
//       auto result = rapl.stop();
//       std::cout << "Energy: " << result.package_uj << " uJ\n";
//   }

#include <string>
#include <cstdint>
#include <iostream>

// Platform detection
#if defined(__linux__) || defined(__linux) || defined(linux)
    #define UNIVERSAL_RAPL_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
    #define UNIVERSAL_RAPL_MACOS 1
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #define UNIVERSAL_RAPL_WINDOWS 1
#else
    #define UNIVERSAL_RAPL_UNKNOWN 1
#endif

#ifdef UNIVERSAL_RAPL_LINUX
// Linux-specific includes
#include <fstream>
#include <sstream>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#endif

namespace sw { namespace universal { namespace energy {

/// Energy measurement result from RAPL
struct RaplEnergy {
    uint64_t package_uj;      // Package (CPU + uncore) energy in microjoules
    uint64_t cores_uj;        // CPU cores energy in microjoules (PP0)
    uint64_t uncore_uj;       // Uncore (GPU, LLC) energy in microjoules (PP1)
    uint64_t dram_uj;         // DRAM energy in microjoules
    double   elapsed_ms;      // Elapsed time in milliseconds
    bool     valid;           // True if measurement succeeded

    RaplEnergy() : package_uj(0), cores_uj(0), uncore_uj(0), dram_uj(0),
                   elapsed_ms(0.0), valid(false) {}

    /// Get package energy in joules
    double packageJoules() const { return package_uj / 1000000.0; }

    /// Get cores energy in joules
    double coresJoules() const { return cores_uj / 1000000.0; }

    /// Get DRAM energy in joules
    double dramJoules() const { return dram_uj / 1000000.0; }

    /// Get total energy in joules
    double totalJoules() const { return (package_uj + dram_uj) / 1000000.0; }

    /// Get average power in watts
    double averagePowerWatts() const {
        if (elapsed_ms <= 0) return 0.0;
        return totalJoules() / (elapsed_ms / 1000.0);
    }

    /// Report energy measurement
    void report(std::ostream& os = std::cout) const {
        if (!valid) {
            os << "RAPL measurement: invalid/unavailable\n";
            return;
        }
        os << "RAPL Energy Measurement:\n";
        os << "  Package:  " << package_uj << " uJ (" << packageJoules() << " J)\n";
        if (cores_uj > 0)
            os << "  Cores:    " << cores_uj << " uJ (" << coresJoules() << " J)\n";
        if (dram_uj > 0)
            os << "  DRAM:     " << dram_uj << " uJ (" << dramJoules() << " J)\n";
        os << "  Elapsed:  " << elapsed_ms << " ms\n";
        os << "  Avg Power: " << averagePowerWatts() << " W\n";
    }
};

#ifdef UNIVERSAL_RAPL_LINUX
// ============================================================================
// Linux implementation using powercap sysfs
// ============================================================================

class RaplReader {
public:
    RaplReader() : start_time_ns_(0), started_(false) {
        detectDomains();
    }

    /// Check if RAPL is available on this system
    static bool isAvailable() {
        struct stat st;
        return (stat("/sys/class/powercap/intel-rapl", &st) == 0 && S_ISDIR(st.st_mode));
    }

    /// Check if this instance has valid RAPL domains
    bool hasPackage() const { return !package_path_.empty(); }
    bool hasCores() const { return !cores_path_.empty(); }
    bool hasUncore() const { return !uncore_path_.empty(); }
    bool hasDram() const { return !dram_path_.empty(); }

    /// Start energy measurement
    void start() {
        if (!package_path_.empty()) start_package_ = readEnergyFile(package_path_);
        if (!cores_path_.empty())   start_cores_ = readEnergyFile(cores_path_);
        if (!uncore_path_.empty())  start_uncore_ = readEnergyFile(uncore_path_);
        if (!dram_path_.empty())    start_dram_ = readEnergyFile(dram_path_);
        start_time_ns_ = getTimeNs();
        started_ = true;
    }

    /// Stop energy measurement and return results
    RaplEnergy stop() {
        RaplEnergy result;
        if (!started_) {
            return result;
        }

        uint64_t end_time_ns = getTimeNs();
        result.elapsed_ms = (end_time_ns - start_time_ns_) / 1000000.0;

        if (!package_path_.empty()) {
            uint64_t end_val = readEnergyFile(package_path_);
            result.package_uj = computeDelta(start_package_, end_val, package_max_);
        }
        if (!cores_path_.empty()) {
            uint64_t end_val = readEnergyFile(cores_path_);
            result.cores_uj = computeDelta(start_cores_, end_val, cores_max_);
        }
        if (!uncore_path_.empty()) {
            uint64_t end_val = readEnergyFile(uncore_path_);
            result.uncore_uj = computeDelta(start_uncore_, end_val, uncore_max_);
        }
        if (!dram_path_.empty()) {
            uint64_t end_val = readEnergyFile(dram_path_);
            result.dram_uj = computeDelta(start_dram_, end_val, dram_max_);
        }

        result.valid = !package_path_.empty();
        started_ = false;
        return result;
    }

    /// Get system information
    std::string systemInfo() const {
        std::stringstream ss;
        ss << "RAPL domains detected:\n";
        if (hasPackage()) ss << "  Package: " << package_path_ << "\n";
        if (hasCores())   ss << "  Cores (PP0): " << cores_path_ << "\n";
        if (hasUncore())  ss << "  Uncore (PP1): " << uncore_path_ << "\n";
        if (hasDram())    ss << "  DRAM: " << dram_path_ << "\n";
        return ss.str();
    }

private:
    std::string package_path_;
    std::string cores_path_;
    std::string uncore_path_;
    std::string dram_path_;

    uint64_t package_max_ = UINT64_MAX;
    uint64_t cores_max_ = UINT64_MAX;
    uint64_t uncore_max_ = UINT64_MAX;
    uint64_t dram_max_ = UINT64_MAX;

    uint64_t start_package_ = 0;
    uint64_t start_cores_ = 0;
    uint64_t start_uncore_ = 0;
    uint64_t start_dram_ = 0;
    uint64_t start_time_ns_ = 0;
    bool started_ = false;

    void detectDomains() {
        const std::string base = "/sys/class/powercap/intel-rapl";

        // Find package domain (intel-rapl:0, intel-rapl:1, etc.)
        DIR* dir = opendir(base.c_str());
        if (!dir) return;

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name.find("intel-rapl:") == 0 && name.find(':') == name.rfind(':')) {
                // This is a package domain (e.g., intel-rapl:0)
                std::string pkg_path = base + "/" + name;
                std::string energy_file = pkg_path + "/energy_uj";

                struct stat st;
                if (stat(energy_file.c_str(), &st) == 0) {
                    if (package_path_.empty()) {
                        package_path_ = energy_file;
                        package_max_ = readMaxEnergy(pkg_path + "/max_energy_range_uj");
                    }

                    // Look for subdomains (cores, uncore, dram)
                    detectSubdomains(pkg_path);
                }
            }
        }
        closedir(dir);
    }

    void detectSubdomains(const std::string& pkg_path) {
        DIR* dir = opendir(pkg_path.c_str());
        if (!dir) return;

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name.find("intel-rapl:") == 0) {
                std::string sub_path = pkg_path + "/" + name;
                std::string name_file = sub_path + "/name";
                std::string energy_file = sub_path + "/energy_uj";

                std::string domain_name = readNameFile(name_file);

                struct stat st;
                if (stat(energy_file.c_str(), &st) == 0) {
                    if (domain_name == "core" && cores_path_.empty()) {
                        cores_path_ = energy_file;
                        cores_max_ = readMaxEnergy(sub_path + "/max_energy_range_uj");
                    } else if (domain_name == "uncore" && uncore_path_.empty()) {
                        uncore_path_ = energy_file;
                        uncore_max_ = readMaxEnergy(sub_path + "/max_energy_range_uj");
                    } else if (domain_name == "dram" && dram_path_.empty()) {
                        dram_path_ = energy_file;
                        dram_max_ = readMaxEnergy(sub_path + "/max_energy_range_uj");
                    }
                }
            }
        }
        closedir(dir);
    }

    static uint64_t readEnergyFile(const std::string& path) {
        std::ifstream f(path);
        uint64_t val = 0;
        if (f.is_open()) {
            f >> val;
        }
        return val;
    }

    static uint64_t readMaxEnergy(const std::string& path) {
        std::ifstream f(path);
        uint64_t val = UINT64_MAX;
        if (f.is_open()) {
            f >> val;
        }
        return val;
    }

    static std::string readNameFile(const std::string& path) {
        std::ifstream f(path);
        std::string name;
        if (f.is_open()) {
            std::getline(f, name);
        }
        return name;
    }

    static uint64_t computeDelta(uint64_t start, uint64_t end, uint64_t max_val) {
        if (end >= start) {
            return end - start;
        }
        // Counter wrapped around
        return (max_val - start) + end;
    }

    static uint64_t getTimeNs() {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return static_cast<uint64_t>(ts.tv_sec) * 1000000000ULL + ts.tv_nsec;
    }
};

#else
// ============================================================================
// Stub implementation for non-Linux platforms (MacOS, Windows, etc.)
// ============================================================================

class RaplReader {
public:
    RaplReader() {}

    /// RAPL is not available on this platform
    static bool isAvailable() { return false; }

    bool hasPackage() const { return false; }
    bool hasCores() const { return false; }
    bool hasUncore() const { return false; }
    bool hasDram() const { return false; }

    void start() {
        // No-op on unsupported platforms
    }

    RaplEnergy stop() {
        RaplEnergy result;
        result.valid = false;
        return result;
    }

    std::string systemInfo() const {
        return "RAPL not available: requires Linux with Intel/AMD processor\n";
    }
};

#endif // UNIVERSAL_RAPL_LINUX

/// RAII wrapper for RAPL measurement
class ScopedRaplMeasurement {
public:
    explicit ScopedRaplMeasurement(const std::string& label = "")
        : label_(label), reader_() {
        if (RaplReader::isAvailable()) {
            reader_.start();
            active_ = true;
        }
    }

    ~ScopedRaplMeasurement() {
        if (active_) {
            auto result = reader_.stop();
            if (result.valid) {
                std::cout << "RAPL [" << label_ << "]: "
                          << result.package_uj << " uJ, "
                          << result.averagePowerWatts() << " W avg\n";
            }
        }
    }

    // Non-copyable
    ScopedRaplMeasurement(const ScopedRaplMeasurement&) = delete;
    ScopedRaplMeasurement& operator=(const ScopedRaplMeasurement&) = delete;

private:
    std::string label_;
    RaplReader reader_;
    bool active_ = false;
};

}}} // namespace sw::universal::energy
