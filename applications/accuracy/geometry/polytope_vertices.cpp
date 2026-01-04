// polytope_vertices.cpp: 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <vector>
#include <set>
#include <algorithm>
#include <limits>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/ereal/ereal.hpp>
// Stillwater BLAS library
#include <blas/blas.hpp>

#ifdef TBD
namespace sw {
    namespace universal {

        template<typename Scalar>
        struct BoundingBox {
            sw::universal::blas::vector<Scalar> min_coords;
            sw::universal::blas::vector<Scalar> max_coords;
        };

        template<typename Scalar>
        class ConvexPolytope {
            using Matrix = sw::universal::blas::matrix<Scalar>;
            using Vector = sw::universal::blas::vector<Scalar>;
            using iVector = sw::universal::blas::vector<int>;
        public:
            ConvexPolytope(const Matrix& A, const Vector& b)
                : A_(A), b_(b), n_dims_(A.cols()), n_constraints_(A.rows()) {}

            std::vector<sw::universal::blas::vector<Scalar>> enumerateVertices() {
                std::vector<Vector> vertices;
                std::vector<size_t> indices(n_dims_);
        
                // Initialize indices for combination generation
                for (size_t i = 0; i < n_dims_; ++i) {
                    indices[i] = i;
                }

                // Generate all possible combinations of n_dims_ hyperplanes
                do {
                    Matrix A_sub(n_dims_, n_dims_);
                    Vector b_sub(n_dims_);
#ifdef SUBMATRIX_FUNCTIONALITY_IMPLEMENTED
                    // Extract submatrix and subvector
                    for (size_t i = 0; i < n_dims_; ++i) {
                        A_sub.row(i) = A_.row(indices[i]);
                        b_sub(i) = b_(indices[i]);
                    }
#endif
                    // Try to solve the system
                    Vector x;
                    bool success = solveSystem(A_sub, b_sub, x);

                    if (success && isVertexValid(x)) {
                        vertices.push_back(x);
                    }
                } while (nextCombination(indices, n_constraints_));

                // Remove duplicate vertices
                removeDuplicates(vertices);
                return vertices;
            }

            BoundingBox<Scalar> getBoundingBox(const std::vector<Vector>& vertices) {
                BoundingBox<Scalar> bbox;
                if (vertices.empty()) {
                    return bbox;
                }

                bbox.min_coords = Vector(n_dims_, std::numeric_limits<Scalar>::infinity());
                bbox.max_coords = Vector(n_dims_, -std::numeric_limits<Scalar>::infinity());

                for (const auto& vertex : vertices) {
                    for (int i = 0; i < n_dims_; ++i) {
                        bbox.min_coords(i) = std::min(bbox.min_coords(i), vertex(i));
                        bbox.max_coords(i) = std::max(bbox.max_coords(i), vertex(i));
                    }
                }

                return bbox;
            }

            std::vector<iVector> enumerateIntegerPoints(const BoundingBox<Scalar>& bbox) {
                std::vector<iVector> points;
                size_t dim = bbox.min_coords.size();

                // Convert to integer coordinates (floor for min, ceil for max)
                // iVector min_int = bbox.min_coords.array().floor().cast<int>();
                // iVector max_int = bbox.max_coords.array().ceil().cast<int>();
                iVector min_int(dim);
                for (auto e : bbox.min_coords) min_int.push_back(floor(e));

                iVector max_int(dim);
                for (auto e : bbox.max_coords) max_int.push_back(ceil(e));

                // Recursive helper function to generate points
                iVector current_point(n_dims_);
                generateIntegerPoints(points, current_point, min_int, max_int, 0);

                return points;
            }

        private:
            Matrix A_;
            Vector b_;
            int n_dims_;
            int n_constraints_;
            const double EPSILON = 1e-10;

            bool solveSystem(const Matrix& A, const Vector& b, Vector& x) {
                // Solve system using LU decomposition
                //Eigen::FullPivLU<Eigen::MatrixXd> lu(A);
                //if (lu.isInvertible()) {
                //    x = lu.solve(b);
                //    return true;
                //}
                //sw::universal::blas::vector<size_t> P(A.cols());
                //auto PLU(A);
                //sw::universal::blas::plu(PLU, P);
                //x = PLU.solve(b);
                return false;
            }

            bool isVertexValid(const Vector& x) {
                // Check if point satisfies all constraints (Ax <= b)
                // return (A_ * x - b_).array().maxCoeff() <= EPSILON;
                auto v = (A_ * x - b_);
                Scalar maxCoeff{ std::numeric_limits<Scalar>::lowest() };
                for (auto coeff : v) if (coeff > maxCoeff) maxCoeff = coeff;
                return maxCoeff <= EPSILON;
            }

            void removeDuplicates(std::vector<Vector>& vertices) {
                std::set<Vector> unique_vertices;
        
                for (const auto& v : vertices) {
                    unique_vertices.insert(v);
                }

                vertices.clear();
                for (const auto& v : unique_vertices) {
                    vertices.push_back(v);
                }
            }

            bool nextCombination(std::vector<size_t>& indices, size_t n) {
                size_t k = indices.size();
                for (size_t i = k - 1; i < k; --i) {
                    if (indices[i] < n - k + i) {
                        ++indices[i];
                        for (size_t j = i + 1; j < k; ++j) {
                            indices[j] = indices[j-1] + 1;
                        }
                        return true;
                    }
                }
                return false;
            }

            void generateIntegerPoints(
                std::vector<iVector>& points,
                iVector& current_point,
                const iVector& min_coords,
                const iVector& max_coords,
                int dim) {
                if (dim == n_dims_) {
                    points.push_back(current_point);
                    return;
                }

                for (int i = min_coords(dim); i <= max_coords(dim); ++i) {
                    current_point(dim) = i;
                    generateIntegerPoints(points, current_point, min_coords, max_coords, dim + 1);
                }
            }
        };

    }
}
#endif

int main()
try {
    using namespace sw::universal;
#ifdef TBD
    using Scalar = double;
    using Matrix = sw::universal::blas::matrix<Scalar>;
    using Vector = sw::universal::blas::vector<Scalar>;

    // Create matrix A and vector b defining the polytope
    // Example: unit square
    Matrix A = 
    {  { 1,  0}
      ,{ 0,  1}
      ,{-1,  0}
      ,{ 0, -1}
    };
    Vector b = { 1, 1, 1, 1 } ;

    // Create polytope object
    ConvexPolytope<Scalar> polytope(A, b);

    // Get vertices
    auto vertices = polytope.enumerateVertices();

    // Get bounding box
    auto bbox = polytope.getBoundingBox(vertices);

    // Get integer points
    auto integer_points = polytope.enumerateIntegerPoints(bbox);
#endif

    return EXIT_SUCCESS;
}
catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
}
