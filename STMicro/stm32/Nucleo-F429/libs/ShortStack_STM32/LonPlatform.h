/****************************************************************************
 *
 *  Filename: LonPlatform.h
 *
 *  Copyright (c) Echelon Corporation 2002-2009.  All rights reserved.
 *
 *  ECHELON MAKES NO REPRESENTATION, WARRANTY, OR CONDITION OF
 *  ANY KIND, EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE OR IN
 *  ANY COMMUNICATION WITH YOU, INCLUDING, BUT NOT LIMITED TO,
 *  ANY IMPLIED WARRANTIES OF MERCHANTABILITY, SATISFACTORY
 *  QUALITY, FITNESS FOR ANY PARTICULAR PURPOSE,
 *  NONINFRINGEMENT, AND THEIR EQUIVALENTS.
 *
 *  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *  PLEASE MAKE SURE TO READ THE INFORMATION IN THIS FILE CAREFULLY, BECAUSE 
 *  IT CONTAINS IMPORTANT PORTING CONSIDERATIONS.
 *  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Description:  This file contains platform dependant flags
 *  and basic data types.  All data types of LonTalk Interface Developer 
 *  generated code and the Pyxos API are derived from the basic data types of 
 *  this file, unless standard C types are used.
 *
 *  (See the ShortStack, FTXL, or Pyxos API documentation for more detailed  
 *  discussion of these data types.)
 *
 *  Below this header, which contains further discussion and background
 *  information about the issues addressed by this file, there are a
 *  set of C-language 'typedef' expressions and preprocessor macro 
 *  definitions. These 'platform-preferences' are grouped together by a 
 *  preprocessor macro that indicates the compiler in use, and possibly 
 *  the target platform in use. 
 *
 *  If you are developing an FTXL Nios II application, your project should 
 *  define GCC_NIOS to select the correct definitions in this file.  Otherwise,
 *  if you are developing a ShortStack or Pyxos application, you must make sure
 *  to examine the base data types defined in this file, and modify them 
 *  as appropriate for your operating system, compiler, and CPU.
 *
 *  If this file does not include definitions that are appropriate for your 
 *  host and development environment, it is recommended that you derive your 
 *  own set of platform properties by copying the example set for the COSMIC C 
 *  Compiler (_COSMIC) and modifying them as needed. 
 *
 *  You must make sure the correct compiler identifier (such as "_COSMIC") 
 *  is defined at compile time; failure to do so will result in an error
 *  during compilation.
 *
 *  -------- Portability Enhancements -----
 *  Many portability enhancements have been made in ShortStack 2.1 and FTXL 1.0 
 *  to eliminate the need for bit fields. Now, bit fields are defined with their 
 *  enclosing bytes, and macros are provided to extract or manipulate the bit 
 *  field information. See the <LON_GET_ATTRIBUTE> and <LON_SET_ATTRIBUTE> 
 *  macros.
 *
 *  As an example, instead of
 *  typedef struct 
 *  {
 *  #ifdef BITF_BIG_ENDIAN
 *      unsigned alpha : 1;
 *      unsigned beta : 3;
 *      unsigned : 0;
 *  #else
 *      unsigned : 4;
 *      unsigned beta : 3;
 *      unsigned alpha : 1;
 *  #endif
 *      �
 *  } Example;
 *
 *  we will say
 *  #define LON_ALPHA_MASK  0x80
 *  #define LON_ALPHA_SHIFT 7
 *  #define LON_ALPHA_FIELD flags_1
 *  #define LON_BETA_MASK   0x70
 *  #define LON_BETA_SHIFT  4
 *  #define LON_BETA_FIELD  flags_1
 *  
 *  typedef struct 
 *  {
 *      LonByte flags_1;  // contains alpha, beta 
 *      �
 *  } Example;
 *
 *  Another change is the replacement of multiple-byte values in a structure 
 *  with multi-byte scalars. This eliminates the concern of endianness among 
 *  bytes. Macros such as <LON_GET_UNSIGNED_WORD> and <LON_SET_UNSIGNED_WORD> 
 *  are provided to convert these scalars to multiple-byte values and back 
 *  again.
 *
 *  For example, instead of 
 *  typedef struct 
 *  {
 *      Word    alpha;
 *      �
 *  } Example;
 *
 *  we will say
 *  typedef struct 
 *  {
 *      LonWord alpha;
 *      �
 *  } Example;
 *
 *  where LonWord is defined as follows:
 *  typedef LON_STRUCT_BEGIN(LonWord) 
 *  {
 *      LonByte  msb;    
 *      LonByte  lsb;    
 *  } LON_STRUCT_END(LonWord);
 *
 ***************************************************************************/
