#ifndef POSIT_NBITS
    #error "you must define POSIT_NBITS first"
#endif

// If we're including this in the internal C++ code
// then we want implementations (which use the C++ code),
// otherwise we want only headers and inline functions.
#ifdef POSIT_IMPLS
#define POSIT_IMPL(...) __VA_ARGS__
#define POSIT_INLINE(...)
#else
#define POSIT_IMPL(...) ;
#define POSIT_INLINE(...) static inline __VA_ARGS__
#endif

#define POSIT_MKNAME(name) POSIT_GLUE4(posit, POSIT_NBITS, _, name)
#define POSIT_T POSIT_MKNAME(t)
#define POSIT_VEC_T(width) POSIT_GLUE5(posit, POSIT_NBITS, x, width, _t)
#define POSIT_API POSIT_GLUE(capi, POSIT_NBITS)

// creates:  posit8_t posit8_subull(posit8_t p, unsigned long long x)
// and also: posit8_t posit8_ullsub(unsigned long long x, posit8_t p)
#define POSIT_OP(rett, op, name, type) \
    POSIT_INLINE(rett POSIT_GLUE(POSIT_MKNAME(op), name)(POSIT_T x, type y) { \
        return POSIT_GLUE3(POSIT_MKNAME(op), p, POSIT_NBITS)( \
            x, POSIT_GLUE(POSIT_MKNAME(from), name)(y)); \
    }) \
    POSIT_INLINE(rett POSIT_GLUE(POSIT_MKNAME(name), op)(type x, POSIT_T y) { \
        return POSIT_GLUE3(POSIT_MKNAME(op), p, POSIT_NBITS)( \
            POSIT_GLUE(POSIT_MKNAME(from), name)(x), y); \
    })

// If you add another op, you need to add it here
// You will need to also write a base op implementation below.
#define POSIT_OPS(name, type) \
    POSIT_OP(POSIT_T, add, name, type) \
    POSIT_OP(POSIT_T, sub, name, type) \
    POSIT_OP(POSIT_T, mul, name, type) \
    POSIT_OP(POSIT_T, div, name, type) \
    POSIT_OP(int, cmp, name, type)

// This creates functions for all ops for all types which can be converted to a posit.
// e.g. float posit8_tof(posit8_t p)  double posit8_tod(posit8_t, double)
// "name" is the mnemonic for the type (f, ld, sll, ul, ui)
// "type" is the type (float, long double, signed long long, unsigned long long, unsigned int)
#define POSIT_FUNCS(name, type) \
	type POSIT_GLUE(POSIT_MKNAME(to), name)(POSIT_T p) POSIT_IMPL({ return POSIT_API::to<type>(p); }) \
	POSIT_T POSIT_GLUE(POSIT_MKNAME(from), name)(type p) POSIT_IMPL({ return POSIT_API::from<type>(p); }) \
    POSIT_OPS(name, type)


// base functions, e.g.  posit8_t posit8_addp8(posit8_t x, posity_t y)
// these functions must be made specially because everything else is defined in terms of them
#define POSIT_BASE_OP(__rett__, __type__, __op__) \
    __rett__ POSIT_GLUE3(POSIT_MKNAME(__op__),p,POSIT_NBITS)(POSIT_T x, POSIT_T y) POSIT_IMPL({ \
        return POSIT_API::__type__<POSIT_GLUE(op_, __op__)<POSIT_API::nbits, POSIT_API::es>>(x, y); \
    }) \
    POSIT_INLINE(__rett__ POSIT_GLUE3(POSIT_MKNAME(p), POSIT_NBITS, __op__)(POSIT_T x, POSIT_T y) { \
        return POSIT_GLUE3(POSIT_MKNAME(__op__), p, POSIT_NBITS)(x, y); \
    }) \
    POSIT_INLINE(__rett__ POSIT_MKNAME(__op__)(POSIT_T x, POSIT_T y) { \
        return POSIT_GLUE3(POSIT_MKNAME(__op__), p, POSIT_NBITS)(x, y); \
    })

// single argument operation
#define POSIT_BASE_OP1(__rett__, __type__, __op__) \
    POSIT_T POSIT_MKNAME(__op__)(POSIT_T x) POSIT_IMPL({ \
        return POSIT_API::__type__<POSIT_GLUE(op_, __op__)<POSIT_API::nbits, POSIT_API::es>>(x); \
    })

#define POSIT_GLUE3(a,b,c) POSIT_GLUE(POSIT_GLUE(a,b),c)
#define POSIT_GLUE4(a,b,c,d) POSIT_GLUE(POSIT_GLUE(a,b),POSIT_GLUE(c,d))
#define POSIT_GLUE5(a,b,c,d,e) POSIT_GLUE(POSIT_GLUE4(a,b,c,d),e)
#define POSIT_GLUE(x,y) _POSIT_GLUE(x,y)
#define _POSIT_GLUE(x,y) x ## y

/////
// We're done defining stuff, now we make functions
/////

#ifndef POSIT_IMPLS
typedef struct POSIT_GLUE3(posit, POSIT_NBITS, x2_s) { POSIT_T x; POSIT_T y; } POSIT_VEC_T(2);
#endif

