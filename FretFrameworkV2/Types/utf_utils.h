//==================================================================================================
//  File:       unicode_utils.h
//
//  Summary:    Header file for fast UTF-8 to UTF-32/UTF-16 conversion routines.
//
//  Copyright (c) 2018 Bob Steagall and KEWB Computing, All Rights Reserved
//==================================================================================================
//
#ifndef KEWB_UNICODE_UTILS_H_DEFINED
#define KEWB_UNICODE_UTILS_H_DEFINED

//#include <cstdint>
#include <string>

//- Detect the compiler; only Clang, GCC, and Visual C++ are currently supported.
//
#if defined __clang__

    #define KEWB_COMPILER_CLANG
    #ifdef __OPTIMIZE__
        #define KEWB_FORCE_INLINE   inline __attribute__ ((always_inline))
    #else
        #define KEWB_FORCE_INLINE   inline
    #endif
    #define KEWB_ALIGN_FN   __attribute__ ((aligned (128)))

#elif defined __GNUG__ || defined __GNUC__

    #define KEWB_COMPILER_GCC
    #ifdef __OPTIMIZE__
        #define KEWB_FORCE_INLINE   inline __attribute__ ((always_inline))
    #else
        #define KEWB_FORCE_INLINE   inline
    #endif
    #define KEWB_ALIGN_FN   __attribute__ ((aligned (128)))

#elif defined _MSC_VER

    #define KEWB_COMPILER_MSVC
    #ifdef NDEBUG
        #define KEWB_FORCE_INLINE   inline __forceinline
    #else
        #define KEWB_FORCE_INLINE   inline
    #endif
    #define KEWB_ALIGN_FN

#else
    #error "Unsupported combination of compiler and platform"
#endif

//- Detect OS and include relevant SSE headers; only Linux and Windows are currently supported.
//
#if defined __linux__
    #define KEWB_PLATFORM_LINUX
#elif defined _WIN32
    #define KEWB_PLATFORM_WINDOWS
#else
    #error "Unsupported combination of compiler and platform"
#endif

namespace uu {
//--------------------------------------------------------------------------------------------------
/// \brief  Traits style class to perform conversions from UTF-8 to UTF-32/UTF-16
///
/// \details
///     This traits-style class provides a demonstration of functions for converting strings
///     of UTF-8 code units to strings of UTF-32 code points, as well as transcoding UTF-8
///     into strings of UTF-16 code units.  Its focus is on converting _from_ UTF-8 as quickly
///     as possible, although it does include member functions for converting a UTF-32 code
///     point into sequences of UTF-8/UTF-16 code units.
///
///     It implements conversion from UTF-8 in three different, but related ways:
///       * using a purely DFA-based approach to recognizing valid sequences of UTF-8 code units;
///       * using the DFA-based approach with a short-circuit optimization for ASCII code units;
///       * using the DFA-based approach with an SSE-based optimization for ASCII code units.
///
///     The member functions implement STL-style argument ordering, with source arguments on the
///     left and destination arguments on the right.  The string-to-string conversion member
///     functions are analogous to std::copy() in that the first two arguments define an input
///     range and the third argument defines the starting point of the output range.
///
///     This class is not intended for production usage, as it does not currently provide a
///     mechanism for reporting/handling errors.  No checking is done for null pointers; it
///     is assumed that the input and output pointers sensibly point to buffers that exist.
///
///     Finally, please note that this was developed and tested on x64/x86 hardware, and so
///     there is an implicit assumption that UTF-32 code points and UTF-16 code units are
///     little endian.
//--------------------------------------------------------------------------------------------------
//
class UtfUtils
{
  public:
    using ptrdiff_t = std::ptrdiff_t;

  public:
    static  bool        GetCodePoint(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t& cdpt) noexcept;

    static  uint64_t    GetCodeUnits(char32_t cdpt, unsigned char* pDst) noexcept;

    //- Conversion to UTF-32/UTF-16 using fastest typical (lookup/computation on first code unit).
    //  These member functions are wrappers to the '*BigTableConvert' and '*SmallTableConvert'
    //  member functions declared further down.
    //
    static  ptrdiff_t   BasicConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept;
    static  ptrdiff_t   FastConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept;
    static  ptrdiff_t   SseConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept;

