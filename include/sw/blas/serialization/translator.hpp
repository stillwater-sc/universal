#pragma once
// translator.hpp: translate data files between different client type sets
//
// The translator reads a data file using a source type registry,
// converts values directly from SrcT to DstT using native type
// conversion operators, and writes to a new data file using a
// target type registry. This preserves precision for multi-component
// types like dd and qd that would be truncated through double.
//
// Usage:
//   using SrcTypes = type_list<float, cfloat<19,8,...>>;  // NVIDIA (TF32)
//   using DstTypes = type_list<float, cfloat<16,8,...>>;  // AMD (BF16)
//   translate<SrcTypes, DstTypes>(input, output);
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdint>
#include <functional>

#include <blas/serialization/datafile.hpp>
#include <blas/serialization/type_registry.hpp>

namespace sw { namespace blas {

// type_map_entry: maps a source type ID + parameters to a target type ID + parameters
struct type_map_entry {
    uint32_t src_typeId;
    uint32_t src_nrParams;
    uint32_t src_params[16];
    uint32_t dst_typeId;
    uint32_t dst_nrParams;
    uint32_t dst_params[16];
};

// type_map: a collection of source-to-target type mappings
using type_map = std::vector<type_map_entry>;

// make_mapping: create a type_map_entry from two compile-time types
template<typename SrcType, typename DstType>
type_map_entry make_mapping() {
    type_map_entry entry{};
    generateScalarTypeId<SrcType>(entry.src_typeId, entry.src_nrParams, entry.src_params);
    generateScalarTypeId<DstType>(entry.dst_typeId, entry.dst_nrParams, entry.dst_params);
    return entry;
}

// find_mapping: look up a destination type for a given source type descriptor
inline bool find_mapping(const type_map& map,
                         uint32_t srcTypeId, uint32_t srcNrParams, const uint32_t* srcParams,
                         uint32_t& dstTypeId, uint32_t& dstNrParams, uint32_t* dstParams) {
    for (const auto& entry : map) {
        if (entry.src_typeId != srcTypeId) continue;
        if (entry.src_nrParams != srcNrParams) continue;
        if (srcNrParams > 16) continue;  // bounds guard
        bool match = true;
        for (uint32_t i = 0; i < srcNrParams; ++i) {
            if (entry.src_params[i] != srcParams[i]) { match = false; break; }
        }
        if (match) {
            dstTypeId = entry.dst_typeId;
            dstNrParams = entry.dst_nrParams;
            if (dstNrParams > 16) { dstNrParams = 0; return false; }  // bounds guard
            for (uint32_t i = 0; i < dstNrParams; ++i) dstParams[i] = entry.dst_params[i];
            return true;
        }
    }
    return false;
}

// translate: read a data file with SrcTypes, convert to DstTypes, write output
//
// The translation reads each dataset, converts values through double,
// and writes them in the target type. The type_map specifies which
// source types map to which target types.
//
// Types that appear in the source file but have no mapping are skipped
// with a warning. Types that map to themselves are copied unchanged.
template<typename SrcTypeList, typename DstTypeList>
bool translate(std::istream& input, std::ostream& output, const type_map& map) {
    // read magic number
    uint32_t magic;
    input >> magic;
    if (magic != UNIVERSAL_DATA_FILE_MAGIC_NUMBER) {
        std::cerr << "translator: not a Universal datafile\n";
        return false;
    }
    output << UNIVERSAL_DATA_FILE_MAGIC_NUMBER << '\n';

    uint32_t typeId;
    input >> typeId;
    while (typeId > 0) {
        // read source type descriptor
        uint32_t nrParams;
        input >> nrParams;
        if (nrParams > 16) {
            std::cerr << "translator: malformed file (nrParams=" << nrParams << " > 16)\n";
            return false;
        }
        uint32_t params[16]{ 0 };
        for (uint32_t i = 0; i < nrParams; ++i) input >> params[i];

        // skip comment
        std::string token;
        input >> token;
        std::string comment;
        std::getline(input, comment);

        // read aggregation info
        uint32_t aggType, nrElements;
        input >> aggType >> nrElements;

        // find the mapping
        uint32_t dstTypeId, dstNrParams;
        uint32_t dstParams[16]{ 0 };
        bool mapped = find_mapping(map, typeId, nrParams, params,
                                   dstTypeId, dstNrParams, dstParams);

        if (!mapped) {
            std::cerr << "translator: no mapping for source type id " << typeId
                      << ", skipping " << nrElements << " elements\n";
            // skip payload
            std::string tok;
            std::string newline;
            std::getline(input, newline);
            for (uint32_t i = 0; i < nrElements; ++i) input >> tok;
            std::string name;
            input >> name;
            input >> typeId;
            continue;
        }

        // Read source values into a string buffer for type-safe parsing
        std::string newline;
        std::getline(input, newline);
        std::vector<std::string> tokens(nrElements);
        for (uint32_t i = 0; i < nrElements; ++i) {
            input >> tokens[i];
        }

        // read dataset name
        std::string name;
        input >> name;

        // Buffer the output for this dataset so we only commit on success.
        // If either source or destination dispatch fails, the dataset is
        // skipped entirely without corrupting the output file.
        std::ostringstream dsBuf;

        // write destination type descriptor
        dsBuf << dstTypeId << '\n';
        dsBuf << dstNrParams;
        for (uint32_t i = 0; i < dstNrParams; ++i) dsBuf << ' ' << dstParams[i];
        dsBuf << '\n';

        // write comment
        dsBuf << "# translated " << scalarType(dstTypeId) << '\n';

        // write aggregation info
        dsBuf << aggType << ' ' << nrElements << '\n';

        // Convert: parse each token as SrcT, convert to DstT, write.
        // Uses native type conversion (not truncated through double).
        bool dstOk = false;
        bool srcOk = dispatch(SrcTypeList{}, typeId, nrParams, params,
            [&](auto srcTag) {
                using SrcT = typename decltype(srcTag)::type;
                dstOk = dispatch(DstTypeList{}, dstTypeId, dstNrParams, dstParams,
                    [&](auto dstTag) {
                        using DstT = typename decltype(dstTag)::type;
                        int count = 0;
                        for (uint32_t i = 0; i < nrElements; ++i) {
                            SrcT srcVal{};
                            std::istringstream iss(tokens[i]);
                            iss >> srcVal;
                            DstT dstVal{};
                            if constexpr (std::is_same_v<SrcT, DstT>) {
                                dstVal = srcVal;
                            }
                            else {
                                dstVal = static_cast<DstT>(srcVal);
                            }
                            dsBuf << dstVal;
                            ++count;
                            if ((count % 10) == 0) dsBuf << '\n'; else dsBuf << ' ';
                        }
                        dsBuf << '\n';
                    });
            });

        if (!srcOk) {
            std::cerr << "translator: source type id " << typeId
                      << " not in source registry, skipping dataset\n";
        }
        else if (!dstOk) {
            std::cerr << "translator: destination type id " << dstTypeId
                      << " not in destination registry, skipping dataset\n";
        }
        else {
            // commit the buffered dataset to the output
            dsBuf << name << '\n';
            output << dsBuf.str();
        }

        // read next type id
        input >> typeId;
    }

    // write termination token
    output << 0 << std::endl;
    return true;
}

}} // namespace sw::blas