#ifndef _LON_PLATFORM_H
#define _LON_PLATFORM_H

#pragma pack(1)
#pragma align(4)
     
#if defined(_COSMIC)

    /* 
     * Cosmic C programs.  Assume _COSMIC is defined.
     *
     */

    #define COSMIC_HOSTED        /* runs in the COSMIC C environment */

    /*
     * indicate a compiler/platform has been defined
     */
    #if !defined(_LON_PLATFORM_DEFINED)
        #define _LON_PLATFORM_DEFINED
    #else
        #error Multiple platform definitions
    #endif

    #define LON_ENUM_BEGIN(n)   enum
    #define LON_ENUM_END(n)     n
    #define LON_ENUM(n)         n 

    #define LON_STRUCT_BEGIN(n) struct 
    #define LON_STRUCT_END(n)   n
        
    #define LON_STRUCT_NESTED_BEGIN(n)  struct 
    #define LON_STRUCT_NESTED_END(n)    n

    #define LON_UNION_BEGIN(n)  union
    #define LON_UNION_END(n)    n
        
    #define LON_UNION_NESTED_BEGIN(n)   union 
    #define LON_UNION_NESTED_END(n)     n
        
    /*
     * Compiler-dependent types for signed and unsigned 8-bit, 16-bit scalars, 
     * and 32-bit scalars. All LonTalk Interface Developer-Builder generated 
     * types use NEURON C equivalent types which are based on the following 
     * type definitions.
     *
     * To enhance portability between different platforms, no aggregate shall 
     * contain multi-byte scalars, but shall use multiple byte-sized scalars 
     * instead. We will define only the basic type LonByte and the rest 
     * (LonWord, LonDoubleWord) derive from it.
     *
     * Note that "float" type variables are handled through a "float_type" 
     * equivalent structure. See the ShortStack or FTXL documentation 
     * for more details about Builder-generated type definitions including 
     * details about "float" type handling.  
     */
    
    typedef unsigned char   LonUbits8;          /* 8-bits */
    typedef signed   char   LonBits8;           /* 8-bits, signed */
    typedef unsigned short  LonUbits16;         /* 16-bits */
    typedef signed   short  LonBits16;          /* 16-bits, signed */
    typedef unsigned long   LonUbits32;         /* 32-bits */
    typedef signed   long   LonBits32;          /* 32-bits, signed */
    
    typedef LonUbits8       LonByte;            /* 8-bits */
    
    /*
     *  typedef: LonWord
     *  Holds a 16-bit numerical value.
     *
     *  The LonWord structure holds a 16-bit unsigned value in big-endian 
     *  ordering through two separate high-order and low-order bytes. 
     *  Use the <LON_SET_SIGNED_WORD> or <LON_SET_UNSIGNED_WORD> macro to 
     *  obtain the signed or unsigned numerical value in the correct byte 
     *  ordering.
     */
    typedef LON_STRUCT_BEGIN(LonWord) 
    {
        LonByte  msb;    /* high-order byte, the most significant byte, the 0x12 in 0x1234 */
        LonByte  lsb;    /* low-order byte, the least significant byte, the 0x34 in 0x1234 */
    } LON_STRUCT_END(LonWord);
    
    /*
     *  typedef: LonDoubleWord
     *  Holds a 32-bit numerical value.
     *
     *  The LonDoubleWord structure holds a 32-bit unsigned value in big-endian 
     *  ordering through two separate high-order and low-order <LonWord> members.
     *  Use the <LON_SET_SIGNED_DOUBLEWORD> or <LON_SET_UNSIGNED_DOUBLEWORD> 
     *  macro to obtain the signed or unsigned numerical value in the correct 
     *  byte ordering.
     */
    typedef LON_STRUCT_BEGIN(LonDoubleWord) 
    {
        LonWord  msw;    /* high-order word, the most significant word, the 0x1234 in 0x12345678 */
        LonWord  lsw;    /* low-order word, the least significant word, the 0x5678 in 0x12345678 */
    } LON_STRUCT_END(LonDoubleWord);
        
    /*
     * Basic boolean type. You must make sure to have a type 
     * with name 'LonBool' defined, and that type must accept TRUE and FALSE 
     * (defined below).
     */
    typedef int     LonBool;                      
    #ifndef TRUE
        #define TRUE    1
    #endif
    #ifndef FALSE
        #define FALSE   0
    #endif
    
