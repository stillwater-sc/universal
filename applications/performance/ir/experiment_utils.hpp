#pragma once

namespace sw {
    namespace universal {

void ReportKappaValuesForTestMatrices() {
    using namespace sw::universal::blas;
    for (auto& matrixName : TestMatrixList) {
        std::cout << matrixName << '\n';
        matrix<double> ref = getTestMatrix(matrixName);
        std::cout << "Size: (" << ref.rows() << ", " << ref.cols() << ")\n";
        std::cout << "Condition Number = " << kappa(matrixName) << '\n';
        //    std::cout << "Condition estimate: " << condest(ref) << '\n';
    }
}


/// <summary>
/// print the results of a LUIR experiments
/// </summary>
/// <param name="ostr"></param>
/// <param name="testMatrices"></param>
/// <param name="typeLabels"></param>
/// <param name="results"></param>
void PrintIterativeRefinementExperimentResults(std::ostream& ostr, const std::vector<std::string>& testMatrices, const sw::universal::blas::vector<std::string>& typeLabels, std::map<std::string, sw::universal::blas::vector<int> >& results) {
    // create CSV output
   // create the header
    ostr << "Matrix";
    for (auto& e : typeLabels) {
        ostr << ',' << e;
    }
    ostr << '\n';
    for (auto& m : testMatrices) {
        ostr << m;
        if (auto it = results.find(m); it != results.end()) {
            auto& r = results[m];
            for (auto& e : r) {
                ostr << ',' << e;
            }
        }
        ostr << '\n';
    }
}

}
}