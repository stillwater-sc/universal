#pragma once
// memory_profiler.hpp: estimate memory access patterns and energy for algorithms
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// The memory_profiler estimates memory access patterns and associated energy
// costs for mixed-precision algorithm design. It models the memory hierarchy
// (registers, L1, L2, L3, DRAM) based on working set size and access patterns.
//
// Usage:
//   #include <universal/utility/memory_profiler.hpp>
//
//   using namespace sw::universal;
//
//   MemoryProfiler profiler;
//   profiler.recordRead(array_ptr, n_elements * sizeof(float));
//   profiler.recordWrite(result_ptr, sizeof(float));
//   profiler.report(std::cout);
//
//   // Get energy estimate using a cost model
//   double energy = profiler.estimateEnergy(getDefaultModel());

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstdint>
#include <cmath>
#include <map>
#include <vector>
#include <algorithm>

namespace sw { namespace universal {

/// Memory access pattern classification
enum class AccessPattern {
    Sequential,     // Linear traversal (unit stride)
    Strided,        // Regular stride (e.g., column access in row-major)
    Random,         // Random/irregular access
    Reuse           // Repeated access to same data
};

/// Memory hierarchy level for profiling
enum class MemoryTier {
    Register,
    L1_Cache,
    L2_Cache,
    L3_Cache,
    DRAM
};

/// Convert MemoryTier to string
inline const char* memoryTierName(MemoryTier tier) {
    switch (tier) {
        case MemoryTier::Register: return "Register";
        case MemoryTier::L1_Cache: return "L1 Cache";
        case MemoryTier::L2_Cache: return "L2 Cache";
        case MemoryTier::L3_Cache: return "L3 Cache";
        case MemoryTier::DRAM:     return "DRAM";
        default: return "Unknown";
    }
}

/// Cache configuration for modeling
struct CacheConfig {
    uint64_t l1_size;       // L1 cache size in bytes
    uint64_t l2_size;       // L2 cache size in bytes
    uint64_t l3_size;       // L3 cache size in bytes
    uint64_t cache_line;    // Cache line size in bytes

    // Default: typical modern x86 configuration
    CacheConfig()
        : l1_size(32 * 1024)        // 32 KB L1D
        , l2_size(256 * 1024)       // 256 KB L2
        , l3_size(8 * 1024 * 1024)  // 8 MB L3
        , cache_line(64)            // 64-byte cache lines
    {}

    // Named configurations
    static CacheConfig intel_skylake() {
        CacheConfig cfg;
        cfg.l1_size = 32 * 1024;
        cfg.l2_size = 256 * 1024;
        cfg.l3_size = 8 * 1024 * 1024;
        cfg.cache_line = 64;
        return cfg;
    }

    static CacheConfig arm_cortex_a76() {
        CacheConfig cfg;
        cfg.l1_size = 64 * 1024;
        cfg.l2_size = 512 * 1024;
        cfg.l3_size = 4 * 1024 * 1024;
        cfg.cache_line = 64;
        return cfg;
    }

    static CacheConfig arm_cortex_a55() {
        CacheConfig cfg;
        cfg.l1_size = 32 * 1024;
        cfg.l2_size = 128 * 1024;
        cfg.l3_size = 2 * 1024 * 1024;
        cfg.cache_line = 64;
        return cfg;
    }
};

/// Statistics for a memory region
struct RegionStats {
    uint64_t reads;         // Number of read operations
    uint64_t writes;        // Number of write operations
    uint64_t bytes_read;    // Total bytes read
    uint64_t bytes_written; // Total bytes written
    AccessPattern pattern;  // Detected access pattern

    RegionStats()
        : reads(0), writes(0), bytes_read(0), bytes_written(0)
        , pattern(AccessPattern::Sequential) {}

    uint64_t totalBytes() const { return bytes_read + bytes_written; }
    uint64_t totalAccesses() const { return reads + writes; }
};

/// Memory access profiler for algorithm analysis
class MemoryProfiler {
public:
    MemoryProfiler(const CacheConfig& config = CacheConfig())
        : config_(config) {
        reset();
    }

