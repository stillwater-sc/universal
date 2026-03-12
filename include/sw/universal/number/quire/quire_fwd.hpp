#pragma once
// quire_fwd.hpp: type forwards of the generalize quire
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Forward declarations
template<typename NumberType, unsigned capacity, typename LimbType> class quire;
template<typename NumberType, unsigned capacity, typename LimbType> quire<NumberType, capacity, LimbType> abs(const quire<NumberType, capacity, LimbType>& q);

}} // namespace sw::universal
