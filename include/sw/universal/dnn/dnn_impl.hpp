#pragma once
// dnn_impl.hpp: DNN implementation
//
// Copyright (c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <numeric/containers/vector.hpp>

namespace sw {
    namespace dnn {

        template<typename LearningRateType = float>
        class dnn {
        public:
            dnn() : name("unknown"), learningRate(0.1f) {}
            dnn(const std::string& name, LearningRateType lr) : name(name), learningRate(lr) {}

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
            ostr << "Learning Rate       : " << network.learningRate << '\n';
            return ostr;
        }

    }
} // namespace sw::dnn
