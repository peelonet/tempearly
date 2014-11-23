#ifndef TEMPEARLY_SCRIPT_PARAMETER_H_GUARD
#define TEMPEARLY_SCRIPT_PARAMETER_H_GUARD

#include "script/node.h"
#include "script/typehint.h"

namespace tempearly
{
    class Parameter : public CountedObject
    {
    public:
        explicit Parameter(const String& name,
                           const Handle<TypeHint>& type = Handle<TypeHint>(),
                           const Handle<Node>& default_value = Handle<Node>(),
                           bool rest = false);

        static bool Apply(const Handle<Interpreter>& interpreter,
                          const Vector<Handle<Parameter> >& parameters,
                          const Vector<Value>& arguments);

        /**
         * Returns name of the parameter.
         */
        inline const String& GetName() const
        {
            return m_name;
        }

        /**
         * Returns optional type of the parameter.
         */
        inline Handle<TypeHint> GetType() const
        {
            return m_type;
        }
        
        /**
         * Returns optional default value of the parameter.
         */
        inline Handle<Node> GetDefaultValue() const
        {
            return m_default_value;
        }

        /**
         * Returns true if parameters takes in variable number of arguments.
         */
        inline bool IsRest() const
        {
            return m_rest;
        }

        void Mark();

    private:
        const String m_name;
        TypeHint* m_type;
        Node* m_default_value;
        const bool m_rest;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Parameter);
    };
}

#endif /* !TEMPEARLY_SCRIPT_PARAMETER_H_GUARD */
