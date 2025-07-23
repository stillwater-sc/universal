#pragma once
// layer.hpp: DNN implementation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <numeric/containers.hpp>

namespace sw { namespace dnn {

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



//////////////////////////////////////////////////////////////////////////////
///           FULLY CONNECTED LAYER

template<typename WeightScalarType, typename ActivationScalarType>
class FullyConnectedLayer : public AbstractLayer {
public:
    FullyConnectedLayer() noexcept = default;
    FullyConnectedLayer(unsigned nrNodes, unsigned nrChannels, Activation activation) : nrChannels{ nrChannels }, weight(nrNodes), bias(nrNodes), activation{ activation } {}

protected:

private:
    unsigned nrChannels;
    sw::numeric::containers::vector<WeightScalarType> weight;
    sw::numeric::containers::vector<WeightScalarType> bias;
    Activation activation;

    template<typename WWeightScalarType, typename AActivationScalarType>
    friend std::ostream& operator<<(std::ostream& ostr, FullyConnectedLayer<WWeightScalarType, AActivationScalarType>& fcLayer);
};

template<typename WeightScalarType, typename ActivationScalarType>
FullyConnectedLayer<WeightScalarType, ActivationScalarType> CreateFullyConnectedLayer(unsigned nrNodes, Activation activation) {
    return FullyConnectedLayer<WeightScalarType, ActivationScalarType>(nrNodes, 1, activation);
}

template<typename WeightScalarType, typename ActivationScalarType>
std::ostream& operator<<(std::ostream& ostr, FullyConnectedLayer<WeightScalarType, ActivationScalarType>& fcLayer) {
    ostr << "Fully Connected Layer\n";
    ostr << "weights :\n" << fcLayer.weight << '\n';
    ostr << "biases  :\n" << fcLayer.bias << '\n';
    return ostr;
}

//////////////////////////////////////////////////////////////////////////////
///           CONVOLUTIONAL LAYER

template<typename WeightScalarType, typename ActivationScalarType>
class ConvolutionalLayer : public AbstractLayer {
public:
    ConvolutionalLayer() noexcept = default;
    ConvolutionalLayer(unsigned N, unsigned C, unsigned H, unsigned W, Activation activation) : N{N}, C{C}, H{H}, W{W}, weight(C*H*W), bias(C*H*W), activation{activation} {}

protected:

private:
    unsigned N, C, H, W;
    sw::numeric::containers::vector<WeightScalarType> weight;
    sw::numeric::containers::vector<WeightScalarType> bias;
    Activation activation;

    template<typename W, typename A>
    friend std::ostream& operator<<(std::ostream& ostr, ConvolutionalLayer<W, A>& fcLayer);
};

template<typename WeightScalarType, typename ActivationScalarType>
ConvolutionalLayer<WeightScalarType, ActivationScalarType> CreateConvolutionLayer(unsigned N, unsigned C, unsigned H, unsigned W, Activation activation) {
    return ConvolutionalLayer<WeightScalarType, ActivationScalarType>(N, C, H, W, activation);
}

template<typename WeightScalarType, typename ActivationScalarType>
std::ostream& operator<<(std::ostream& ostr, ConvolutionalLayer<WeightScalarType, ActivationScalarType>& convLayer) {
    ostr << "Convolutional Layer\n";
    ostr << "batch size  : " << convLayer.N << '\n';
    ostr << "channels    : " << convLayer.C << '\n';
    ostr << "height      : " << convLayer.H << '\n';
    ostr << "width       : " << convLayer.W << '\n';
    ostr << "weights     : " << convLayer.weight.size() << '\n';
    ostr << "biases      : " << convLayer.bias.size() << '\n';
    return ostr;
}

}} // namespace sw::dnn
