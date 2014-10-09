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
        bool NextBool();

        double NextDouble();

        u64 NextU64();

        i64 NextI64();

    private:
        TEMPEARLY_DISALLOW_IMPLICIT_CONSTRUCTORS(Random);
    };
}

#endif /* !TEMPEARLY_CORE_RANDOM_H_GUARD */
