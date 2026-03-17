#pragma once
// translator.hpp: translate data files between different client type sets
//
// The translator reads a data file using a source type registry,
// converts values through a common intermediate (double), and writes
// to a new data file using a target type registry.
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
        bool match = true;
        for (uint32_t i = 0; i < srcNrParams; ++i) {
            if (entry.src_params[i] != srcParams[i]) { match = false; break; }
        }
        if (match) {
            dstTypeId = entry.dst_typeId;
            dstNrParams = entry.dst_nrParams;
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

        // read source values through double intermediate
        std::string newline;
        std::getline(input, newline);
        std::vector<double> values(nrElements);
        // dispatch source read through SrcTypeList
        bool srcRead = dispatch(SrcTypeList{}, typeId, nrParams, params,
            [&](auto tag) {
                using SrcT = typename decltype(tag)::type;
                SrcT item{};
                for (uint32_t i = 0; i < nrElements; ++i) {
                    input >> item;
                    values[i] = static_cast<double>(item);
                }
            });

        if (!srcRead) {
            std::cerr << "translator: source type id " << typeId
                      << " not in source registry\n";
            // try to skip
            std::string tok;
            for (uint32_t i = 0; i < nrElements; ++i) input >> tok;
        }

        // read dataset name
        std::string name;
        input >> name;

        // write destination type descriptor
        output << dstTypeId << '\n';
        output << dstNrParams;
        for (uint32_t i = 0; i < dstNrParams; ++i) output << ' ' << dstParams[i];
        output << '\n';

        // write comment
        output << "# translated " << scalarType(dstTypeId) << '\n';

        // write aggregation info
        output << aggType << ' ' << nrElements << '\n';

        // dispatch destination write through DstTypeList
        bool dstWritten = dispatch(DstTypeList{}, dstTypeId, dstNrParams, dstParams,
            [&](auto tag) {
                using DstT = typename decltype(tag)::type;
                int count = 0;
                for (uint32_t i = 0; i < nrElements; ++i) {
                    DstT item = static_cast<DstT>(values[i]);
                    output << item;
                    ++count;
                    if ((count % 10) == 0) output << '\n'; else output << ' ';
                }
                output << '\n';
            });

        if (!dstWritten) {
            std::cerr << "translator: destination type id " << dstTypeId
                      << " not in destination registry\n";
        }

        // write dataset name
        output << name << '\n';

        // read next type id
        input >> typeId;
    }

    // write termination token
    output << 0 << std::endl;
    return true;
}

}} // namespace sw::blas