    /// Reset all statistics
    void reset() {
        total_reads_ = 0;
        total_writes_ = 0;
        total_bytes_read_ = 0;
        total_bytes_written_ = 0;
        working_set_size_ = 0;
        regions_.clear();
    }

    /// Record a read operation
    void recordRead(uint64_t bytes, AccessPattern pattern = AccessPattern::Sequential) {
        total_reads_++;
        total_bytes_read_ += bytes;
        updateWorkingSet(bytes, pattern);
    }

    /// Record a write operation
    void recordWrite(uint64_t bytes, AccessPattern pattern = AccessPattern::Sequential) {
        total_writes_++;
        total_bytes_written_ += bytes;
        updateWorkingSet(bytes, pattern);
    }

    /// Record access to a named region (for detailed tracking)
    void recordRegionRead(const std::string& name, uint64_t bytes,
                          AccessPattern pattern = AccessPattern::Sequential) {
        regions_[name].reads++;
        regions_[name].bytes_read += bytes;
        regions_[name].pattern = pattern;
        recordRead(bytes, pattern);
    }

    void recordRegionWrite(const std::string& name, uint64_t bytes,
                           AccessPattern pattern = AccessPattern::Sequential) {
        regions_[name].writes++;
        regions_[name].bytes_written += bytes;
        regions_[name].pattern = pattern;
        recordWrite(bytes, pattern);
    }

    /// Set working set size directly (for pre-computed analysis)
    void setWorkingSetSize(uint64_t bytes) {
        working_set_size_ = bytes;
    }

    /// Get statistics
    uint64_t totalReads() const { return total_reads_; }
    uint64_t totalWrites() const { return total_writes_; }
    uint64_t totalBytesRead() const { return total_bytes_read_; }
    uint64_t totalBytesWritten() const { return total_bytes_written_; }
    uint64_t totalBytes() const { return total_bytes_read_ + total_bytes_written_; }
    uint64_t workingSetSize() const { return working_set_size_; }

    /// Estimate which cache level will serve most accesses
    MemoryTier estimatePrimaryTier() const {
        if (working_set_size_ <= config_.l1_size) return MemoryTier::L1_Cache;
        if (working_set_size_ <= config_.l2_size) return MemoryTier::L2_Cache;
        if (working_set_size_ <= config_.l3_size) return MemoryTier::L3_Cache;
        return MemoryTier::DRAM;
    }

    /// Estimate cache miss rates (simplified model)
    struct MissRates {
        double l1_miss_rate;
        double l2_miss_rate;
        double l3_miss_rate;
    };

    MissRates estimateMissRates() const {
        MissRates rates = {0.0, 0.0, 0.0};

        // Simple capacity-based model
        // Real miss rates depend on access patterns, associativity, etc.
        if (working_set_size_ > config_.l1_size) {
            rates.l1_miss_rate = std::min(1.0,
                (double)(working_set_size_ - config_.l1_size) / working_set_size_);
        }
        if (working_set_size_ > config_.l2_size) {
            rates.l2_miss_rate = std::min(1.0,
                (double)(working_set_size_ - config_.l2_size) / working_set_size_);
        }
        if (working_set_size_ > config_.l3_size) {
            rates.l3_miss_rate = std::min(1.0,
                (double)(working_set_size_ - config_.l3_size) / working_set_size_);
        }

        return rates;
    }

    /// Estimate memory access distribution across cache levels
    struct AccessDistribution {
        double l1_fraction;
        double l2_fraction;
        double l3_fraction;
        double dram_fraction;
    };

    AccessDistribution estimateDistribution() const {
        AccessDistribution dist = {0.0, 0.0, 0.0, 0.0};
        auto miss = estimateMissRates();

        // Fraction of accesses served by each level
        dist.l1_fraction = 1.0 - miss.l1_miss_rate;
        dist.l2_fraction = miss.l1_miss_rate * (1.0 - miss.l2_miss_rate);
        dist.l3_fraction = miss.l1_miss_rate * miss.l2_miss_rate * (1.0 - miss.l3_miss_rate);
        dist.dram_fraction = miss.l1_miss_rate * miss.l2_miss_rate * miss.l3_miss_rate;

        return dist;
    }