#if defined(__cplusplus) || defined(_MSC_VER)
void POSIT_MKNAME(str)(char* out, POSIT_T p) POSIT_IMPL({ POSIT_API::format(p, out); })
#else
// Feature of C which is not in C++
// https://hamberg.no/erlend/posts/2013-02-18-static-array-indices.html
void POSIT_MKNAME(str)(char out[static POSIT_MKNAME(str_SIZE)], POSIT_T p);
#endif

// These are implemented in posit_c_api invocations of the OPERATION() macro
POSIT_BASE_OP(POSIT_T, op21, add)
POSIT_BASE_OP(POSIT_VEC_T(2), op22, add_exact)
POSIT_BASE_OP(POSIT_T, op21, sub)
POSIT_BASE_OP(POSIT_VEC_T(2), op22, sub_exact)
POSIT_BASE_OP(POSIT_T, op21, mul)
POSIT_BASE_OP(POSIT_T, op21, div)
POSIT_BASE_OP1(POSIT_T, op11, sqrt)
POSIT_BASE_OP1(POSIT_T, op11, log)
POSIT_BASE_OP1(POSIT_T, op11, exp)


// cmp is special because the return type is int and we need to call a different
// function in the POSIT_API class
int POSIT_GLUE3(POSIT_MKNAME(cmp),p,POSIT_NBITS)(POSIT_T x, POSIT_T y) POSIT_IMPL({
    return POSIT_API::cmp(x, y);
})
POSIT_INLINE(int POSIT_GLUE3(POSIT_MKNAME(p), POSIT_NBITS, cmp)(POSIT_T x, POSIT_T y) {
    return POSIT_GLUE3(POSIT_MKNAME(cmp), p, POSIT_NBITS)(x, y);
})
POSIT_INLINE(int POSIT_MKNAME(cmp)(POSIT_T x, POSIT_T y) {
    return POSIT_GLUE3(POSIT_MKNAME(cmp), p, POSIT_NBITS)(x, y);
})

// posit->posit conversions
POSIT_INLINE(POSIT_T POSIT_GLUE(POSIT_MKNAME(fromp), POSIT_NBITS)(POSIT_T p) { return p; })
#if POSIT_NBITS != 4
POSIT_T POSIT_MKNAME(fromp4)(posit4_t p) POSIT_IMPL({ return POSIT_API::fromp<capi4>(p); })
POSIT_OPS(p4, posit4_t)
#endif
//#if POSIT_NBITS != 8
//POSIT_T POSIT_MKNAME(fromp8)(posit8_t p) POSIT_IMPL({ return POSIT_API::fromp<capi8>(p); })
//POSIT_OPS(p8, posit8_t)
//#endif
#if POSIT_NBITS != 16
POSIT_T POSIT_MKNAME(fromp16)(posit16_t p) POSIT_IMPL({ return POSIT_API::fromp<capi16>(p); })
POSIT_OPS(p16, posit16_t)
#endif
#if POSIT_NBITS != 32
POSIT_T POSIT_MKNAME(fromp32)(posit32_t p) POSIT_IMPL({ return POSIT_API::fromp<capi32>(p); })
POSIT_OPS(p32, posit32_t)
#endif
#if POSIT_NBITS != 64
POSIT_T POSIT_MKNAME(fromp64)(posit64_t p) POSIT_IMPL({ return POSIT_API::fromp<capi64>(p); })
POSIT_OPS(p64, posit64_t)
#endif
#if POSIT_NBITS != 128
POSIT_T POSIT_MKNAME(fromp128)(posit128_t p) POSIT_IMPL({ return POSIT_API::fromp<capi128>(p); })
POSIT_OPS(p128, posit128_t)
#endif
#if POSIT_NBITS != 256
POSIT_T POSIT_MKNAME(fromp256)(posit256_t p) POSIT_IMPL({ return POSIT_API::fromp<capi256>(p); })
POSIT_OPS(p256, posit256_t)
#endif

POSIT_FUNCS(ld, long double)
POSIT_FUNCS(d, double)
POSIT_FUNCS(f, float)
POSIT_FUNCS(sll, long long)
POSIT_FUNCS(sl, long)
POSIT_FUNCS(si, int)
POSIT_FUNCS(ull, unsigned long long)
POSIT_FUNCS(ul, unsigned long)
POSIT_FUNCS(ui, unsigned int)

// cat ../posit/posit_c_macros.h | grep define | sed 's/^.*#define \([A-Za-z0-9_]*\).*$/#undef \1/'
#undef POSIT_NBITS
#undef POSIT_IMPL
#undef POSIT_INLINE
#undef POSIT_IMPL
#undef POSIT_INLINE
#undef POSIT_MKNAME
#undef POSIT_T
#undef POSIT_API
#undef POSIT_OP
#undef POSIT_OPS
#undef POSIT_FUNCS
#undef POSIT_BASE_OP
#undef POSIT_GLUE3
#undef POSIT_GLUE4
#undef POSIT_GLUE
#undef _POSIT_GLUE