    //- Conversion to UTF-32/UTF-16 using pre-computed first code unit lookup table.
    //
    static  ptrdiff_t   BasicBigTableConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept;
    static  ptrdiff_t   FastBigTableConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept;
    static  ptrdiff_t   SseBigTableConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept;

    //- Conversion to UTF-32/UTF-16 using small lookup table and masking operations on first code unit.
    //
    static  ptrdiff_t   BasicSmallTableConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept;
    static  ptrdiff_t   FastSmallTableConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept;
    static  ptrdiff_t   SseSmallTableConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept;

  private:
    enum CharClass : uint8_t
    {
        ILL = 0,    //- C0..C1, F5..FF  ILLEGAL octets that should never appear in a UTF-8 sequence
                    //
        ASC = 1,    //- 00..7F          ASCII leading byte range
                    //
        CR1 = 2,    //- 80..8F          Continuation range 1
        CR2 = 3,    //- 90..9F          Continuation range 2
        CR3 = 4,    //- A0..BF          Continuation range 3
                    //
        L2A = 5,    //- C2..DF          Leading byte range A / 2-byte sequence
                    //
        L3A = 6,    //- E0              Leading byte range A / 3-byte sequence
        L3B = 7,    //- E1..EC, EE..EF  Leading byte range B / 3-byte sequence
        L3C = 8,    //- ED              Leading byte range C / 3-byte sequence
                    //
        L4A = 9,    //- F0              Leading byte range A / 4-byte sequence
        L4B = 10,   //- F1..F3          Leading byte range B / 4-byte sequence
        L4C = 11,   //- F4              Leading byte range C / 4-byte sequence
    };

    enum State : uint8_t
    {
        BGN = 0,    //- Start
        ERR = 12,   //- Invalid sequence
                    //
        CS1 = 24,   //- Continuation state 1
        CS2 = 36,   //- Continuation state 2
        CS3 = 48,   //- Continuation state 3
                    //
        P3A = 60,   //- Partial 3-byte sequence state A
        P3B = 72,   //- Partial 3-byte sequence state B
                    //
        P4A = 84,   //- Partial 4-byte sequence state A
        P4B = 96,   //- Partial 4-byte sequence state B
                    //
        END = BGN,  //- Start and End are the same state!
        err = ERR,  //- For readability in the state transition table
    };

    struct FirstUnitInfo
    {
        unsigned char   mFirstOctet;
        State           mNextState;
    };

    struct alignas(2048) LookupTables
    {
        FirstUnitInfo   maFirstUnitTable[256];
        CharClass       maOctetCategory[256];
        State           maTransitions[108];
        std::uint8_t    maFirstOctetMask[16];
    };

