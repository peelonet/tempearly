#ifndef TEMPEARLY_SCOPE_H_GUARD
#define TEMPEARLY_SCOPE_H_GUARD

#include "value.h"

namespace tempearly
{
    /**
     * Represents local variable scope.
     */
    class Scope : public CountedObject
    {
    public:
        typedef Dictionary<Value> VariableMap;

        explicit Scope(const Handle<Scope>& previous,
                       const Handle<Scope>& parent);

        virtual ~Scope();

        /**
         * Returns previous scope in scope chain or invalid handle if this
         * scope if the first scope in the chain.
         */
        inline Handle<Scope> GetPrevious() const
        {
            return m_previous;
        }

        /**
         * Returns parent scope in scope chain or invalid handle if this scope
         * is the topmost scope in the chain.
         */
        inline Handle<Scope> GetParent() const
        {
            return m_parent;
        }

        /**
         * Returns true if scope has variable with given identifier.
         */
        bool HasVariable(const String& id) const;

        /**
         * Retrieves an variable from this scope and this scope only. Parent
         * scopes are not included in the search.
         *
         * \param id    Name of the variable to look for
         * \param value Where value of the variable is assigned to
         * \return      A boolean flag indicating whether variable with given
         *              name was found in the scope
         */
        bool GetVariable(const String& id, Value& value) const;

        /**
         * Inserts an variable into this scope. Existing variables with same
         * identifier are overridden.
         *
         * \param id    Name of the variable
         * \param value Value of the variable
         */
        void SetVariable(const String& id, const Value& value);

        /**
         * Constructs an hash map from contents of the variable scope.
         */
        Handle<MapObject> ToMap(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        /** Pointer to previous scope in scope chain. */
        Scope* m_previous;
        /** Pointer to parent scope in scope chain. */
        Scope* m_parent;
        /** Contains variables declared in this scope. */
        VariableMap* m_variables;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Scope);
    };
}

#endif /* !TEMPEARLY_SCOPE_H_GUARD */
