#pragma once
// type_registry.hpp: compile-time type registry for client-directed serialization
//
// The type registry allows clients to declare their supported number types
// at compile time. The dispatch mechanism uses fold expressions to generate
// a compact if-else chain that matches runtime type IDs from the file
// against the client's compile-time type list.
//
// Usage:
//   using MyTypes = sw::blas::type_list<float, double, sw::universal::half>;
//   sw::blas::typed_datafile<MyTypes> df;
//   df.restore(input_stream);
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <type_traits>

namespace sw { namespace blas {

// type_list: a compile-time list of types that a client supports
template<typename... Types>
struct type_list {};

// type_identity: C++20 std::type_identity backport for use as a tag type
template<typename T>
struct type_identity { using type = T; };

// try_match: compare a runtime type descriptor against a compile-time type T
// Returns true if the type matches and invokes the handler
template<typename T, typename Handler>
bool try_match(uint32_t typeId, uint32_t nrParams, const uint32_t* params, Handler&& handler) {
    uint32_t myId{ 0 };
    uint32_t myNrParams{ 0 };
    uint32_t myParams[16]{ 0 };
    if (!generateScalarTypeId<T>(myId, myNrParams, myParams)) return false;
    if (myId != typeId) return false;
    if (myNrParams != nrParams) return false;
    for (uint32_t i = 0; i < nrParams; ++i) {
        if (myParams[i] != params[i]) return false;
    }
    handler(type_identity<T>{});
    return true;
}

// dispatch: try to match a runtime type descriptor against each type in a type_list
// Uses fold expression to generate a compact if-else chain at compile time.
// Returns true if a match was found.
template<typename... Types, typename Handler>
bool dispatch(type_list<Types...>, uint32_t typeId, uint32_t nrParams, const uint32_t* params, Handler&& handler) {
    return (try_match<Types>(typeId, nrParams, params, handler) || ...);
}

}} // namespace sw::blas