  private:
      static constexpr LookupTables smTables
      {
          //- Initialize the maFirstUnitTable member array.  This array implements a lookup table that
          //  maps the first code unit of a sequence to: 1. a pre-masked value to start the computation
          //  of the resulting code point; and, 2. the next state in the DFA for this code unit.
          //
          {
              { 0x00, BGN },   //- 0x00
              { 0x01, BGN },   //- 0x01
              { 0x02, BGN },   //- 0x02
              { 0x03, BGN },   //- 0x03
              { 0x04, BGN },   //- 0x04
              { 0x05, BGN },   //- 0x05
              { 0x06, BGN },   //- 0x06
              { 0x07, BGN },   //- 0x07
              { 0x08, BGN },   //- 0x08
              { 0x09, BGN },   //- 0x09
              { 0x0A, BGN },   //- 0x0A
              { 0x0B, BGN },   //- 0x0B
              { 0x0C, BGN },   //- 0x0C
              { 0x0D, BGN },   //- 0x0D
              { 0x0E, BGN },   //- 0x0E
              { 0x0F, BGN },   //- 0x0F

              { 0x10, BGN },   //- 0x10
              { 0x11, BGN },   //- 0x11
              { 0x12, BGN },   //- 0x12
              { 0x13, BGN },   //- 0x13
              { 0x14, BGN },   //- 0x14
              { 0x15, BGN },   //- 0x15
              { 0x16, BGN },   //- 0x16
              { 0x17, BGN },   //- 0x17
              { 0x18, BGN },   //- 0x18
              { 0x19, BGN },   //- 0x19
              { 0x1A, BGN },   //- 0x1A
              { 0x1B, BGN },   //- 0x1B
              { 0x1C, BGN },   //- 0x1C
              { 0x1D, BGN },   //- 0x1D
              { 0x1E, BGN },   //- 0x1E
              { 0x1F, BGN },   //- 0x1F

              { 0x20, BGN },   //- 0x20
              { 0x21, BGN },   //- 0x21
              { 0x22, BGN },   //- 0x22
              { 0x23, BGN },   //- 0x23
              { 0x24, BGN },   //- 0x24
              { 0x25, BGN },   //- 0x25
              { 0x26, BGN },   //- 0x26
              { 0x27, BGN },   //- 0x27
              { 0x28, BGN },   //- 0x28
              { 0x29, BGN },   //- 0x29
              { 0x2A, BGN },   //- 0x2A
              { 0x2B, BGN },   //- 0x2B
              { 0x2C, BGN },   //- 0x2C
              { 0x2D, BGN },   //- 0x2D
              { 0x2E, BGN },   //- 0x2E
              { 0x2F, BGN },   //- 0x2F

              { 0x30, BGN },   //- 0x30
              { 0x31, BGN },   //- 0x31
              { 0x32, BGN },   //- 0x32
              { 0x33, BGN },   //- 0x33
              { 0x34, BGN },   //- 0x34
              { 0x35, BGN },   //- 0x35
              { 0x36, BGN },   //- 0x36
              { 0x37, BGN },   //- 0x37
              { 0x38, BGN },   //- 0x38
              { 0x39, BGN },   //- 0x39
              { 0x3A, BGN },   //- 0x3A
              { 0x3B, BGN },   //- 0x3B
              { 0x3C, BGN },   //- 0x3C
              { 0x3D, BGN },   //- 0x3D
              { 0x3E, BGN },   //- 0x3E
              { 0x3F, BGN },   //- 0x3F

              { 0x40, BGN },   //- 0x40
              { 0x41, BGN },   //- 0x41
              { 0x42, BGN },   //- 0x42
              { 0x43, BGN },   //- 0x43
              { 0x44, BGN },   //- 0x44
              { 0x45, BGN },   //- 0x45
              { 0x46, BGN },   //- 0x46
              { 0x47, BGN },   //- 0x47
              { 0x48, BGN },   //- 0x48
              { 0x49, BGN },   //- 0x49
              { 0x4A, BGN },   //- 0x4A
              { 0x4B, BGN },   //- 0x4B
              { 0x4C, BGN },   //- 0x4C
              { 0x4D, BGN },   //- 0x4D
              { 0x4E, BGN },   //- 0x4E
              { 0x4F, BGN },   //- 0x4F

              { 0x50, BGN },   //- 0x50
              { 0x51, BGN },   //- 0x51
              { 0x52, BGN },   //- 0x52
              { 0x53, BGN },   //- 0x53
              { 0x54, BGN },   //- 0x54
              { 0x55, BGN },   //- 0x55
              { 0x56, BGN },   //- 0x56
              { 0x57, BGN },   //- 0x57
              { 0x58, BGN },   //- 0x58
              { 0x59, BGN },   //- 0x59
              { 0x5A, BGN },   //- 0x5A
              { 0x5B, BGN },   //- 0x5B
              { 0x5C, BGN },   //- 0x5C
              { 0x5D, BGN },   //- 0x5D
              { 0x5E, BGN },   //- 0x5E
              { 0x5F, BGN },   //- 0x5F

              { 0x60, BGN },   //- 0x60
              { 0x61, BGN },   //- 0x61
              { 0x62, BGN },   //- 0x62
              { 0x63, BGN },   //- 0x63
              { 0x64, BGN },   //- 0x64
              { 0x65, BGN },   //- 0x65
              { 0x66, BGN },   //- 0x66
              { 0x67, BGN },   //- 0x67
              { 0x68, BGN },   //- 0x68
              { 0x69, BGN },   //- 0x69
              { 0x6A, BGN },   //- 0x6A
              { 0x6B, BGN },   //- 0x6B
              { 0x6C, BGN },   //- 0x6C
              { 0x6D, BGN },   //- 0x6D
              { 0x6E, BGN },   //- 0x6E
              { 0x6F, BGN },   //- 0x6F

              { 0x70, BGN },   //- 0x70
              { 0x71, BGN },   //- 0x71
              { 0x72, BGN },   //- 0x72
              { 0x73, BGN },   //- 0x73
              { 0x74, BGN },   //- 0x74
              { 0x75, BGN },   //- 0x75
              { 0x76, BGN },   //- 0x76
              { 0x77, BGN },   //- 0x77
              { 0x78, BGN },   //- 0x78
              { 0x79, BGN },   //- 0x79
              { 0x7A, BGN },   //- 0x7A
              { 0x7B, BGN },   //- 0x7B
              { 0x7C, BGN },   //- 0x7C
              { 0x7D, BGN },   //- 0x7D
              { 0x7E, BGN },   //- 0x7E
              { 0x7F, BGN },   //- 0x7F

              { 0x00, ERR },   //- 0x80
              { 0x01, ERR },   //- 0x81
              { 0x02, ERR },   //- 0x82
              { 0x03, ERR },   //- 0x83
              { 0x04, ERR },   //- 0x84
              { 0x05, ERR },   //- 0x85
              { 0x06, ERR },   //- 0x86
              { 0x07, ERR },   //- 0x87
              { 0x08, ERR },   //- 0x88
              { 0x09, ERR },   //- 0x89
              { 0x0A, ERR },   //- 0x8A
              { 0x0B, ERR },   //- 0x8B
              { 0x0C, ERR },   //- 0x8C
              { 0x0D, ERR },   //- 0x8D
              { 0x0E, ERR },   //- 0x8E
              { 0x0F, ERR },   //- 0x8F

              { 0x10, ERR },   //- 0x90
              { 0x11, ERR },   //- 0x91
              { 0x12, ERR },   //- 0x92
              { 0x13, ERR },   //- 0x93
              { 0x14, ERR },   //- 0x94
              { 0x15, ERR },   //- 0x95
              { 0x16, ERR },   //- 0x96
              { 0x17, ERR },   //- 0x97
              { 0x18, ERR },   //- 0x98
              { 0x19, ERR },   //- 0x99
              { 0x1A, ERR },   //- 0x9A
              { 0x1B, ERR },   //- 0x9B
              { 0x1C, ERR },   //- 0x9C
              { 0x1D, ERR },   //- 0x9D
              { 0x1E, ERR },   //- 0x9E
              { 0x1F, ERR },   //- 0x9F

              { 0x20, ERR },   //- 0xA0
              { 0x21, ERR },   //- 0xA1
              { 0x22, ERR },   //- 0xA2
              { 0x23, ERR },   //- 0xA3
              { 0x24, ERR },   //- 0xA4
              { 0x25, ERR },   //- 0xA5
              { 0x26, ERR },   //- 0xA6
              { 0x27, ERR },   //- 0xA7
              { 0x28, ERR },   //- 0xA8
              { 0x29, ERR },   //- 0xA9
              { 0x2A, ERR },   //- 0xAA
              { 0x2B, ERR },   //- 0xAB
              { 0x2C, ERR },   //- 0xAC
              { 0x2D, ERR },   //- 0xAD
              { 0x2E, ERR },   //- 0xAE
              { 0x2F, ERR },   //- 0xAF

              { 0x30, ERR },   //- 0xB0
              { 0x31, ERR },   //- 0xB1
              { 0x32, ERR },   //- 0xB2
              { 0x33, ERR },   //- 0xB3
              { 0x34, ERR },   //- 0xB4
              { 0x35, ERR },   //- 0xB5
              { 0x36, ERR },   //- 0xB6
              { 0x37, ERR },   //- 0xB7
              { 0x38, ERR },   //- 0xB8
              { 0x39, ERR },   //- 0xB9
              { 0x3A, ERR },   //- 0xBA
              { 0x3B, ERR },   //- 0xBB
              { 0x3C, ERR },   //- 0xBC
              { 0x3D, ERR },   //- 0xBD
              { 0x3E, ERR },   //- 0xBE
              { 0x3F, ERR },   //- 0xBF

              { 0xC0, ERR },   //- 0xC0
              { 0xC1, ERR },   //- 0xC1
              { 0x02, CS1 },   //- 0xC2
              { 0x03, CS1 },   //- 0xC3
              { 0x04, CS1 },   //- 0xC4
              { 0x05, CS1 },   //- 0xC5
              { 0x06, CS1 },   //- 0xC6
              { 0x07, CS1 },   //- 0xC7
              { 0x08, CS1 },   //- 0xC8
              { 0x09, CS1 },   //- 0xC9
              { 0x0A, CS1 },   //- 0xCA
              { 0x0B, CS1 },   //- 0xCB
              { 0x0C, CS1 },   //- 0xCC
              { 0x0D, CS1 },   //- 0xCD
              { 0x0E, CS1 },   //- 0xCE
              { 0x0F, CS1 },   //- 0xCF

              { 0x10, CS1 },   //- 0xD0
              { 0x11, CS1 },   //- 0xD1
              { 0x12, CS1 },   //- 0xD2
              { 0x13, CS1 },   //- 0xD3
              { 0x14, CS1 },   //- 0xD4
              { 0x15, CS1 },   //- 0xD5
              { 0x16, CS1 },   //- 0xD6
              { 0x17, CS1 },   //- 0xD7
              { 0x18, CS1 },   //- 0xD8
              { 0x19, CS1 },   //- 0xD9
              { 0x1A, CS1 },   //- 0xDA
              { 0x1B, CS1 },   //- 0xDB
              { 0x1C, CS1 },   //- 0xDC
              { 0x1D, CS1 },   //- 0xDD
              { 0x1E, CS1 },   //- 0xDE
              { 0x1F, CS1 },   //- 0xDF

              { 0x00, P3A },   //- 0xE0
              { 0x01, CS2 },   //- 0xE1
              { 0x02, CS2 },   //- 0xE2
              { 0x03, CS2 },   //- 0xE3
              { 0x04, CS2 },   //- 0xE4
              { 0x05, CS2 },   //- 0xE5
              { 0x06, CS2 },   //- 0xE6
              { 0x07, CS2 },   //- 0xE7
              { 0x08, CS2 },   //- 0xE8
              { 0x09, CS2 },   //- 0xE9
              { 0x0A, CS2 },   //- 0xEA
              { 0x0B, CS2 },   //- 0xEB
              { 0x0C, CS2 },   //- 0xEC
              { 0x0D, P3B },   //- 0xED
              { 0x0E, CS2 },   //- 0xEE
              { 0x0F, CS2 },   //- 0xEF

              { 0x00, P4A },   //- 0xF0
              { 0x01, CS3 },   //- 0xF1
              { 0x02, CS3 },   //- 0xF2
              { 0x03, CS3 },   //- 0xF3
              { 0x04, P4B },   //- 0xF4
              { 0xF5, ERR },   //- 0xF5
              { 0xF6, ERR },   //- 0xF6
              { 0xF7, ERR },   //- 0xF7
              { 0xF8, ERR },   //- 0xF8
              { 0xF9, ERR },   //- 0xF9
              { 0xFA, ERR },   //- 0xFA
              { 0xFB, ERR },   //- 0xFB
              { 0xFC, ERR },   //- 0xFC
              { 0xFD, ERR },   //- 0xFD
              { 0xFE, ERR },   //- 0xFE
              { 0xFF, ERR },   //- 0xFF
          },

          //- Initialize the maOctetCategory member array.  This array implements a lookup table
          //  that maps an input octet to a corresponding octet category.
          //
          //   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
          //============================================================================================
          {
              ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, //- 00..0F
              ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, //- 10..1F
              ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, //- 20..2F
              ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, //- 30..3F
                                                                                              //
              ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, //- 40..4F
              ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, //- 50..5F
              ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, //- 60..6F
              ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, //- 70..7F
                                                                                              //
              CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, //- 80..8F
              CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, //- 90..9F
              CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, //- A0..AF
              CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, //- B0..BF
                                                                                              //
              ILL, ILL, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, //- C0..CF
              L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, //- D0..DF
              L3A, L3B, L3B, L3B, L3B, L3B, L3B, L3B, L3B, L3B, L3B, L3B, L3B, L3C, L3B, L3B, //- E0..EF
              L4A, L4B, L4B, L4B, L4C, ILL, ILL, ILL, ILL, ILL, ILL, ILL, ILL, ILL, ILL, ILL, //- F0..FF
          },

          //- Initialize the maTransitions member array.  This array implements a lookup table that,
          //  given the current DFA state and an input code unit, indicates the next DFA state. 
          //
          //  ILL  ASC  CR1  CR2  CR3  L2A  L3A  L3B  L3C  L4A  L4B  L4C  CLASS/STATE
          //=========================================================================
          {
              err, END, err, err, err, CS1, P3A, CS2, P3B, P4A, CS3, P4B,	  //- BGN|END
              err, err, err, err, err, err, err, err, err, err, err, err,   //- ERR
                                                                            //
              err, err, END, END, END, err, err, err, err, err, err, err,   //- CS1
              err, err, CS1, CS1, CS1, err, err, err, err, err, err, err,   //- CS2
              err, err, CS2, CS2, CS2, err, err, err, err, err, err, err,   //- CS3
                                                                            //
              err, err, err, err, CS1, err, err, err, err, err, err, err,   //- P3A
              err, err, CS1, CS1, err, err, err, err, err, err, err, err,   //- P3B
                                                                            //
              err, err, err, CS2, CS2, err, err, err, err, err, err, err,   //- P4A
              err, err, CS2, err, err, err, err, err, err, err, err, err,   //- P4B
          },

          //- Initialize the maFirstOctetMask member array.  This array implements a lookup table that
          //  maps a character class to a mask that is applied to the first code unit in a sequence.
          //
          {
              0xFF,   //- ILL - C0..C1, F5..FF    Illegal code unit
                      //
              0x7F,   //- ASC - 00..7F            ASCII byte range
                      //
              0x3F,   //- CR1 - 80..8F            Continuation range 1
              0x3F,   //- CR2 - 90..9F            Continuation range 2
              0x3F,   //- CR3 - A0..BF            Continuation range 3
                      //
              0x1F,   //- L2A - C2..DF            Leading byte range 2A / 2-byte sequence
                      //
              0x0F,   //- L3A - E0                Leading byte range 3A / 3-byte sequence
              0x0F,   //- L3B - E1..EC, EE..EF    Leading byte range 3B / 3-byte sequence
              0x0F,   //- L3C - ED                Leading byte range 3C / 3-byte sequence
                      //
              0x07,   //- L4A - F0                Leading byte range 4A / 4-byte sequence
              0x07,   //- L4B - F1..F3            Leading byte range 4B / 4-byte sequence
              0x07,   //- L4C - F4                Leading byte range 4C / 4-byte sequence
          },
      };

