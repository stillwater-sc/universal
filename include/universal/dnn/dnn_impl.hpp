#pragma once
// dnn_impl.hpp: DNN implementation
//
// Copyright (C) 2021-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/blas/vector.hpp>

namespace sw { namespace universal { namespace dnn {

template<typename LearningRateType = float>
class dnn {
public:
    dnn() : name("unknown"), learningRate(0.1f) {}
    dnn(const std::string name, LearningRateType lr) : name(name), learningRate(lr) {}

    template<typename LayerType>
    void addLayer(LayerType& layer) noexcept {
        layers.push_back(&layer);
    }

protected:


private:
    std::string name;
    LearningRateType learningRate;
    std::vector<AbstractLayer*> layers;

    template<typename LR>
    friend std::ostream& operator<<(std::ostream& ostr, const dnn<LR>& network);
    template<typename LR>
    friend std::istream& operator>>(std::istream& istr, dnn<LR>& network);
};

template<typename LearningRateType>
std::ostream& operator<<(std::ostream& ostr, const dnn< LearningRateType>& network) {
    ostr << "Deep Neural Network : " << network.name << '\n';
    ostr << "learning Rate       : " << network.learningRate << '\n';
    return ostr;
}

}}} // namespace sw::universal::dnn