#elif defined(_HITECH)
    /* 
     * Hi-Tech C programs.  Assume _HITECH is defined.
     *
     */
       
    #define HITECH_HOSTED       /* runs in the HITECH C environment */

    /*
     * indicate a compiler/platform has been defined
     */
    #if !defined(_LON_PLATFORM_DEFINED)
        #define _LON_PLATFORM_DEFINED
    #else
        #error Multiple platform definitions
    #endif

    #define LON_ENUM_BEGIN(n)   enum
    #define LON_ENUM_END(n)     n
    #define LON_ENUM(n)         n 

    #define LON_STRUCT_BEGIN(n) struct 
    #define LON_STRUCT_END(n)   n
        
    #define LON_STRUCT_NESTED_BEGIN(n)  struct 
    #define LON_STRUCT_NESTED_END(n)    n

    #define LON_UNION_BEGIN(n)  union
    #define LON_UNION_END(n)    n
        
    #define LON_UNION_NESTED_BEGIN(n)   union 
    #define LON_UNION_NESTED_END(n)     n
        
    /*
     * Compiler-dependent types for signed and unsigned 8-bit, 16-bit scalars, 
     * and 32-bit scalars. All LonTalk Interface Developer-Builder generated 
     * types use NEURON C equivalent types which are based on the following 
     * type definitions.
     *
     * To enhance portability between different platforms, no aggregate shall 
     * contain multi-byte scalars, but shall use multiple byte-sized scalars 
     * instead. We will define only the basic type LonByte and the rest 
     * (LonWord, LonDoubleWord) derive from it.
     *
     * Note that "float" type variables are handled through a "float_type" 
     * equivalent structure. See the ShortStack or FTXL documentation 
     * for more details about Builder-generated type definitions including 
     * details about "float" type handling.  
     */
    
    typedef unsigned char   LonUbits8;          /* 8-bits */
    typedef signed   char   LonBits8;           /* 8-bits, signed */
    typedef unsigned short  LonUbits16;         /* 16-bits */
    typedef signed   short  LonBits16;          /* 16-bits, signed */
    typedef unsigned long   LonUbits32;         /* 32-bits */
    typedef signed   long   LonBits32;          /* 32-bits, signed */
    
    typedef LonUbits8       LonByte;            /* 8-bits */
    
    /*
     *  typedef: LonWord
     *  Holds a 16-bit numerical value.
     *
     *  The LonWord structure holds a 16-bit unsigned value in big-endian 
     *  ordering through two separate high-order and low-order bytes. 
     *  Use the <LON_SET_SIGNED_WORD> or <LON_SET_UNSIGNED_WORD> macro to 
     *  obtain the signed or unsigned numerical value in the correct byte 
     *  ordering.
     */
    typedef LON_STRUCT_BEGIN(LonWord) 
    {
        LonByte  msb;    /* high-order byte, the most significant byte, the 0x12 in 0x1234 */
        LonByte  lsb;    /* low-order byte, the least significant byte, the 0x34 in 0x1234 */
    } LON_STRUCT_END(LonWord);
    
    /*
     *  typedef: LonDoubleWord
     *  Holds a 32-bit numerical value.
     *
     *  The LonDoubleWord structure holds a 32-bit unsigned value in big-endian 
     *  ordering through two separate high-order and low-order <LonWord> members.
     *  Use the <LON_SET_SIGNED_DOUBLEWORD> or <LON_SET_UNSIGNED_DOUBLEWORD> 
     *  macro to obtain the signed or unsigned numerical value in the correct 
     *  byte ordering.
     */
    typedef LON_STRUCT_BEGIN(LonDoubleWord) 
    {
        LonWord  msw;    /* high-order word, the most significant word, the 0x1234 in 0x12345678 */
        LonWord  lsw;    /* low-order word, the least significant word, the 0x5678 in 0x12345678 */
    } LON_STRUCT_END(LonDoubleWord);
        
    /*
     * Basic boolean type. You must make sure to have a type with the name 
     * 'LonBool' defined, and that type must accept TRUE and FALSE (defined 
     * below).
     */
    typedef int     LonBool;                      
    #ifndef TRUE
        #define TRUE    1
    #endif
    #ifndef FALSE
        #define FALSE   0
    #endif

