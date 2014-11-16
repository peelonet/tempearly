#ifndef TEMPEARLY_API_CLASS_H_GUARD
#define TEMPEARLY_API_CLASS_H_GUARD

#include "coreobject.h"
#include "core/vector.h"

namespace tempearly
{
    class Class : public CoreObject
    {
    public:
        typedef Handle<CoreObject> (*Allocator)(const Handle<Interpreter>&,
                                                const Handle<Class>&);
        typedef Value (*MethodCallback)(const Handle<Interpreter>&, const Vector<Value>&);

        /**
         * Allocator which allocates nothing.
         */
        static const Allocator kNoAlloc;

        /**
         * Constructs new class. Manual construction of classes is discouraged,
         * since it can fuck up the type system. Newly constructed class will
         * have no super classes.
         */
        explicit Class();

        ~Class();

        Handle<Class> GetClass(const Handle<Interpreter>& interpreter) const;

        /**
         * Returns name of the class or "<anonymous class>" if the class has no
         * name.
         */
        String GetName() const;

        /**
         * Returns true if type is subclass of given type, either direct or
         * virtual. Type is considered to be subclass of itself.
         */
        bool IsSubclassOf(const Handle<Class>& that) const;

        /**
         * Marks class as subclass of given class.
         *
         * \return A boolean flag indicating whether this class can inherit
         *         from class given as argument
         */
        bool AddBase(const Handle<Class>& base);

        /**
         * Returns the allocator used by the class.
         */
        inline Allocator GetAllocator() const
        {
            return m_allocator;
        }

        /**
         * Sets the allocator used by the class.
         */
        inline void SetAllocator(Allocator allocator)
        {
            m_allocator = allocator;
        }

        Dictionary<Value> GetAllAttributes() const;

        bool HasAttribute(const String& id) const;

        bool GetAttribute(const String& id, Value& value) const;

        void SetAttribute(const String& id, const Value& value);

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
        /** List of super classes. */
        Vector<Class*>* m_bases;
        /** Allocator callback. */
        Allocator m_allocator;
        /** Contains attributes for the class. */
        AttributeMap* m_attributes;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Class);
    };
}

#endif /* !TEMPEARLY_API_CLASS_H_GUARD */