  private:
    static  int32_t AdvanceWithBigTable(unsigned char const*& pSrc, unsigned char const* pSrcEnd, char32_t& cdpt) noexcept;
    static  int32_t AdvanceWithSmallTable(unsigned char const*& pSrc, unsigned char const* pSrcEnd, char32_t& cdpt) noexcept;

    static  void    ConvertAsciiWithSse(unsigned char const*& pSrc, char32_t*& pDst) noexcept;
    static  int32_t GetTrailingZeros(int32_t x) noexcept;
};

//--------------------------------------------------------------------------------------------------
/// \brief  Converts a sequence of UTF-8 code units to a UTF-32 code point.
///
/// \param pSrc
///     A non-null pointer defining the beginning of the input range of code units.
/// \param pSrcEnd
///     A non-null past-the-end pointer defining the end of the input range.
/// \param cdpt
///     A mutable reference to a char32_t variable which will receive the code unit.
///
/// \returns
///     Boolean value `true` on success.
//--------------------------------------------------------------------------------------------------
//
KEWB_FORCE_INLINE bool
UtfUtils::GetCodePoint(unsigned char const* pSrc, unsigned char const* const pSrcEnd, char32_t& cdpt) noexcept
{
    return (pSrc < pSrcEnd) ? (AdvanceWithSmallTable(pSrc, pSrcEnd, cdpt) != ERR) : false;
}

//--------------------------------------------------------------------------------------------------
/// \brief  Converts a UTF-32 code point to a sequence of one to four UTF-8 code units.
///
/// \details
///     Note that the conversion performed by this member function is "unsafe", in that it does
///     not check for invalid/illegal values of the input code point.
///
/// \param cdpt
///     A the source code unit.
/// \param pDst
///     A reference to non-null pointer defining the beginning of the output range of code units.
///
/// \returns
///     The number of UTF-8 code units resulting from the conversion of `cdpt`.
//--------------------------------------------------------------------------------------------------
//
KEWB_FORCE_INLINE std::uint64_t
UtfUtils::GetCodeUnits(char32_t cdpt, unsigned char* pDst) noexcept
{
    if (cdpt <= 0x7F)
    {
        pDst[0] = (unsigned char)(cdpt);
        return 1;
    }
    else if (cdpt <= 0x7FF)
    {
        pDst[0] = (unsigned char)(0xC0 | ((cdpt >> 6) & 0x1F));
        pDst[1] = (unsigned char)(0x80 | (cdpt & 0x3F));
        return 2;
    }
    else if (cdpt <= 0xFFFF)
    {
        pDst[0] = (unsigned char)(0xE0 | ((cdpt >> 12) & 0x0F));
        pDst[1] = (unsigned char)(0x80 | ((cdpt >> 6) & 0x3F));
        pDst[2] = (unsigned char)(0x80 | (cdpt & 0x3F));
        return 3;
    }
    else if (cdpt <= 0x10FFFF)
    {
        pDst[0] = (unsigned char)(0xF0 | ((cdpt >> 18) & 0x07));
        pDst[1] = (unsigned char)(0x80 | ((cdpt >> 12) & 0x3F));
        pDst[2] = (unsigned char)(0x80 | ((cdpt >> 6) & 0x3F));
        pDst[3] = (unsigned char)(0x80 | (cdpt & 0x3F));
        return 4;
    }
    return 0;
}

//--------------------------------------------------------------------------------------------------
/// \brief  Converts a sequence of UTF-8 code units to a sequence of UTF-32 code points.
///
/// \param pSrc
///     A non-null pointer defining the beginning of the code unit input range.
/// \param pSrcEnd
///     A non-null past-the-end pointer defining the end of the code unit input range.
/// \param pDst
///     A non-null pointer defining the beginning of the code point output range.
///
/// \returns
///     If successful, the number of UTF-32 code points written; otherwise -1 is returned to
///     indicate an error was encountered.
//--------------------------------------------------------------------------------------------------
//
KEWB_FORCE_INLINE ptrdiff_t
UtfUtils::BasicConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept
{
    return BasicBigTableConvert(pSrc, pSrcEnd, pDst);
}

//--------------------------------------------------------------------------------------------------
/// \brief  Converts a sequence of UTF-8 code units to a sequence of UTF-32 code points.
///
/// \param pSrc
///     A non-null pointer defining the beginning of the code unit input range.
/// \param pSrcEnd
///     A non-null past-the-end pointer defining the end of the code unit input range.
/// \param pDst
///     A non-null pointer defining the beginning of the code point output range.
///
/// \returns
///     If successful, the number of UTF-32 code points written; otherwise -1 is returned to
///     indicate an error was encountered.
//--------------------------------------------------------------------------------------------------
//
KEWB_FORCE_INLINE ptrdiff_t
UtfUtils::FastConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept
{
    return FastBigTableConvert(pSrc, pSrcEnd, pDst);
}

//--------------------------------------------------------------------------------------------------
/// \brief  Converts a sequence of UTF-8 code units to a sequence of UTF-32 code points.
///
/// \param pSrc
///     A non-null pointer defining the beginning of the code unit input range.
/// \param pSrcEnd
///     A non-null past-the-end pointer defining the end of the code unit input range.
/// \param pDst
///     A non-null pointer defining the beginning of the code point output range.
///
/// \returns
///     If successful, the number of UTF-32 code points written; otherwise -1 is returned to
///     indicate an error was encountered.
//--------------------------------------------------------------------------------------------------
//
KEWB_FORCE_INLINE ptrdiff_t
UtfUtils::SseConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept
{
    return SseBigTableConvert(pSrc, pSrcEnd, pDst);
}

//--------------------------------------------------------------------------------------------------
/// \brief  Converts a sequence of UTF-8 code units to a UTF-32 code point.
///
/// \details
///     This static member function reads input octets and uses them to traverse a DFA that
///     recognizes valid sequences of UTF-8 code units.  It is the heart of all non-ASCII
///     conversions in all member functions of this class.  This function uses the "big"
///     first-unit lookup table and the state machine table to traverse the DFA.
///
/// \param pSrc
///     A reference to a non-null pointer defining the beginning of the code unit input range.
/// \param pSrcEnd
///     A non-null past-the-end pointer defining the end of the code unit input range.
/// \param cdpt
///     A reference to the output code point.
///
/// \returns
///     An internal flag describing the current DFA state.
//--------------------------------------------------------------------------------------------------
//
KEWB_FORCE_INLINE int32_t
UtfUtils::AdvanceWithBigTable(unsigned char const*& pSrc, unsigned char const* const pSrcEnd, char32_t& cdpt) noexcept
{
    FirstUnitInfo   info;   //- The descriptor for the first code unit
    char32_t        unit;   //- The current UTF-8 code unit
    int32_t         type;   //- The current code unit's character class
    int32_t         curr;   //- The current DFA state

    info = smTables.maFirstUnitTable[*pSrc++];              //- Look up the first code unit descriptor
    cdpt = info.mFirstOctet;                                //- From it, get the initial code point value
    curr = info.mNextState;                                 //- From it, get the second state

    while (curr > ERR)
    {
        if (pSrc < pSrcEnd)
        {
            unit = *pSrc++;                                 //- Cache the current code unit
            cdpt = (cdpt << 6) | (unit & 0x3F);             //- Adjust code point with continuation bits
            type = smTables.maOctetCategory[unit];          //- Look up the code unit's character class
            curr = smTables.maTransitions[curr + type];     //- Look up the next state
        }
        else
        {
            return ERR;
        }
    }
    return curr;
}

//--------------------------------------------------------------------------------------------------
/// \brief  Converts a sequence of UTF-8 code units to a UTF-32 code point.
///
/// \details
///     This static member function reads input octets and uses them to traverse a DFA that
///     recognizes valid sequences of UTF-8 code units.  It is the heart of all non-ASCII
///     conversions in all member functions of this class.  This function uses the "small"
///     first-unit lookup table and the state machine table to traverse the DFA.
///
/// \param pSrc
///     A reference to a non-null pointer defining the beginning of the code unit input range.
/// \param pSrcEnd
///     A non-null past-the-end pointer defining the end of the code unit input range.
/// \param cdpt
///     A reference to the output code point.
///
/// \returns
///     An internal flag describing the current DFA state.
//--------------------------------------------------------------------------------------------------
//
KEWB_FORCE_INLINE int32_t
UtfUtils::AdvanceWithSmallTable(unsigned char const*& pSrc, unsigned char const* const pSrcEnd, char32_t& cdpt) noexcept
{
    char32_t    unit;   //- The current UTF-8 code unit
    int32_t     type;   //- The current code unit's character class
    int32_t     curr;   //- The current DFA state

    unit = *pSrc++;                                         //- Cache the first code unit
    type = smTables.maOctetCategory[unit];                  //- Get the first code unit's character class
    cdpt = smTables.maFirstOctetMask[type] & unit;          //- Apply the first octet mask 
    curr = smTables.maTransitions[type];                    //- Look up the second state

    while (curr > ERR)
    {
        if (pSrc < pSrcEnd)
        {
            unit = *pSrc++;                                 //- Cache the current code unit
            cdpt = (cdpt << 6) | (unit & 0x3F);             //- Adjust code point with continuation bits
            type = smTables.maOctetCategory[unit];          //- Look up the code unit's character class
            curr = smTables.maTransitions[curr + type];     //- Look up the next state
        }
        else
        {
            return ERR;
        }
    }
    return curr;
}

}       //- namespace uu
#endif  //- KEWB_UNICODE_UTILS_H_DEFINED