#elif defined(WIN32)
    /* 
     * WIN32 C programs.  Assume WIN32 is defined.
     *
     */
       
    #define WIN32_HOSTED       /* runs in the WIN32 environment */

    /*
     * indicate a compiler/platform has been defined
     */
    #if !defined(_LON_PLATFORM_DEFINED)
        #define _LON_PLATFORM_DEFINED
    #else
        #error Multiple platform definitions
    #endif

    /*
     * Compiler-dependent types for signed and unsigned 8-bit, 16-bit scalars, 
     * and 32-bit scalars. All LonTalk Interface Developer-Builder generated 
     * types use NEURON C equivalent types which are based on the following 
     * type definitions.
     *
     * To enhance portability between different platforms, no aggregate shall 
     * contain multi-byte scalars, but shall use multiple byte-sized scalars 
     * instead. We will define only the basic type LonByte and the rest 
     * (LonWord, LonDoubleWord) derive from it.
     *
     * Note that "float" type variables are handled through a "float_type" 
     * equivalent structure. See the ShortStack or FTXL documentation 
     * for more details about Builder-generated type definitions including 
     * details about "float" type handling.  
     */
    
    typedef unsigned char   LonUbits8;          /* 8-bits */
    typedef signed   char   LonBits8;           /* 8-bits, signed */
    typedef unsigned short  LonUbits16;         /* 16-bits */
    typedef signed   short  LonBits16;          /* 16-bits, signed */
    typedef unsigned long   LonUbits32;         /* 32-bits */
    typedef signed   long   LonBits32;          /* 32-bits, signed */
    
    typedef LonUbits8       LonByte;            /* 8-bits */
    
    #define LON_ENUM_BEGIN(n)   enum 
    #define LON_ENUM_END(n)     n
    #define LON_ENUM(n)         LonByte 

    #define LON_STRUCT_BEGIN(n) struct __declspec(align(1)) 
    #define LON_STRUCT_END(n)   n
        
    #define LON_STRUCT_NESTED_BEGIN(n)  struct 
    #define LON_STRUCT_NESTED_END(n)    n

    #define LON_UNION_BEGIN(n)  union __declspec(align(1))
    #define LON_UNION_END(n)    n
        
    #define LON_UNION_NESTED_BEGIN(n)   union 
    #define LON_UNION_NESTED_END(n)     n
        
    /*
     *  typedef: LonWord
     *  Holds a 16-bit numerical value.
     *
     *  The LonWord structure holds a 16-bit unsigned value in big-endian 
     *  ordering through two separate high-order and low-order bytes. 
     *  Use the <LON_SET_SIGNED_WORD> or <LON_SET_UNSIGNED_WORD> macro to 
     *  obtain the signed or unsigned numerical value in the correct byte 
     *  ordering.
     */
    typedef LON_STRUCT_BEGIN(LonWord) 
    {
        LonByte  msb;    /* high-order byte, the most significant byte, the 0x12 in 0x1234 */
        LonByte  lsb;    /* low-order byte, the least significant byte, the 0x34 in 0x1234 */
    } LON_STRUCT_END(LonWord);
    
    /*
     *  typedef: LonDoubleWord
     *  Holds a 32-bit numerical value.
     *
     *  The LonDoubleWord structure holds a 32-bit unsigned value in big-endian 
     *  ordering through two separate high-order and low-order <LonWord> members.
     *  Use the <LON_SET_SIGNED_DOUBLEWORD> or <LON_SET_UNSIGNED_DOUBLEWORD> 
     *  macro to obtain the signed or unsigned numerical value in the correct 
     *  byte ordering.
     */
    typedef LON_STRUCT_BEGIN(LonDoubleWord) 
    {
        LonWord  msw;    /* high-order word, the most significant word, the 0x1234 in 0x12345678 */
        LonWord  lsw;    /* low-order word, the least significant word, the 0x5678 in 0x12345678 */
    } LON_STRUCT_END(LonDoubleWord);
        
    /*
     * Basic boolean type. You must make sure to have a type with the name 
     * 'LonBool' defined, and that type must accept TRUE and FALSE (defined 
     * below).
     */
    typedef int     LonBool;                      
    #ifndef TRUE
        #define TRUE    1
    #endif
    #ifndef FALSE
        #define FALSE   0
    #endif


