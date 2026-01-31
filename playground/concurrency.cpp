// https://en.cppreference.com/w/cpp/algorithm/execution_policy_tag

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

#define PARALLEL
#if defined(PARALLEL) && defined(__cpp_lib_execution)
#include <execution>
    namespace execution = std::execution;
    #define HAS_STD_EXECUTION 1
#else
    enum class execution { seq, unseq, par_unseq, par };
    #define HAS_STD_EXECUTION 0
#endif
 
void measure([[maybe_unused]] auto policy, std::vector<std::uint64_t> v)
{
    const auto start = std::chrono::steady_clock::now();
#if HAS_STD_EXECUTION
    std::sort(policy, v.begin(), v.end());
#else
    std::sort(v.begin(), v.end());
#endif
    const auto finish = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start)
              << '\n';
};
 
int main()
{
    std::vector<std::uint64_t> v(1'000'000);
    std::mt19937 gen {std::random_device{}()};
    std::ranges::generate(v, gen);
 /*
  1M random uint64_t's 

    83ms
    74ms
    12ms
    12ms

  on an 8 core machine
 */
    measure(execution::seq, v);
    measure(execution::unseq, v);
    measure(execution::par_unseq, v);
    measure(execution::par, v);
}
