.. universal documentation master file, created by
   sphinx-quickstart on Sun Mar 21 08:26:01 2021.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to Universal!
=========================================

Universal is a header-only C++17/20 template library for mixed-precision algorithm 
design and optimization. It contains plug-in replacements for native arithmetic types, 
parameterized in terms of precision, dynamic range, sampling profile, and rounding 
algorithms. The number systems provided in Universal enable algorithm development 
that optimizes performance and energy efficiency by tailoring the number systems 
to the requirements of the algorithm.

The motivation to find improvements to IEEE floating-point had been brewing in the HPC community 
since the late 90's. Most algorithms had become memory bound and computational scientists were 
looking for alternatives that provided more granularity in precision and dynamic range
to extract more performance from the memory and networking subsystems. 
Even though the inefficiency of IEEE floating-point had been measured and agreed upon 
in the HPC community, it was the commercial demands of Deep Learning that provided the 
incentive to replace IEEE-754 with alternatives, such as half-floats, bfloats, TensorFloats,
and DeepFloats. 
These alternatives are tailored to the application and yield speed-ups of two to three 
orders of magnitude, making it possible to scale AI deep learning algorithms to ever
more capable and accurate solutions. Other computational science and engineering
algorithms, such as Krylov solvers, Iso Geometric Analysis, N-Body problems, multi-grid methods,
fast multipole methods, etc all stand the benefit from mixed-precision optimization,
and Universal is offering the foundation for this new class of computational science
and engineering performance.

The basic use pattern is as simple as:

| #include <universal/number/posit/posit>
|
| template<typename Real>
| Real MyKernel(const Real& a, const Real& b) {
|   return a * b;  // replace this with your kernel computation
| }
| 
| constexpr double pi = 3.14159265358979323846;
|
| int main() {
|   using Real = sw::universal::posit<32,2>;  
|
|   Real a = sqrt(2);
|   Real b = pi;
|   std::cout << "Result: " << MyKernel(a, b) << std::endl;
| }


The library contains integers, decimals, fixed-points, rationals, linear floats, tapered floats, logarithmic, interval and adaptive-precision integers and floats. There are example number system skeletons to get you started quickly if you desire to add your own, which is highly encouraged.

## Communication channels

* [GitHub Issue](https://github.com/stillwater-sc/universal/issues): bug reports,
  feature requests, etc.
* [Forum](https://groups.google.com/u/1/g/unum-computing): discussion of alternatives to IEEE-754 for computational science.
* [Slack](https://fptalks.slack.com): online chats, discussions,
  and collaboration with other users, researchers and developers.

## Citation

Please cite [our work](https://arxiv.org/abs/2012.11011) if you use _Universal_.

| @article{Omtzigt2020,
|    author    = {E. Theodore L. Omtzigt and Peter Gottschling and Mark Seligman and William Zorn},
|    title     = {{Universal Numbers Library}: design and implementation of a high-performance reproducible number systems library},
|    journal   = {arXiv:2012.11011},
|    year      = {2020},
| }

User Guide
----------

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   usage/installation
   usage/quickstart


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
