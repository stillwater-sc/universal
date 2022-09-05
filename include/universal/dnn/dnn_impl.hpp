#pragma once
// dnn_impl.hpp: DNN implementation
//
// Copyright (C) 2021-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/blas/vector.hpp>

namespace sw { namespace universal { namespace dnn {

template<typename Scalar = float>
class dnn {
public:
    dnn() : learningRate(0.1f) {}

    template<typename LayerType>
    void addLayer(LayerType& layer) noexcept {
        layers.push_back(&layer);
    }

protected:


private:
    Scalar learningRate;
    std::vector<AbstractLayer*> layers;
};

}}} // namespace sw::universal::dnn