#elif defined(ARM7)
    /* 
     * ARM7 C programs.  Assume ARM7 is defined.
     *
     */
       
    #define ARM7_HOSTED       /* runs in the ARM7 environment */

    /*
     * indicate a compiler/platform has been defined
     */
    #if !defined(_LON_PLATFORM_DEFINED)
        #define _LON_PLATFORM_DEFINED
    #else
        #error Multiple platform definitions
    #endif
        
    /*
     * Definition for ARM7 specific pragmas, definitions, and so on. 
     * For example, packing directives to align objects on byte boundary.
     */
    #define INCLUDE_LON_BEGIN_END

    /*
     * Compiler-dependent types for signed and unsigned 8-bit, 16-bit scalars, 
     * and 32-bit scalars. All LonTalk Interface Developer-Builder generated 
     * types use NEURON C equivalent types which are based on the following 
     * type definitions.
     *
     * To enhance portability between different platforms, no aggregate shall 
     * contain multi-byte scalars, but shall use multiple byte-sized scalars 
     * instead. We will define only the basic type LonByte and the rest 
     * (LonWord, LonDoubleWord) derive from it.
     *
     * Note that "float" type variables are handled through a "float_type" 
     * equivalent structure. See the ShortStack or FTXL documentation 
     * for more details about Builder-generated type definitions including 
     * details about "float" type handling.  
     */
    
    typedef unsigned char   LonUbits8;          /* 8-bits */
    typedef signed   char   LonBits8;           /* 8-bits, signed */
    typedef unsigned short  LonUbits16;         /* 16-bits */
    typedef signed   short  LonBits16;          /* 16-bits, signed */
    typedef unsigned long   LonUbits32;         /* 32-bits */
    typedef signed   long   LonBits32;          /* 32-bits, signed */
    
    typedef LonUbits8       LonByte;            /* 8-bits */
    
    #define LON_ENUM_BEGIN(n)   enum
    #define LON_ENUM_END(n)     n
    #define LON_ENUM(n)         n 

    #define LON_STRUCT_BEGIN(n) struct 
    #define LON_STRUCT_END(n)   n
        
    #define LON_STRUCT_NESTED_BEGIN(n)  struct 
    #define LON_STRUCT_NESTED_END(n)    n

    #define LON_UNION_BEGIN(n)  union
    #define LON_UNION_END(n)    n
        
    #define LON_UNION_NESTED_BEGIN(n)   union 
    #define LON_UNION_NESTED_END(n)     n
        
    /*
     *  typedef: LonWord
     *  Holds a 16-bit numerical value.
     *
     *  The LonWord structure holds a 16-bit unsigned value in big-endian 
     *  ordering through two separate high-order and low-order bytes. 
     *  Use the <LON_SET_SIGNED_WORD> or <LON_SET_UNSIGNED_WORD> macro to 
     *  obtain the signed or unsigned numerical value in the correct byte 
     *  ordering.
     */
    typedef LON_STRUCT_BEGIN(LonWord) 
    {
        LonByte  msb;    /* high-order byte, the most significant byte, the 0x12 in 0x1234 */
        LonByte  lsb;    /* low-order byte, the least significant byte, the 0x34 in 0x1234 */
    } LON_STRUCT_END(LonWord);
    
    /*
     *  typedef: LonDoubleWord
     *  Holds a 32-bit numerical value.
     *
     *  The LonDoubleWord structure holds a 32-bit unsigned value in big-endian 
     *  ordering through two separate high-order and low-order <LonWord> members.
     *  Use the <LON_SET_SIGNED_DOUBLEWORD> or <LON_SET_UNSIGNED_DOUBLEWORD> 
     *  macro to obtain the signed or unsigned numerical value in the correct 
     *  byte ordering.
     */
    typedef LON_STRUCT_BEGIN(LonDoubleWord) 
    {
        LonWord  msw;    /* high-order word, the most significant word, the 0x1234 in 0x12345678 */
        LonWord  lsw;    /* low-order word, the least significant word, the 0x5678 in 0x12345678 */
    } LON_STRUCT_END(LonDoubleWord);
        
    /*
     * Basic boolean type. You must make sure to have a type with the name 
     * 'LonBool' defined, and that type must accept TRUE and FALSE (defined 
     * below).
     */
    typedef int     LonBool;                      
    #ifndef TRUE
        #define TRUE    1
    #endif
    #ifndef FALSE
        #define FALSE   0
    #endif
    
