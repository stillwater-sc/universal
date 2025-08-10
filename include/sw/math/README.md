# Universal Math Library Shim

The Universal library offers plug-in replacements to the native types of the C++ language.
These types live in the sw::universal namespace. To make it simpler to mix custom types
with the standard library type, this math library shim injects the std math library 
functions for the native IEEE-754 types into the sw::universal namespace. This enables
straightforward code that can be parameterized by number type and receive both
native IEEE-754 types and custom Universal number types.


For example, a fully generalized L1 norm function can now be written as:

#include <universal/math/math>   // injection of native IEEE-754 math library functions into sw::universal namespace

namespace sw { namespace universal { namespace blas {

// L1-norm of a vector 
template<typename Scalar> 
Scalar normL1(const sw::universal::blas::vector<Scalar>& v) { 
    using namespace sw::universal; // to specialize abs() for sw::universal types
    Scalar L1Norm{ 0 };
    for (auto e : v) {
        L1Norm += abs(e);
    }
    return L1Norm;
}

}}} // namespace sw::universal::blas
