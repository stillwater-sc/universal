#pragma once
// typed_datafile.hpp: client-directed serialization for Universal number types
//
// typed_datafile<TypeList> is parameterized by the client's supported types.
// The save() path is type-agnostic (uses type erasure via ICollection).
// The restore() path uses the TypeList to dispatch to the correct template
// instantiation based on the type descriptor read from the file.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <string>
#include <cstdint>

#include <blas/serialization/datafile.hpp>
#include <blas/serialization/type_registry.hpp>

namespace sw { namespace blas {

template<typename TypeList>
class typed_datafile {
public:
    void clear() {
        dataStructures.clear();
        dsNames.clear();
    }

    // add a named collection to the datafile for saving
    template<typename Aggregate>
    void add(Aggregate& ds, const std::string& name = "unnamed") {
        dataStructures.push_back(std::make_unique<CollectionContainer<Aggregate>>(ds));
        dsNames.push_back(name);
    }

    // save all collections to a stream (decimal format for portability)
    bool save(std::ostream& ostr) const {
        return save(ostr, false);
    }

    // save with format option (hex or decimal)
    // NOTE: hex format requires hex-aware restore (not yet implemented)
    // Use decimal format for round-trip via standard operator>>
    bool save(std::ostream& ostr, bool hex) const {
        ostr << UNIVERSAL_DATA_FILE_MAGIC_NUMBER << '\n';
        for (size_t i = 0; i < dataStructures.size(); ++i) {
            dataStructures[i]->save(ostr, hex);
            ostr << dsNames[i] << '\n';
        }
        ostr << 0 << std::endl;
        return true;
    }

    // restore collections from a stream using the client's type registry
    bool restore(std::istream& istr) {
        uint32_t magic;
        istr >> magic;
        if (magic != UNIVERSAL_DATA_FILE_MAGIC_NUMBER) {
            std::cerr << "typed_datafile::restore: not a Universal datafile\n";
            return false;
        }
        clear();

        uint32_t typeId;
        istr >> typeId;
        while (typeId > 0) {
            // read type parameters
            uint32_t nrParams;
            istr >> nrParams;
            uint32_t params[16]{ 0 };
            for (uint32_t i = 0; i < nrParams; ++i) {
                istr >> params[i];
            }

            // skip comment line
            std::string token;
            istr >> token;  // '#' token
            std::string comment;
            std::getline(istr, comment);

            // read aggregation info
            uint32_t aggType, nrElements;
            istr >> aggType >> nrElements;

            // dispatch to the correct type using the client's registry
            bool matched = dispatch(TypeList{}, typeId, nrParams, params,
                [&](auto tag) {
                    using T = typename decltype(tag)::type;
                    restoreTypedCollection<T>(istr, aggType, nrElements);
                });

            if (!matched) {
                std::cerr << "typed_datafile::restore: unsupported type id "
                          << typeId << " (not in client type registry)\n";
                // skip the data payload
                skipPayload(istr, nrElements);
            }

            // read dataset name
            std::string name;
            istr >> name;
            if (!dsNames.empty()) {
                dsNames.back() = name;
            }

            // read next type id or termination token
            istr >> typeId;
        }
        return true;
    }

    size_t size() const { return dataStructures.size(); }
    const std::string& name(size_t i) const { return dsNames.at(i); }

private:
    std::vector<std::unique_ptr<ICollection>> dataStructures;
    std::vector<std::string> dsNames;

    // OwnedCollectionContainer: owns the data structure (for restore path)
    // Unlike CollectionContainer which holds a reference, this owns the data
    // and frees it on destruction.
    template<typename CollectionType>
    class OwnedCollectionContainer : public ICollection {
    public:
        OwnedCollectionContainer(std::unique_ptr<CollectionType> data)
            : owned(std::move(data))
            , container(*owned) {}

        void save(std::ostream& ostr, bool hex) const override {
            container.save(ostr, hex);
        }
        void restore(std::istream& istr) override {
            container.restore(istr);
        }
    private:
        std::unique_ptr<CollectionType> owned;
        CollectionContainer<CollectionType> container;
    };

    template<typename Scalar>
    void restoreTypedCollection(std::istream& istr, uint32_t aggType, uint32_t nrElements) {
        using namespace sw::numeric::containers;
        std::string newline;
        std::getline(istr, newline);  // consume trailing newline

        switch (aggType) {
        case UNIVERSAL_AGGREGATE_VECTOR: {
            auto v = std::make_unique<vector<Scalar>>(nrElements);
            Scalar item{};
            for (uint32_t i = 0; i < nrElements; ++i) {
                istr >> item;
                (*v)[i] = item;
            }
            dataStructures.push_back(
                std::make_unique<OwnedCollectionContainer<vector<Scalar>>>(std::move(v)));
            dsNames.push_back("placeholder");
            break;
        }
        case UNIVERSAL_AGGREGATE_MATRIX: {
            // TODO(Phase 3): extend file format to include rows/cols dimensions
            // For now, store as a flat vector; matrix shape recovery requires
            // additional metadata in the aggregation header.
            auto m = std::make_unique<vector<Scalar>>(nrElements);
            Scalar item{};
            for (uint32_t i = 0; i < nrElements; ++i) {
                istr >> item;
                (*m)[i] = item;
            }
            dataStructures.push_back(
                std::make_unique<OwnedCollectionContainer<vector<Scalar>>>(std::move(m)));
            dsNames.push_back("placeholder");
            break;
        }
        default:
            std::cerr << "typed_datafile::restore: unsupported aggregate type " << aggType << '\n';
            skipPayload(istr, nrElements);
            break;
        }
    }

    void skipPayload(std::istream& istr, uint32_t nrElements) {
        std::string token;
        for (uint32_t i = 0; i < nrElements; ++i) {
            istr >> token;
        }
    }
};

}} // namespace sw::blas
