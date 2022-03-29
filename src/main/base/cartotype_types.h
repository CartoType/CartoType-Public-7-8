/*
cartotype_types.h
Copyright (C) 2004-2016 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_TYPES_H__
#define CARTOTYPE_TYPES_H__

#if defined (__unix__) || defined(__APPLE__) || (_MSC_VER >= 1600)

#include <stdint.h>

#else

// Define our own integer limits, because pstdint.h makes the minimum values unsigned, unless they already exist.
#ifndef INT8_MAX
    #define INT8_MAX 127
    #define INT16_MAX 32767
    #define INT32_MAX 2147483647
    #define INT64_MAX 9223372036854775807LL
    #define INT8_MIN -128
    #define INT16_MIN -32768
    #define INT32_MIN (-INT32_MAX-1)
    #define INT64_MIN (-INT64_MAX-1)
    #define UINT8_MAX 255
    #define UINT16_MAX 65535
    #define UINT32_MAX 4294967295U
    #define UINT64_MAX 18446744073709551615ULL
#endif

#include "pstdint.h"

#ifdef _WIN32_WCE
#define _INTPTR_T_DEFINED
#define _UINTPTR_T_DEFINED
#endif

#endif

/** A namespace for the CartoType library. */
namespace CartoType
{

// Microsoft Visual C++
#ifdef _MSC_VER

    /*
    Insist on a version of Microsoft Visual C++ no earlier than Visual Studio 2015,
    which has the internal version number 1900.
    */
    #if (_MSC_VER >= 1900)

    // Disable warnings so that warning level 4 can be used.
    #pragma warning(disable: 4511) // copy constructor could not be generated
    #pragma warning(disable: 4512) // assignment operator could not be generated
    #pragma warning(disable: 4514) // unreferenced inline function has been removed
    #pragma warning(disable: 4710) // function not inlined

    // enable some warnings
    #pragma warning(3: 4265) // class has virtual functions, but destructor is not virtual

    // Define _DEBUG if needed: Microsoft Embedded C++ defines DEBUG only.
    #if defined(DEBUG) || defined(_DEBUG)
        #undef   DEBUG
        #define  DEBUG
        #undef  _DEBUG
        #define _DEBUG
    #endif

    #else
    #error CartoType needs Microsoft Visual Studio 2015 or later.
    #endif

#endif

/**
This CartoType-specific version of DBL_MAX is supplied to avoid the bug in
C++ Builder, which defines DBL_MAX as a very small number.
It uses a different name from DBL_MAX in case DBL_MAX is defined as a macro.
*/
const double CT_DBL_MAX = 1.7976931348623158e+308;

#if defined(_WIN32) || defined(_WIN64)
/*
The header WinUser.h defines DrawText, FindText, etc., as DrawTextA or DrawTextW,
FindTextA or FindTextW, etc., which causes conflicts with CartoType functions of
the same names.
*/
#undef DrawText
#undef FindText
#undef LoadIcon
#endif

} // namespace CartoType

#endif // CARTOTYPE_TYPES_H__
