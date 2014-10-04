#ifndef TEMPEARLY_SCOPE_H_GUARD
#define TEMPEARLY_SCOPE_H_GUARD

#include "dictionary.h"
#include "value.h"

namespace tempearly
{
    class Scope : public CountedObject
    {
    public:
        typedef Dictionary<Value> VariableMap;

        explicit Scope(const Handle<Scope>& previous,
                       const Handle<Scope>& parent);

        virtual ~Scope();

        inline Handle<Scope> GetPrevious() const
        {
            return m_previous;
        }

        inline Handle<Scope> GetParent() const
        {
            return m_parent;
        }

        bool HasVariable(const String& id) const;

        bool GetVariable(const String& id, Value& value) const;

        void SetVariable(const String& id, const Value& value);

        void Mark();

    private:
        Scope* m_previous;
        Scope* m_parent;
        VariableMap* m_variables;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Scope);
    };
}

#endif /* !TEMPEARLY_SCOPE_H_GUARD */
