#ifndef TEMPEARLY_CORE_RANDOM_H_GUARD
#define TEMPEARLY_CORE_RANDOM_H_GUARD

#include "tempearly.h"

namespace tempearly
{
    /**
     * Implementation of random number generator (mersenne twister).
     */
    class Random
    {
    public:
        static bool NextBool();

        static double NextDouble();

        static u8 NextU8();

        static i8 NextI8();

        static u64 NextU64();

        static i64 NextI64();

    private:
        TEMPEARLY_DISALLOW_IMPLICIT_CONSTRUCTORS(Random);
    };
}

#endif /* !TEMPEARLY_CORE_RANDOM_H_GUARD */
