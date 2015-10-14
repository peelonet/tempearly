#ifndef TEMPEARLY_MACROS_H_GUARD
#define TEMPEARLY_MACROS_H_GUARD

#define TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName &) = delete; \
    void operator=(const TypeName &) = delete

#define TEMPEARLY_DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
    TypeName() = delete; \
    TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(TypeName)

#define TEMPEARLY_NATIVE_METHOD(MethodName) \
    static void MethodName(const Handle<Interpreter>& interpreter, \
                           const Handle<Frame>& frame, \
                           const Vector<Value>& args)

#endif /* !TEMPEARLY_MACROS_H_GUARD */
