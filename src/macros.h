#ifndef TEMPEARLY_MACROS_H_GUARD
#define TEMPEARLY_MACROS_H_GUARD

#if __cplusplus > 199711L
# define TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&) = delete; \
    void operator=(const TypeName&) = delete
# define TEMPEARLY_DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
    TypeName() = delete; \
    TEMPERALY_DISALLOW_COPY_AND_ASSIGN(TypeName)
#else
# define TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&); \
    void operator=(const TypeName&)
# define TEMPEARLY_DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
    TypeName(); \
    TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(TypeName)
#endif

#endif /* !TEMPEARLY_MACROS_H_GUARD */
