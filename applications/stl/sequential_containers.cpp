// sequential_containers.cpp: Using STL containers and algorithms with posits
#include "common.hpp"
// configure the number system
// simply use defaults
#include <universal/posit/posit>

// generic template function for all integer types
template<typename Ty>
bool GenerateData(size_t nrOfSamples, std::vector<Ty>& data) {
	std::random_device seed;
	std::mt19937 engine(seed());
	std::uniform_int_distribution<Ty> dist(0, 100);
	data.clear();
	data.reserve(nrOfSamples);
	for (size_t i = 0; i < nrOfSamples; ++i) {
		data.push_back(dist(engine));
	}
	return true;
}

// specializations for the floating point types and posit types
template<>
bool GenerateData(size_t nrOfSamples, std::vector<float>& data) {
	std::random_device seed;
	std::mt19937 engine(seed());
	std::uniform_real_distribution<float> dist(0, 100);
	data.clear();
	data.reserve(nrOfSamples);
	for (size_t i = 0; i < nrOfSamples; ++i) {
		data.push_back(dist(engine));
	}
	return true;
}

template<>
bool GenerateData(size_t nrOfSamples, std::vector<double>& data) {
	std::random_device seed;
	std::mt19937 engine(seed());
	std::uniform_real_distribution<double> dist(0, 100);
	data.clear();
	data.reserve(nrOfSamples);
	for (size_t i = 0; i < nrOfSamples; ++i) {
		data.push_back(dist(engine));
	}
	return true;
}
template<>
bool GenerateData(size_t nrOfSamples, std::vector<long double>& data) {
	std::random_device seed;
	std::mt19937 engine(seed());
	std::uniform_real_distribution<long double> dist(0, 100);
	data.clear();
	data.reserve(nrOfSamples);
	for (size_t i = 0; i < nrOfSamples; ++i) {
		data.push_back(dist(engine));
	}
	return true;
}

template<size_t nbits, size_t es>
bool GenerateData(size_t nrOfSamples, std::vector< sw::universal::posit<nbits, es> >& data) {
	std::random_device seed;
	std::mt19937 engine(seed());
	std::uniform_real_distribution< double > dist(0, 100);  // use automatic conversion to take it from double to posit
	data.clear();
	data.reserve(nrOfSamples);
	for (size_t i = 0; i < nrOfSamples; ++i) {
		//data.push_back(sw::posit<nbits, es>(dist(engine)));
		data.push_back(dist(engine));
	}
	return true;
}

template <typename Container>
void TimedAccumulate(Container& X, const std::string& legend) {
	using value_type = typename Container::value_type;
	// time the operation
	auto begin = std::chrono::steady_clock::now();
		value_type totalSum = std::accumulate(X.begin(), X.end(), value_type(0));
	std::chrono::duration<double> last = std::chrono::steady_clock::now() - begin;

	// report
	std::cout << legend << std::endl;
	std::cout << "time : " << last.count() << std::endl;
	std::cout << "total: " << totalSum << std::endl;
	std::cout << std::endl;
}

template<typename Ty>
void TimedExperiment(std::vector<Ty>& data) {
	using namespace std;

	vector<Ty> myVector(data.begin(), data.end());
	TimedAccumulate(myVector, "std::vector");

	deque<Ty> myDeque(data.begin(), data.end());
	TimedAccumulate(myDeque, "std::deque");

	list<Ty> myList(data.begin(), data.end());
	TimedAccumulate(myList, "std::list");

	forward_list<Ty> mySingleLinkedList(data.begin(), data.end());
	TimedAccumulate(mySingleLinkedList, "std::forward_list");
}

int main()
try {
	using namespace std;

#ifdef NDEBUG
	constexpr size_t NR_SAMPLES = 10'000'000;
#else
	constexpr size_t NR_SAMPLES = 1000;
#endif
	{
		using value_type = int;

		vector<value_type> data;
		GenerateData(NR_SAMPLES, data);
		TimedExperiment(data);
	}

	{
		using namespace sw::universal;
		constexpr size_t nbits = 16;
		constexpr size_t es = 1;
		using value_type = posit<nbits,es>;

		vector<value_type> data;
		GenerateData(NR_SAMPLES, data); 
		TimedExperiment(data);
	}

    
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}