    /// Estimate total memory energy in picojoules
    /// Uses simple cost model (can be replaced with EnergyCostModel integration)
    double estimateEnergyPJ() const {
        // Default energy costs per access (picojoules) - Skylake-class
        constexpr double l1_energy = 3.3;
        constexpr double l2_energy = 17.0;
        constexpr double l3_energy = 66.0;
        constexpr double dram_energy = 650.0;

        auto dist = estimateDistribution();
        uint64_t total = totalBytes();

        // Convert bytes to cache line accesses
        uint64_t cache_accesses = (total + config_.cache_line - 1) / config_.cache_line;

        double energy = 0.0;
        energy += cache_accesses * dist.l1_fraction * l1_energy;
        energy += cache_accesses * dist.l2_fraction * l2_energy;
        energy += cache_accesses * dist.l3_fraction * l3_energy;
        energy += cache_accesses * dist.dram_fraction * dram_energy;

        return energy;
    }

    /// Get energy estimate in microjoules
    double estimateEnergyUJ() const { return estimateEnergyPJ() / 1e6; }

    /// Report memory profile to stream
    void report(std::ostream& ostr) const {
        ostr << "Memory Profile Report\n";
        ostr << std::string(50, '=') << "\n\n";

        ostr << "Access Summary:\n";
        ostr << "  Total reads:    " << total_reads_ << "\n";
        ostr << "  Total writes:   " << total_writes_ << "\n";
        ostr << "  Bytes read:     " << formatBytes(total_bytes_read_) << "\n";
        ostr << "  Bytes written:  " << formatBytes(total_bytes_written_) << "\n";
        ostr << "  Working set:    " << formatBytes(working_set_size_) << "\n\n";

        ostr << "Cache Configuration:\n";
        ostr << "  L1: " << formatBytes(config_.l1_size);
        ostr << ", L2: " << formatBytes(config_.l2_size);
        ostr << ", L3: " << formatBytes(config_.l3_size) << "\n";
        ostr << "  Cache line: " << config_.cache_line << " bytes\n\n";

        ostr << "Estimated Cache Behavior:\n";
        ostr << "  Primary tier: " << memoryTierName(estimatePrimaryTier()) << "\n";

        auto miss = estimateMissRates();
        ostr << std::fixed << std::setprecision(1);
        ostr << "  L1 miss rate: " << (miss.l1_miss_rate * 100) << "%\n";
        ostr << "  L2 miss rate: " << (miss.l2_miss_rate * 100) << "%\n";
        ostr << "  L3 miss rate: " << (miss.l3_miss_rate * 100) << "%\n\n";

        auto dist = estimateDistribution();
        ostr << "Access Distribution:\n";
        ostr << "  L1 Cache: " << (dist.l1_fraction * 100) << "%\n";
        ostr << "  L2 Cache: " << (dist.l2_fraction * 100) << "%\n";
        ostr << "  L3 Cache: " << (dist.l3_fraction * 100) << "%\n";
        ostr << "  DRAM:     " << (dist.dram_fraction * 100) << "%\n\n";

        ostr << std::setprecision(2);
        ostr << "Estimated Energy:\n";
        ostr << "  " << estimateEnergyPJ() << " pJ\n";
        ostr << "  " << estimateEnergyUJ() << " uJ\n";

        if (!regions_.empty()) {
            ostr << "\nRegion Details:\n";
            ostr << std::string(40, '-') << "\n";
            for (const auto& kv : regions_) {
                ostr << "  " << kv.first << ": ";
                ostr << kv.second.reads << " reads, ";
                ostr << kv.second.writes << " writes, ";
                ostr << formatBytes(kv.second.totalBytes()) << "\n";
            }
        }
    }

