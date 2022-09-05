#pragma once
// layer.hpp: DNN implementation
//
// Copyright (C) 2021-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/blas/vector.hpp>

namespace sw { namespace universal { namespace dnn {



enum class Activation {
    ReLU, Sigmoid, Tanh
};

enum class LayerOperation {
    FullyConnected, Sparse, MaxPooling, AvgPooling, Convolutional
};

class AbstractLayer {
public:
    AbstractLayer() {};
    virtual ~AbstractLayer() = 0;
};

AbstractLayer::~AbstractLayer() {}

template<typename WeightScalarType, typename ActivationScalarType>
class Layer : public AbstractLayer {
public:
    Layer() noexcept = default;
    Layer(LayerOperation operation, size_t nrNodes, size_t nrChannels, Activation activation) : op{ operation }, nrChannels{ nrChannels }, weight(nrNodes), bias(nrNodes), activation{ activation } {}


protected:

private:
    LayerOperation op;
    size_t nrChannels;
    sw::universal::blas::vector<WeightScalarType> weight;
    sw::universal::blas::vector<WeightScalarType> bias;
    Activation activation;

    template<typename WWeightScalarType, typename AActivationScalarType>
    friend std::ostream& operator<<(std::ostream& ostr, Layer<WWeightScalarType, AActivationScalarType>& dnnLayer);
};

template<typename WeightScalarType, typename ActivationScalarType>
Layer<WeightScalarType, ActivationScalarType> CreateDenseLayer(size_t nrNodes, Activation activation) {
    return Layer<WeightScalarType, ActivationScalarType>(LayerOperation::FullyConnected, nrNodes, 1, activation);
}

template<typename WeightScalarType, typename ActivationScalarType>
std::ostream& operator<<(std::ostream& ostr, Layer<WeightScalarType, ActivationScalarType>& dnnLayer) {
    ostr << "weights: " << dnnLayer.weight << '\n';
    ostr << "biases : " << dnnLayer.bias << '\n';
    return ostr;
}

}}} // namespace sw::universal::dnn
