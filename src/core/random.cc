#include "core/random.h"

#if defined(_WIN32)
# include <windows.h>
#else
# include <ctime>
#endif

#if defined(TEMPEARLY_HAVE_CFLOAT)
# include <cfloat>
#elif defined(TEMPEARLY_HAVE_FLOAT_H)
# include <float.h>
#endif
#if defined(TEMPEARLY_HAVE_CLIMITS)
# include <climits>
#elif defined(TEMPEARLY_HAVE_LIMITS_H)
# include <limits.h>
#endif

#if !defined(DBL_EPSILON)
# define DBL_EPSILON 2.2204460492503131e-16
#endif
#if !defined(INT64_MAX)
# if defined(LLONG_MAX)
#  define INT64_MAX LLONG_MAX
# else
#  define INT64_MAX 0x7fffffffffffffff
# endif
#endif

namespace tempearly
{
    static const int n = 624;
    static const int m = 397;

    static bool initialized = false;
    static u64 state[n] = {0x0};
    static std::size_t offset = 0;

    static void seed(u64);
    static void gen_state();

    bool Random::NextBool()
    {
        return NextU64() % 2 != 0;
    }

    double Random::NextDouble()
    {
        double r = 0.0;
        double s = 1.0;

        do
        {
            s /= INT64_MAX + 1.0;
            r += static_cast<double>(NextU64()) * s;
        }
        while (s > DBL_EPSILON);

        return r;
    }

    u8 Random::NextU8()
    {
        u8 x;

        if (!initialized)
        {
#if defined(_WIN32)
            seed((::GetTickCount() >> 8) ^ ::GetTickCount());
#else
            seed((std::time(0) >> 8) ^ std::time(0));
#endif
        }
        if (offset == n)
        {
            gen_state();
        }
        x = state[offset++];
        x ^= (x >> 11);
        x ^= (x << 7) & 0x9D2C5680ULL;
        x ^= (x << 15) & 0xEFC60000ULL;

        return x ^ (x >> 18);
    }

    i8 Random::NextI8()
    {
        const bool sign = NextBool();
        const u8 value = NextU8();

        return sign ? value : -value;
    }

    u64 Random::NextU64()
    {
        u64 x;

        if (!initialized)
        {
#if defined(_WIN32)
            seed((::GetTickCount() >> 8) ^ ::GetTickCount());
#else
            seed((std::time(0) >> 8) ^ std::time(0));
#endif
        }
        if (offset == n)
        {
            gen_state();
        }
        x = state[offset++];
        x ^= (x >> 11);
        x ^= (x << 7) & 0x9D2C5680ULL;
        x ^= (x << 15) & 0xEFC60000ULL;

        return x ^ (x >> 18);
    }

    i64 Random::NextI64()
    {
        const bool sign = NextBool();
        const u64 value = NextU64();

        return sign ? value : -value;
    }

    static void seed(u64 s)
    {
        state[0] = s & 0xFFFFFFFFULL;
        for (int i = 1; i < n; ++i)
        {
            state[i] = 1812433253ULL * (state[i - 1]
                    ^ (state[i - 1] >> 30)) + i;
            state[i] &= 0xFFFFFFFFULL;
        }
        offset = n;
        initialized = true;
    }

    static inline u64 twiddle(u64 u, u64 v)
    {
        return (((u & 0x80000000ULL) | (v & 0x7FFFFFFFULL)) >> 1)
            ^ ((v & 1ULL) * 0x9908B0DFULL);
    }

    static void gen_state()
    {
        for (int i = 0; i < n - m; ++i)
        {
            state[i] = state[i + m] ^ twiddle(state[i], state[i + 1]);
        }
        for (int i = n - m; i < n - 1; ++i)
        {
            state[i] = state[i + m - n] ^ twiddle(state[i], state[i + 1]);
        }
        state[n - 1] = state[m - 1] ^ twiddle(state[n - 1], state[0]);
        offset = 0;
    }
}