#elif defined(STM32F429xx) || defined(STM32F103xB) || defined(STM32G071xx)
    /*
     * STM32
     *
     */

    #define STM32_HOSTED

    /*
     * indicate a compiler/platform has been defined
     */
    #if !defined(_LON_PLATFORM_DEFINED)
        #define _LON_PLATFORM_DEFINED
    #else
        #error Multiple platform definitions
    #endif

    /*
     * Definition for ARM7 specific pragmas, definitions, and so on.
     * For example, packing directives to align objects on byte boundary.
     */
    #define INCLUDE_LON_BEGIN_END

    /*
     * Compiler-dependent types for signed and unsigned 8-bit, 16-bit scalars,
     * and 32-bit scalars. All LonTalk Interface Developer-Builder generated
     * types use NEURON C equivalent types which are based on the following
     * type definitions.
     *
     * To enhance portability between different platforms, no aggregate shall
     * contain multi-byte scalars, but shall use multiple byte-sized scalars
     * instead. We will define only the basic type LonByte and the rest
     * (LonWord, LonDoubleWord) derive from it.
     *
     * Note that "float" type variables are handled through a "float_type"
     * equivalent structure. See the ShortStack or FTXL documentation
     * for more details about Builder-generated type definitions including
     * details about "float" type handling.
     */

    typedef unsigned char   LonUbits8;          /* 8-bits */
    typedef signed   char   LonBits8;           /* 8-bits, signed */
    typedef unsigned short  LonUbits16;         /* 16-bits */
    typedef signed   short  LonBits16;          /* 16-bits, signed */
    typedef unsigned long   LonUbits32;         /* 32-bits */
    typedef signed   long   LonBits32;          /* 32-bits, signed */

    typedef LonUbits8       LonByte;            /* 8-bits */

    #define LON_ENUM_BEGIN(n)   enum
    #define LON_ENUM_END(n)     n
    #define LON_ENUM(n)         n

    #define LON_STRUCT_BEGIN(n) struct
    #define LON_STRUCT_END(n)   n

    #define LON_STRUCT_NESTED_BEGIN(n)  struct
    #define LON_STRUCT_NESTED_END(n)    n

    #define LON_UNION_BEGIN(n)  union
    #define LON_UNION_END(n)    n

    #define LON_UNION_NESTED_BEGIN(n)   union
    #define LON_UNION_NESTED_END(n)     n

    /*
     *  typedef: LonWord
     *  Holds a 16-bit numerical value.
     *
     *  The LonWord structure holds a 16-bit unsigned value in big-endian
     *  ordering through two separate high-order and low-order bytes.
     *  Use the <LON_SET_SIGNED_WORD> or <LON_SET_UNSIGNED_WORD> macro to
     *  obtain the signed or unsigned numerical value in the correct byte
     *  ordering.
     */
    typedef LON_STRUCT_BEGIN(LonWord)
    {
        LonByte  msb;    /* high-order byte, the most significant byte, the 0x12 in 0x1234 */
        LonByte  lsb;    /* low-order byte, the least significant byte, the 0x34 in 0x1234 */
    } LON_STRUCT_END(LonWord);

    /*
     *  typedef: LonDoubleWord
     *  Holds a 32-bit numerical value.
     *
     *  The LonDoubleWord structure holds a 32-bit unsigned value in big-endian
     *  ordering through two separate high-order and low-order <LonWord> members.
     *  Use the <LON_SET_SIGNED_DOUBLEWORD> or <LON_SET_UNSIGNED_DOUBLEWORD>
     *  macro to obtain the signed or unsigned numerical value in the correct
     *  byte ordering.
     */
    typedef LON_STRUCT_BEGIN(LonDoubleWord)
    {
        LonWord  msw;    /* high-order word, the most significant word, the 0x1234 in 0x12345678 */
        LonWord  lsw;    /* low-order word, the least significant word, the 0x5678 in 0x12345678 */
    } LON_STRUCT_END(LonDoubleWord);

    /*
     * Basic boolean type. You must make sure to have a type with the name
     * 'LonBool' defined, and that type must accept TRUE and FALSE (defined
     * below).
     */
    typedef int     LonBool;
    #ifndef TRUE
        #define TRUE    1
    #endif
    #ifndef FALSE
        #define FALSE   0
    #endif

