#ifndef TEMPEARLY_SCRIPT_NODE_H_GUARD
#define TEMPEARLY_SCRIPT_NODE_H_GUARD

#include "core/pair.h"
#include "script/result.h"

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

        /**
         * Same as Assign, but uses current local variable scope, instead of
         * scope chain.
         *
         * Default implementation simply calls Assign. Override this method
         * only when it makes sense.
         *
         * \param interpreter Script interpreter
         * \param value Value to assign
         * \return Boolean flag indicating whether operation was
         * successfull or not, if false it means that an
         * exception was thrown
         */
         virtual bool AssignLocal(const Handle<Interpreter>& interpreter,
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

    class BlockNode : public Node
    {
    public:
        explicit BlockNode(const Vector<Handle<Node> >& nodes);

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        const Vector<Node*> m_nodes;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(BlockNode);
    };

    class IfNode : public Node
    {
    public:
        explicit IfNode(const Handle<Node>& condition,
                        const Handle<Node>& then_statement,
                        const Handle<Node>& else_statement = Handle<Node>());

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        Node* m_condition;
        Node* m_then_statement;
        Node* m_else_statement;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(IfNode);
    };

    class WhileNode : public Node
    {
    public:
        explicit WhileNode(const Handle<Node>& condition,
                           const Handle<Node>& statement);

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        Node* m_condition;
        Node* m_statement;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(WhileNode);
    };

    class ForNode : public Node
    {
    public:
        explicit ForNode(const Handle<Node>& variable,
                         const Handle<Node>& collection,
                         const Handle<Node>& statement);

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        Node* m_variable;
        Node* m_collection;
        Node* m_statement;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ForNode);
    };

    class CatchNode : public Node
    {
    public:
        explicit CatchNode(const Handle<TypeHint>& type, const Handle<Node>& variable, const Handle<Node>& statement);

        bool IsCatch(const Handle<Interpreter>& interpreter, const Value& exception, bool& slot) const;

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        TypeHint* m_type;
        Node* m_variable;
        Node* m_statement;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(CatchNode);
    };

    class TryNode : public Node
    {
    public:
        explicit TryNode(const Handle<Node>& statement,
                         const Vector<Handle<CatchNode> >& catches = Vector<Handle<CatchNode> >(),
                         const Handle<Node>& else_statement = Handle<Node>(),
                         const Handle<Node>& finally_statement = Handle<Node>());

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        Node* m_statement;
        const Vector<CatchNode*> m_catches;
        Node* m_else_statement;
        Node* m_finally_statement;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(TryNode);
    };

    class BreakNode : public Node
    {
    public:
        explicit BreakNode();

        Result Execute(const Handle<Interpreter>& interpreter) const;

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(BreakNode);
    };

    class ContinueNode : public Node
    {
    public:
        explicit ContinueNode();

        Result Execute(const Handle<Interpreter>& interpreter) const;

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ContinueNode);
    };

    class ReturnNode : public Node
    {
    public:
        explicit ReturnNode(const Handle<Node>& value = Handle<Node>());

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        Node* m_value;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ReturnNode);
    };

    class ThrowNode : public Node
    {
    public:
        explicit ThrowNode(const Handle<Node>& exception = Handle<Node>());

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        Node* m_exception;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ThrowNode);
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
                          const Vector<Handle<Node> >& args = Vector<Handle<Node> >(),
                          bool null_safe = false);

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        Node* m_receiver;
        const String m_id;
        const Vector<Node*> m_args;
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

    class SubscriptNode : public Node
    {
    public:
        explicit SubscriptNode(const Handle<Node>& container,
                               const Handle<Node>& index);

        bool IsVariable() const;

        Result Execute(const Handle<Interpreter>& interpreter) const;

        bool Assign(const Handle<Interpreter>& interpreter,
                    const Value& value) const;

        void Mark();

    private:
        Node* m_container;
        Node* m_index;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(SubscriptNode);
    };

    class AssignNode : public Node
    {
    public:
        explicit AssignNode(const Handle<Node>& variable,
                            const Handle<Node>& value);

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        Node* m_variable;
        Node* m_value;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(AssignNode);
    };

    class IdentifierNode : public Node
    {
    public:
        explicit IdentifierNode(const String& id);

        bool IsVariable() const;

        Result Execute(const Handle<Interpreter>& interpreter) const;

        bool Assign(const Handle<Interpreter>& interpreter,
                    const Value& value) const;

        bool AssignLocal(const Handle<Interpreter>& interpreter,
                         const Value& value) const;

    private:
        const String m_id;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(IdentifierNode);
    };

    class ListNode : public Node
    {
    public:
        explicit ListNode(const Vector<Handle<Node> >& elements);

        bool IsVariable() const;

        Result Execute(const Handle<Interpreter>& interpreter) const;

        bool Assign(const Handle<Interpreter>& interpreter,
                    const Value& value) const;

        void Mark();

    private:
        const Vector<Node*> m_elements;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ListNode);
    };

    class MapNode : public Node
    {
    public:
        explicit MapNode(const Vector<Pair<Handle<Node> > >& entries);

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        const Vector<Pair<Node*> > m_entries;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(MapNode);
    };

    class RangeNode : public Node
    {
    public:
        explicit RangeNode(const Handle<Node>& begin,
                           const Handle<Node>& end,
                           bool exclusive);

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        Node* m_begin;
        Node* m_end;
        const bool m_exclusive;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(RangeNode);
    };

    class FunctionNode : public Node
    {
    public:
        explicit FunctionNode(const Vector<Handle<Parameter> >& parameters,
                              const Vector<Handle<Node> >& nodes);

        Result Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        const Vector<Parameter*> m_parameters;
        const Vector<Node*> m_nodes;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(FunctionNode);
    };
}

#endif /* !TEMPEARLY_SCRIPT_NODE_H_GUARD */
