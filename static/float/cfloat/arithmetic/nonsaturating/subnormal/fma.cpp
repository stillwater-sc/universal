#include <universal/number/cfloat/cfloat.hpp>
#include <random>
#include <cmath>
#include <iostream>
using namespace sw::universal;
template<typename Fp> void check(const char* nm){
  std::mt19937_64 rng(99); std::uniform_real_distribution<double> ud(-8,8);
  int p = std::numeric_limits<Fp>::digits;
  int mismatch=0, total=0; double maxUlp=0;
  for(int i=0;i<100000;++i){
    Fp a(ud(rng)), b(ud(rng)), c(ud(rng));
    Fp got = fma(a,b,c);
    // correctly-rounded fused reference: long double fma (true fused) then round to Fp
    Fp ref = Fp( std::fma((long double)a, (long double)b, (long double)c) );
    ++total;
    if (got != ref) {
      ++mismatch;
      double ul = std::ldexp(std::fmax(std::fabs(double(ref)),1e-30), -(p-1));
      double e = std::fabs(double(got)-double(ref))/ul;
      if (e>maxUlp) maxUlp=e;
    }
  }
  std::cout<<nm<<" p="<<p<<": fma != correctly-rounded-fused in "<<mismatch<<"/"<<total
           <<" cases, max "<<maxUlp<<" ulp\n";
}
// the sharp two_prod test: fma(a,b,-(a*b)) should be the exact residual if fused
template<typename Fp> void residual(const char* nm){
  std::mt19937_64 rng(5); std::uniform_real_distribution<double> ud(0.5,2.0);
  int zero=0, total=0;
  for(int i=0;i<100000;++i){
    Fp a(ud(rng)), b(ud(rng));
    Fp prod = a*b;
    Fp r = fma(a, b, -prod);     // fused: exact residual (often nonzero); naive: round(prod)-prod = 0
    Fp exactResid = Fp((long double)a*(long double)b - (long double)prod);
    ++total;
    if (double(r)==0.0 && double(exactResid)!=0.0) ++zero;  // would-be-zero only if NOT fused
  }
  std::cout<<nm<<" residual: fma(a,b,-a*b)==0 while true residual!=0 in "<<zero<<"/"<<total<<" (nonzero count => NOT fused)\n";
}
int main(){
  check<cfloat<24,5,uint16_t,true,false,false>>("cf24,5");
  check<cfloat<32,8,uint32_t,true,false,false>>("cf32,8");
  residual<cfloat<24,5,uint16_t,true,false,false>>("cf24,5");
  return 0;
}