#elif defined(AVR_TINY13)
    /* 
     * AVR Tiny-13 & C programs.  Assume AVR_TINY13 is defined.
     *
     */
       
    #define AVR_TINY13_HOSTED       /* runs in the AVR_TINY13 environment */

    /*
     * indicate a compiler/platform has been defined
     */
    #if !defined(_LON_PLATFORM_DEFINED)
        #define _LON_PLATFORM_DEFINED
    #else
        #error Multiple platform definitions
    #endif

    /*
     * Compiler-dependent types for signed and unsigned 8-bit, 16-bit scalars, 
     * and 32-bit scalars. All LonTalk Interface Developer-Builder generated 
     * types use NEURON C equivalent types which are based on the following 
     * type definitions.
     *
     * To enhance portability between different platforms, no aggregate shall 
     * contain multi-byte scalars, but shall use multiple byte-sized scalars 
     * instead. We will define only the basic type LonByte and the rest 
     * (LonWord, LonDoubleWord) derive from it.
     *
     * Note that "float" type variables are handled through a "float_type" 
     * equivalent structure. See the ShortStack or FTXL documentation 
     * for more details about Builder-generated type definitions including 
     * details about "float" type handling.  
     */
    
    typedef unsigned char   LonUbits8;          /* 8-bits */
    typedef signed   char   LonBits8;           /* 8-bits, signed */
    typedef unsigned short  LonUbits16;         /* 16-bits */
    typedef signed   short  LonBits16;          /* 16-bits, signed */
    typedef unsigned long   LonUbits32;         /* 32-bits */
    typedef signed   long   LonBits32;          /* 32-bits, signed */
    
    typedef LonUbits8       LonByte;            /* 8-bits */
    
    #define LON_ENUM_BEGIN(n)   enum
    #define LON_ENUM_END(n)     n
    #define LON_ENUM(n)         n 

    #define LON_STRUCT_BEGIN(n) struct 
    #define LON_STRUCT_END(n)   n
        
    #define LON_STRUCT_NESTED_BEGIN(n)  struct 
    #define LON_STRUCT_NESTED_END(n)    n

    #define LON_UNION_BEGIN(n)  union
    #define LON_UNION_END(n)    n
        
    #define LON_UNION_NESTED_BEGIN(n)   union 
    #define LON_UNION_NESTED_END(n)     n
        
    /*
     *  typedef: LonWord
     *  Holds a 16-bit numerical value.
     *
     *  The LonWord structure holds a 16-bit unsigned value in big-endian 
     *  ordering through two separate high-order and low-order bytes. 
     *  Use the <LON_SET_SIGNED_WORD> or <LON_SET_UNSIGNED_WORD> macro to 
     *  obtain the signed or unsigned numerical value in the correct byte 
     *  ordering.
     */
    typedef LON_STRUCT_BEGIN(LonWord) 
    {
        LonByte  msb;    /* high-order byte, the most significant byte, the 0x12 in 0x1234 */
        LonByte  lsb;    /* low-order byte, the least significant byte, the 0x34 in 0x1234 */
    } LON_STRUCT_END(LonWord);
    
    /*
     *  typedef: LonDoubleWord
     *  Holds a 32-bit numerical value.
     *
     *  The LonDoubleWord structure holds a 32-bit unsigned value in big-endian 
     *  ordering through two separate high-order and low-order <LonWord> members.
     *  Use the <LON_SET_SIGNED_DOUBLEWORD> or <LON_SET_UNSIGNED_DOUBLEWORD> 
     *  macro to obtain the signed or unsigned numerical value in the correct 
     *  byte ordering.
     */
    typedef LON_STRUCT_BEGIN(LonDoubleWord) 
    {
        LonWord  msw;    /* high-order word, the most significant word, the 0x1234 in 0x12345678 */
        LonWord  lsw;    /* low-order word, the least significant word, the 0x5678 in 0x12345678 */
    } LON_STRUCT_END(LonDoubleWord);
        
    /*
     * Basic boolean type. You must make sure to have a type with the name 
     * 'LonBool' defined, and that type must accept TRUE and FALSE (defined 
     * below).
     */
    typedef int     LonBool;                      
    #ifndef TRUE
        #define TRUE    1
    #endif
    #ifndef FALSE
        #define FALSE   0
    #endif

