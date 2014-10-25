#ifndef TEMPEARLY_DEFINES_H_GUARD
#define TEMPEARLY_DEFINES_H_GUARD

#if defined(TEMPEARLY_HAVE_CSTDINT)
# include <cstdint>
#elif defined(TEMPEARLY_HAVE_STDINT_H)
# include <stdint.h>
#endif

namespace tempearly
{
#if defined(TEMPEARLY_HAVE_CSTDINT)
    typedef std::int8_t i8;
    typedef std::uint8_t u8;
    typedef std::int16_t i6;
    typedef std::uint16_t u16;
    typedef std::int32_t i32;
    typedef std::uint32_t u32;
    typedef std::int64_t i64;
    typedef std::uint64_t u64;
#elif defined(TEMPEARLY_HAVE_STDINT_H)
    typedef ::int8_t i8;
    typedef ::uint8_t u8;
    typedef ::int16_t i16;
    typedef ::uint16_t u16;
    typedef ::int32_t i32;
    typedef ::uint32_t u32;
    typedef ::int64_t i64;
    typedef ::uint64_t u64;
#else
    typedef signed char i8;
    typedef unsigned char u8;
    typedef signed short i16;
    typedef unsigned short u16;
    typedef signed int i32;
    typedef unsigned int u32;
# if defined(_MSC_VER)
    typedef signed __int64 i64;
    typedef unsigned __int64 u64;
# else
    typedef signed long long i64;
    typedef unsigned long long u64;
# endif
#endif

    typedef u8 byte;
    typedef u32 rune;

    class ByteString;
    class Class;
    class CoreObject;
    class CountedObject;
    class Date;
    class DateTime;
    class ExceptionObject;
    class Filename;
    class FileObject;
    class FunctionObject;
    class Interpreter;
    class IteratorObject;
    class ListObject;
    class MapObject;
    class Node;
    class Object;
    class Parameter;
    class Parser;
    class Random;
    class RangeObject;
    class Request;
    class Response;
    class Result;
    class Scope;
    class Script;
    class Socket;
    class String;
    class StringBuilder;
    class Time;
    class Token;
    class TypeHint;
    class Value;
}

#endif /* !TEMPEARLY_DEFINES_H_GUARD */
