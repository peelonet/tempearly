#ifndef TEMPEARLY_NODE_H_GUARD
#define TEMPEARLY_NODE_H_GUARD

#include "result.h"
#include "value.h"

namespace tempearly
{
    class Node : public CountedObject
    {
    public:
        explicit Node();

        /**
         * Returns true if this node is an variable, e.g. node where an value
         * can be assigned to.
         */
        virtual bool IsVariable() const
        {
            return false;
        }

        /**
         * Executes node as statement.
         *
         * \param interpreter Script interpreter
         * \return            Execution result returned by the statement
         */
        virtual Result Execute(const Handle<Interpreter>& interpreter) const = 0;

        /**
         * Evaluates node as expression.
         *
         * \param interpreter Script interpreter
         * \return            Value returned by the expression, or error value
         *                    if an exception was thrown
         */
        Value Evaluate(const Handle<Interpreter>& interpreter) const;

        /**
         * Uses node as variable and assigns an value into it.
         *
         * \param interpreter Script interpreter
         * \param value       Value to assign
         * \return            Boolean flag indicating whether operation was
         *                    successfull or not, if false it means that an
         *                    exception was thrown
         */
        virtual bool Assign(const Handle<Interpreter>& interpreter,
                            const Value& value) const;

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Node);
    };

    /**
     * Node which does nothing.
     */
    class EmptyNode : public Node
    {
    public:
        explicit EmptyNode();

        Result Execute(const Handle<Interpreter>& interpreter) const;

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(EmptyNode);
    };

    /**
     * Node which outputs text to the client.
     */
    class TextNode : public Node
    {
    public:
        explicit TextNode(const String& content);

        Result Execute(const Handle<Interpreter>& interpreter) const;

    private:
        const String m_content;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(TextNode);
    };

    class ExpressionNode : public Node
    {
    public:
        explicit ExpressionNode(const Handle<Node>& expression,
                                bool escape = false);

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        Node* m_expression;
        const bool m_escape;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ExpressionNode);
    };

    class ValueNode : public Node
    {
    public:
        explicit ValueNode(const Value& value);

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        const Value m_value;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ValueNode);
    };

    class AndNode : public Node
    {
    public:
        explicit AndNode(const Handle<Node>& left,
                         const Handle<Node>& right);

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        Node* m_left;
        Node* m_right;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(AndNode);
    };

    class OrNode : public Node
    {
    public:
        explicit OrNode(const Handle<Node>& left,
                        const Handle<Node>& right);

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        Node* m_left;
        Node* m_right;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(OrNode);
    };

    class NotNode : public Node
    {
    public:
        explicit NotNode(const Handle<Node>& condition);

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        Node* m_condition;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(NotNode);
    };

    class AttributeNode : public Node
    {
    public:
        explicit AttributeNode(const Handle<Node>& receiver,
                               const String& id,
                               bool null_safe = false);

        bool IsVariable() const;

        Result Execute(const Handle<Interpreter>& interpreter) const;

        bool Assign(const Handle<Interpreter>& interpreter,
                    const Value& value) const;

        void Mark();

    private:
        Node* m_receiver;
        const String m_id;
        const bool m_null_safe;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(AttributeNode);
    };

    class CallNode : public Node
    {
    public:
        explicit CallNode(const Handle<Node>& receiver,
                          const String& id,
                          const std::vector<Handle<Node> >& args = std::vector<Handle<Node> >(),
                          bool null_safe = false);

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        Node* m_receiver;
        const String m_id;
        const std::vector<Node*> m_args;
        const bool m_null_safe;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(CallNode);
    };

    class PrefixNode : public Node
    {
    public:
        enum Kind
        {
            INCREMENT,
            DECREMENT
        };

        explicit PrefixNode(const Handle<Node>& variable, Kind kind);

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        Node* m_variable;
        const Kind m_kind;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(PrefixNode);
    };

    class PostfixNode : public Node
    {
    public:
        enum Kind
        {
            INCREMENT,
            DECREMENT
        };

        explicit PostfixNode(const Handle<Node>& variable, Kind kind);

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        Node* m_variable;
        const Kind m_kind;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(PostfixNode);
    };
}

#endif /* !TEMPEARLY_NODE_H_GUARD */