#elif defined(GCC_NIOS)
    /* 
     * GCC_NIOS C programs.  Assume GCC_NIOS is defined.
     *
     * This is the correct choice for the Nios II platform.
     * This section also assumes that C programs run on a LITTLE_ENDIAN machine,
     * which is the typical case.  If you are running a C program on a BIG_ENDIAN
     * processor, be sure to change LITTLE_ENDIAN to BIG_ENDIAN 
     *
     */

    #define GCC_NIOS_HOSTED       /* runs in the GCC_NIOS C environment */

    /*
     * indicate a compiler/platform has been defined
     */
    #if !defined(_LON_PLATFORM_DEFINED)
        #define _LON_PLATFORM_DEFINED
    #else
        #error Multiple platform definitions
    #endif
        
    /*
     * Compiler-dependent types for signed and unsigned 8-bit, 16-bit scalars, 
     * and 32-bit scalars. All LonTalk Interface Developer-Builder generated 
     * types use NEURON C equivalent types which are based on the following 
     * type definitions.
     *
     * To enhance portability between different platforms, no aggregate shall 
     * contain multi-byte scalars, but shall use multiple byte-sized scalars 
     * instead. We will define only the basic type LonByte and the rest 
     * (LonWord, LonDoubleWord) derive from it.
     *
     * Note that "float" type variables are handled through a "float_type" 
     * equivalent structure. See the ShortStack or FTXL documentation 
     * for more details about Builder-generated type definitions including 
     * details about "float" type handling.  
     */
    
    typedef unsigned char   LonUbits8;          /* 8-bits */
    typedef signed   char   LonBits8;           /* 8-bits, signed */
    typedef unsigned short  LonUbits16;         /* 16-bits */
    typedef signed   short  LonBits16;          /* 16-bits, signed */
    typedef unsigned long   LonUbits32;         /* 32-bits */
    typedef signed   long   LonBits32;          /* 32-bits, signed */
    
    typedef LonUbits8       LonByte;            /* 8-bits */
    
    #define LON_ENUM_BEGIN(n)   enum
    #define LON_ENUM_END(n)     n
    #define LON_ENUM(n)         LonByte

    #define LON_STRUCT_BEGIN(n) struct 
    #define LON_STRUCT_END(n)   __attribute((__packed__)) n
        
    #define LON_STRUCT_NESTED_BEGIN(n)  struct 
    #define LON_STRUCT_NESTED_END(n)    __attribute((__packed__)) n

    #define LON_UNION_BEGIN(n)  union
    #define LON_UNION_END(n)    __attribute((__packed__)) n
        
    #define LON_UNION_NESTED_BEGIN(n)   union 
    #define LON_UNION_NESTED_END(n)     __attribute((__packed__)) n
        
    /*
     *  typedef: LonWord
     *  Holds a 16-bit numerical value.
     *
     *  The LonWord structure holds a 16-bit unsigned value in big-endian 
     *  ordering through two separate high-order and low-order bytes. 
     *  Use the <LON_SET_SIGNED_WORD> or <LON_SET_UNSIGNED_WORD> macro to 
     *  obtain the signed or unsigned numerical value in the correct byte 
     *  ordering.
     */
    typedef LON_STRUCT_BEGIN(LonWord) 
    {
        LonByte  msb;    /* high-order byte, the most significant byte, the 0x12 in 0x1234 */
        LonByte  lsb;    /* low-order byte, the least significant byte, the 0x34 in 0x1234 */
    } LON_STRUCT_END(LonWord);
    
    /*
     *  typedef: LonDoubleWord
     *  Holds a 32-bit numerical value.
     *
     *  The LonDoubleWord structure holds a 32-bit unsigned value in big-endian 
     *  ordering through two separate high-order and low-order <LonWord> members.
     *  Use the <LON_SET_SIGNED_DOUBLEWORD> or <LON_SET_UNSIGNED_DOUBLEWORD> 
     *  macro to obtain the signed or unsigned numerical value in the correct 
     *  byte ordering.
     */
    typedef LON_STRUCT_BEGIN(LonDoubleWord) 
    {
        LonWord  msw;    /* high-order word, the most significant word, the 0x1234 in 0x12345678 */
        LonWord  lsw;    /* low-order word, the least significant word, the 0x5678 in 0x12345678 */
    } LON_STRUCT_END(LonDoubleWord);
        
    /*
     * Basic boolean type. You must make sure to have a type with the name 
     * 'LonBool' defined, and that type must accept TRUE and FALSE (defined 
     * below).
     */
    typedef int     LonBool;                      
    #ifndef TRUE
        #define TRUE    1
    #endif
    #ifndef FALSE
        #define FALSE   0
    #endif

#else
    /*
     * Add compiler and platform-specifics here.
     */
    #error No compiler and platform-specifics defined in platform.h             
                
#endif 

/*
 *  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *  No change should be required to the following section, because it 
 *  does not contain compiler or target platform dependencies.
 *  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
 */
 
/* some error checking first: */
#if !defined(_LON_PLATFORM_DEFINED)
    #error  Platform definition is missing
#endif
 

/*
 *  **************************************************************************
 *  NEURON C type equivalents:
 *  These types are used by LonTalk Interface Developer-Builder generated type 
 *  definitions. Each NEURON C equivalent type is a host-platform dependent 
 *  type definition that is equivalent to the respective NEURON C type. 
 *
 *  For your information, a NEURON C "int" and "short" are both 8-bit scalars,
 *  a NEURON C "long" is a 16 bit variable. See the ShortStack or FTXL 
 *  documentation for more details about LonTalk Interface Developer-Builder 
 *  generated types.
 *  ************************************************************************** 
 */
 
typedef LonUbits8   ncuChar;    /* equivalent of NEURON C "unsigned char"   */
typedef LonUbits8   ncuShort;   /* equivalent of NEURON C "unsigned short"  */
typedef LonUbits8   ncuInt;     /* equivalent of NEURON C "unsigned int"    */
typedef LonWord     ncuLong;    /* equivalent of NEURON C "unsigned long"   */
typedef LonBits8    ncsChar;    /* equivalent of NEURON C "signed char"     */
typedef LonBits8    ncsShort;   /* equivalent of NEURON C "signed short"    */
typedef LonBits8    ncsInt;     /* equivalent of NEURON C "signed int"      */
typedef LonWord     ncsLong;    /* equivalent of NEURON C "signed long"     */

#endif  /* _LON_PLATFORM_H */
