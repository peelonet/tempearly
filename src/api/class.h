#ifndef TEMPEARLY_API_CLASS_H_GUARD
#define TEMPEARLY_API_CLASS_H_GUARD

#include "object.h"

namespace tempearly
{
    class Class : public Object
    {
    public:
        typedef Handle<Object> (*Allocator)(
            const Handle<Interpreter>&,
            const Handle<Class>&
        );
        typedef void (*MethodCallback)(
            const Handle<Interpreter>&,
            const Handle<Frame>&,
            const Vector<Handle<Object>>&
        );

        /**
         * Allocator which allocates nothing.
         */
        static const Allocator kNoAlloc;

        explicit Class(const Handle<Class>& base);

        virtual ~Class();

        Handle<Class> GetClass(const Handle<Interpreter>& interpreter) const;

        /**
         * Returns name of the class.
         */
        String GetName() const;

        inline Handle<Class> GetBase() const
        {
            return m_base;
        }

        /**
         * Returns true if type is subclass of given type, either direct or
         * virtual. Type is considered to be subclass of itself.
         */
        bool IsSubclassOf(const Handle<Class>& that) const;

        inline Allocator GetAllocator() const
        {
            return m_allocator;
        }

        inline void SetAllocator(Allocator allocator)
        {
            m_allocator = allocator;
        }

        Dictionary<Handle<Object>> GetOwnAttributes() const;

        bool GetOwnAttribute(const String& id, Handle<Object>& slot) const;

        bool SetOwnAttribute(const String& id, const Handle<Object>& value);

        void AddMethod(const Handle<Interpreter>& interpreter,
                       const String& name,
                       int arity,
                       MethodCallback callback);

        void AddStaticMethod(const Handle<Interpreter>& interpreter,
                             const String& name,
                             int arity,
                             MethodCallback callback);

        /**
         * Adds an alias method to this class. Alias methods are methods which
         * sipmly redirect the method call to another method.
         *
         * \param interpreter  Script interpreter
         * \param alias_name   Name of the alias method
         * \param aliased_name Name of the aliased method, e.g. the method
         *                     which the alias method calls
         */
        void AddMethodAlias(const Handle<Interpreter>& interpreter,
                            const String& alias_name,
                            const String& aliased_name);

        void Mark();

        bool IsClass() const
        {
            return true;
        }

    private:
        /** Superclass. */
        Class* m_base;
        /** Allocator callback. */
        Allocator m_allocator;
        /** Contains attributes for the class. */
        Dictionary<Object*>* m_attributes;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Class);
    };
}

#endif /* !TEMPEARLY_API_CLASS_H_GUARD */
