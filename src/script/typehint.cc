#include "interpreter.h"
#include "node.h"
#include "typehint.h"
#include "api/class.h"

namespace tempearly
{
    TypeHint::TypeHint() {}

    namespace
    {
        class ExpressionTypeHint : public TypeHint
        {
        public:
            explicit ExpressionTypeHint(const Handle<Node>& node)
                : m_node(node) {}

            bool Accepts(const Handle<Interpreter>& interpreter, const Value& value, bool& slot) const
            {
                Value cls;

                if (!m_node->Evaluate(interpreter, cls))
                {
                    return false;
                }
                else if (!cls.IsClass())
                {
                    interpreter->Throw(interpreter->eTypeError,
                                       "Type required instead of "
                                       + cls.GetClass(interpreter)->GetName());

                    return false;
                } else {
                    slot = value.IsInstance(interpreter, cls.As<Class>());

                    return true;
                }
            }

            void Mark()
            {
                TypeHint::Mark();
                if (!m_node->IsMarked())
                {
                    m_node->Mark();
                }
            }

        private:
            Node* m_node;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ExpressionTypeHint);
        };
    }

    Handle<TypeHint> TypeHint::FromExpression(const Handle<Node>& node)
    {
        return new ExpressionTypeHint(node);
    }

    namespace
    {
        class NullableTypeHint : public TypeHint
        {
        public:
            explicit NullableTypeHint(TypeHint* other)
                : m_other(other) {}

            bool Accepts(const Handle<Interpreter>& interpreter, const Value& value, bool& slot) const
            {
                if (value.IsNull())
                {
                    return slot = true;
                } else {
                    return m_other->Accepts(interpreter, value, slot);
                }
            }

            void Mark()
            {
                TypeHint::Mark();
                if (!m_other->IsMarked())
                {
                    m_other->Mark();
                }
            }

        private:
            TypeHint* m_other;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(NullableTypeHint);
        };
    }

    Handle<TypeHint> TypeHint::MakeNullable()
    {
        return new NullableTypeHint(this);
    }

    namespace
    {
        class AndTypeHint : public TypeHint
        {
        public:
            explicit AndTypeHint(TypeHint* left, const Handle<TypeHint>& right)
                : m_left(left)
                , m_right(right.Get()) {}

            bool Accepts(const Handle<Interpreter>& interpreter, const Value& value, bool& slot) const
            {
                if (!m_left->Accepts(interpreter, value, slot))
                {
                    return false;
                }
                else if (!slot)
                {
                    return true;
                } else {
                    return m_right->Accepts(interpreter, value, slot);
                }
            }

            void Mark()
            {
                TypeHint::Mark();
                if (!m_left->IsMarked())
                {
                    m_left->Mark();
                }
                if (!m_right->IsMarked())
                {
                    m_right->Mark();
                }
            }

        private:
            TypeHint* m_left;
            TypeHint* m_right;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(AndTypeHint);
        };
    }

    Handle<TypeHint> TypeHint::MakeAnd(const Handle<TypeHint>& that)
    {
        return new AndTypeHint(this, that);
    }

    namespace
    {
        class OrTypeHint : public TypeHint
        {
        public:
            explicit OrTypeHint(TypeHint* left, const Handle<TypeHint>& right)
                : m_left(left)
                , m_right(right.Get()) {}

            bool Accepts(const Handle<Interpreter>& interpreter, const Value& value, bool& slot) const
            {
                if (!m_left->Accepts(interpreter, value, slot))
                {
                    return false;
                }
                else if (slot)
                {
                    return true;
                } else {
                    return m_right->Accepts(interpreter, value, slot);
                }
            }

            void Mark()
            {
                TypeHint::Mark();
                if (!m_left->IsMarked())
                {
                    m_left->Mark();
                }
                if (!m_right->IsMarked())
                {
                    m_right->Mark();
                }
            }

        private:
            TypeHint* m_left;
            TypeHint* m_right;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(OrTypeHint);
        };
    }

    Handle<TypeHint> TypeHint::MakeOr(const Handle<TypeHint>& that)
    {
        return new OrTypeHint(this, that);
    }
}
