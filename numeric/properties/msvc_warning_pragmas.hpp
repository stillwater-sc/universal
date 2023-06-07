// msvc_warning_pragmas.hpp : include file to disable MSVC warnings
//
#pragma once
#ifndef NDEBUG
#pragma warning(disable : 4514) // warning C4514: 'std::complex<float>::complex': unreferenced inline function has been removed
#pragma warning(disable : 4571) // warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
#pragma warning(disable : 4625) // warning C4625: 'std::moneypunct<char,true>': copy constructor was implicitly defined as deleted
#pragma warning(disable : 4626) // warning C4626: 'std::codecvt_base': assignment operator was implicitly defined as deleted
#pragma warning(disable : 4710) // warning C4710: 'int swprintf_s(wchar_t *const ,const size_t,const wchar_t *const ,...)': function not inlined
#pragma warning(disable : 4774) // warning C4774: 'sprintf_s' : format string expected in argument 3 is not a string literal
#pragma warning(disable : 4820) // warning C4820: 'std::_Mpunct<_Elem>': '4' bytes padding added after data member 'std::_Mpunct<_Elem>::_Kseparator'
#pragma warning(disable : 5026) // warning C5026 : 'std::_Generic_error_category' : move constructor was implicitly defined as deleted
#pragma warning(disable : 5027) // warning C5027 : 'std::_Generic_error_category' : move assignment operator was implicitly defined as deleted
#endif 