    /// Get one-line summary
    std::string summary() const {
        std::stringstream ss;
        ss << "WS=" << formatBytes(working_set_size_);
        ss << ", " << memoryTierName(estimatePrimaryTier());
        ss << ", " << std::fixed << std::setprecision(2) << estimateEnergyUJ() << " uJ";
        return ss.str();
    }

private:
    CacheConfig config_;
    uint64_t total_reads_;
    uint64_t total_writes_;
    uint64_t total_bytes_read_;
    uint64_t total_bytes_written_;
    uint64_t working_set_size_;
    std::map<std::string, RegionStats> regions_;

    void updateWorkingSet(uint64_t bytes, AccessPattern pattern) {
        // Simplified working set tracking
        // For Reuse pattern, don't increase working set
        if (pattern != AccessPattern::Reuse) {
            // Heuristic: sequential access has good cache utilization
            // Random access may touch more unique cache lines
            if (pattern == AccessPattern::Random) {
                working_set_size_ += bytes;
            } else if (pattern == AccessPattern::Strided) {
                working_set_size_ += bytes;  // Strided still touches unique data
            } else {
                // Sequential: working set grows more slowly due to reuse
                working_set_size_ = std::max(working_set_size_, bytes);
            }
        }
    }

    static std::string formatBytes(uint64_t bytes) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1);
        if (bytes >= 1024ULL * 1024 * 1024) {
            ss << (bytes / (1024.0 * 1024 * 1024)) << " GB";
        } else if (bytes >= 1024 * 1024) {
            ss << (bytes / (1024.0 * 1024)) << " MB";
        } else if (bytes >= 1024) {
            ss << (bytes / 1024.0) << " KB";
        } else {
            ss << bytes << " B";
        }
        return ss.str();
    }
};

/// Convenience function: profile a BLAS-like operation
inline MemoryProfiler profileGEMM(uint64_t M, uint64_t N, uint64_t K,
                                   size_t element_size,
                                   const CacheConfig& config = CacheConfig()) {
    MemoryProfiler profiler(config);

    // GEMM: C[M,N] = A[M,K] * B[K,N]
    uint64_t A_bytes = M * K * element_size;
    uint64_t B_bytes = K * N * element_size;
    uint64_t C_bytes = M * N * element_size;

    // Read A and B, write C
    profiler.recordRegionRead("A", A_bytes, AccessPattern::Sequential);
    profiler.recordRegionRead("B", B_bytes, AccessPattern::Strided);  // Column access
    profiler.recordRegionWrite("C", C_bytes, AccessPattern::Sequential);

    // Working set is all three matrices
    profiler.setWorkingSetSize(A_bytes + B_bytes + C_bytes);

    return profiler;
}

/// Profile a dot product operation
inline MemoryProfiler profileDotProduct(uint64_t N, size_t element_size,
                                         const CacheConfig& config = CacheConfig()) {
    MemoryProfiler profiler(config);

    uint64_t vector_bytes = N * element_size;

    // Read two vectors, write one scalar
    profiler.recordRegionRead("x", vector_bytes, AccessPattern::Sequential);
    profiler.recordRegionRead("y", vector_bytes, AccessPattern::Sequential);
    profiler.recordRegionWrite("result", element_size, AccessPattern::Sequential);

    // Working set is both vectors (result is negligible)
    profiler.setWorkingSetSize(2 * vector_bytes);

    return profiler;
}

/// Profile a matrix-vector multiply
inline MemoryProfiler profileGEMV(uint64_t M, uint64_t N, size_t element_size,
                                   const CacheConfig& config = CacheConfig()) {
    MemoryProfiler profiler(config);

    uint64_t A_bytes = M * N * element_size;
    uint64_t x_bytes = N * element_size;
    uint64_t y_bytes = M * element_size;

    profiler.recordRegionRead("A", A_bytes, AccessPattern::Sequential);
    profiler.recordRegionRead("x", x_bytes, AccessPattern::Reuse);  // Reused M times
    profiler.recordRegionWrite("y", y_bytes, AccessPattern::Sequential);

    profiler.setWorkingSetSize(A_bytes + x_bytes + y_bytes);

    return profiler;
}

}} // namespace sw::universal
