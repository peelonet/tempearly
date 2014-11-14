#include "interpreter.h"
#include "node.h"
#include "parameter.h"
#include "api/function.h"
#include "api/list.h"
#include "api/map.h"
#include "api/range.h"

namespace tempearly
{
    Node::Node() {}

    Value Node::Evaluate(const Handle<Interpreter>& interpreter) const
    {
        Result result = Execute(interpreter);

        if (result.Is(Result::KIND_SUCCESS))
        {
            if (result.HasValue())
            {
                return result.GetValue();
            } else {
                return Value::NullValue();
            }
        }
        else if (result.Is(Result::KIND_BREAK))
        {
            interpreter->Throw(interpreter->eSyntaxError,
                               "Unexpected `break'");
        }
        else if (result.Is(Result::KIND_CONTINUE))
        {
            interpreter->Throw(interpreter->eSyntaxError,
                               "Unexpected `continue'");
        }
        else if (result.Is(Result::KIND_RETURN))
        {
            interpreter->Throw(interpreter->eSyntaxError,
                               "Unexpected `return'");
        }

        return Value();
    }

    bool Node::Assign(const Handle<Interpreter>& interpreter,
                      const Value& value) const
    {
        interpreter->Throw(interpreter->eSyntaxError,
                           "Node is not assignable");

        return false;
    }

    bool Node::AssignLocal(const Handle<Interpreter>& interpreter,
                           const Value& value) const
    {
        return Assign(interpreter, value);
    }

    EmptyNode::EmptyNode() {}

    Result EmptyNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        return Result();
    }

    TextNode::TextNode(const String& content)
        : m_content(content) {}

    Result TextNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        interpreter->response->Write(m_content);

        return Result();
    }

    ExpressionNode::ExpressionNode(const Handle<Node>& expression, bool escape)
        : m_expression(expression.Get())
        , m_escape(escape) {}

    Result ExpressionNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Value value = m_expression->Evaluate(interpreter);

        if (value)
        {
            String string;

            if (value.ToString(interpreter, string))
            {
                if (m_escape)
                {
                    interpreter->response->Write(string.EscapeXml());
                } else {
                    interpreter->response->Write(string);
                }

                return Result();
            }
        }

        return Result(Result::KIND_ERROR);
    }

    void ExpressionNode::Mark()
    {
        Node::Mark();
        if (!m_expression->IsMarked())
        {
            m_expression->Mark();
        }
    }

    BlockNode::BlockNode(const Vector<Handle<Node> >& nodes)
        : m_nodes(nodes) {}

    Result BlockNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        for (std::size_t i = 0; i < m_nodes.GetSize(); ++i)
        {
            Result result = m_nodes[i]->Execute(interpreter);

            if (!result.Is(Result::KIND_SUCCESS))
            {
                return result;
            }
        }

        return Result();
    }

    void BlockNode::Mark()
    {
        Node::Mark();
        for (std::size_t i = 0; i < m_nodes.GetSize(); ++i)
        {
            Node* node = m_nodes[i];

            if (!node->IsMarked())
            {
                node->Mark();
            }
        }
    }

    IfNode::IfNode(const Handle<Node>& condition,
                   const Handle<Node>& then_statement,
                   const Handle<Node>& else_statement)
        : m_condition(condition.Get())
        , m_then_statement(then_statement.Get())
        , m_else_statement(else_statement.Get()) {}

    Result IfNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Value condition = m_condition->Evaluate(interpreter);
        bool b;

        if (!condition || !condition.ToBool(interpreter, b))
        {
            return Result(Result::KIND_ERROR);
        }
        else if (b)
        {
            return m_then_statement->Execute(interpreter);
        }
        else if (m_else_statement)
        {
            return m_else_statement->Execute(interpreter);
        } else {
            return Result();
        }
    }

    void IfNode::Mark()
    {
        Node::Mark();
        if (!m_condition->IsMarked())
        {
            m_condition->Mark();
        }
        if (!m_then_statement->IsMarked())
        {
            m_then_statement->Mark();
        }
        if (m_else_statement && !m_else_statement->IsMarked())
        {
            m_else_statement->Mark();
        }
    }

    WhileNode::WhileNode(const Handle<Node>& condition,
                         const Handle<Node>& statement)
        : m_condition(condition.Get())
        , m_statement(statement.Get()) {}

    Result WhileNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Value condition = m_condition->Evaluate(interpreter);
        bool b;

        if (!condition || !condition.ToBool(interpreter, b))
        {
            return Result(Result::KIND_ERROR);
        }
        while (b)
        {
            Result result = m_statement->Execute(interpreter);

            switch (result.GetKind())
            {
                case Result::KIND_SUCCESS:
                    break;

                case Result::KIND_BREAK:
                    return Result();

                case Result::KIND_CONTINUE:
                    break;

                default:
                    return result;
            }
            if (!(condition = m_condition->Evaluate(interpreter))
                || !condition.ToBool(interpreter, b))
            {
                return Result(Result::KIND_ERROR);
            }
        }

        return Result();
    }

    void WhileNode::Mark()
    {
        Node::Mark();
        if (!m_condition->IsMarked())
        {
            m_condition->Mark();
        }
        if (!m_statement->IsMarked())
        {
            m_statement->Mark();
        }
    }

    ForNode::ForNode(const Handle<Node>& variable,
                     const Handle<Node>& collection,
                     const Handle<Node>& statement)
        : m_variable(variable.Get())
        , m_collection(collection.Get())
        , m_statement(statement.Get()) {}

    Result ForNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Value collection = m_collection->Evaluate(interpreter);
        Value iterator;
        Value element;

        if (!collection || !(iterator = collection.Call(interpreter, "__iter__")))
        {
            return Result(Result::KIND_ERROR);
        }
        interpreter->PushScope(interpreter->GetScope());
        while (iterator.GetNext(interpreter, element))
        {
            if (m_variable->AssignLocal(interpreter, element))
            {
                Result result = m_statement->Execute(interpreter);

                switch (result.GetKind())
                {
                    case Result::KIND_SUCCESS:
                    case Result::KIND_CONTINUE:
                        break;

                    case Result::KIND_BREAK:
                        interpreter->PopScope();
                        return Result();

                    default:
                        interpreter->PopScope();
                        return result;
                }
            } else {
                interpreter->PopScope();

                return Result(Result::KIND_ERROR);
            }
        }
        interpreter->PopScope();
        if (interpreter->HasException())
        {
            return Result(Result::KIND_ERROR);
        } else {
            return Result();
        }
    }

    void ForNode::Mark()
    {
        Node::Mark();
        if (!m_variable->IsMarked())
        {
            m_variable->Mark();
        }
        if (!m_collection->IsMarked())
        {
            m_collection->Mark();
        }
        if (!m_statement->IsMarked())
        {
            m_statement->Mark();
        }
    }

    BreakNode::BreakNode() {}

    Result BreakNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        return Result(Result::KIND_BREAK);
    }

    ContinueNode::ContinueNode() {}

    Result ContinueNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        return Result(Result::KIND_CONTINUE);
    }

    ReturnNode::ReturnNode(const Handle<Node>& value)
        : m_value(value.Get()) {}

    Result ReturnNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        if (m_value)
        {
            Value value = m_value->Evaluate(interpreter);

            if (value)
            {
                return Result(Result::KIND_RETURN, value);
            } else {
                return Result(Result::KIND_ERROR);
            }
        }

        return Result(Result::KIND_RETURN);
    }

    void ReturnNode::Mark()
    {
        Node::Mark();
        if (m_value && !m_value->IsMarked())
        {
            m_value->Mark();
        }
    }

    ValueNode::ValueNode(const Value& value)
        : m_value(value) {}

    Result ValueNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        return m_value;
    }

    void ValueNode::Mark()
    {
        Node::Mark();
        m_value.Mark();
    }

    AndNode::AndNode(const Handle<Node>& left, const Handle<Node>& right)
        : m_left(left.Get())
        , m_right(right.Get()) {}

    Result AndNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Value condition = m_left->Evaluate(interpreter);
        bool b;

        if (!condition || !condition.ToBool(interpreter, b))
        {
            return Result(Result::KIND_ERROR);
        }
        else if (b)
        {
            return m_right->Execute(interpreter);
        } else {
            return condition;
        }
    }

    void AndNode::Mark()
    {
        Node::Mark();
        if (!m_left->IsMarked())
        {
            m_left->Mark();
        }
        if (!m_right->IsMarked())
        {
            m_right->Mark();
        }
    }

    OrNode::OrNode(const Handle<Node>& left, const Handle<Node>& right)
        : m_left(left.Get())
        , m_right(right.Get()) {}

    Result OrNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Value condition = m_left->Evaluate(interpreter);
        bool b;

        if (!condition || !condition.ToBool(interpreter, b))
        {
            return Result(Result::KIND_ERROR);
        }
        else if (b)
        {
            return condition;
        } else {
            return m_right->Execute(interpreter);
        }
    }

    void OrNode::Mark()
    {
        Node::Mark();
        if (!m_left->IsMarked())
        {
            m_left->Mark();
        }
        if (!m_right->IsMarked())
        {
            m_right->Mark();
        }
    }

    NotNode::NotNode(const Handle<Node>& condition)
        : m_condition(condition.Get()) {}

    Result NotNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Value condition = m_condition->Evaluate(interpreter);
        bool b;

        if (!condition || !condition.ToBool(interpreter, b))
        {
            return Result(Result::KIND_ERROR);
        } else {
            return Value::NewBool(!b);
        }
    }

    void NotNode::Mark()
    {
        Node::Mark();
        if (!m_condition->IsMarked())
        {
            m_condition->Mark();
        }
    }

    AttributeNode::AttributeNode(const Handle<Node>& receiver,
                                 const String& id,
                                 bool null_safe)
        : m_receiver(receiver.Get())
        , m_id(id)
        , m_null_safe(null_safe) {}

    bool AttributeNode::IsVariable() const
    {
        return true;
    }

    Result AttributeNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Value value = m_receiver->Evaluate(interpreter);

        if (!value)
        {
            return Result(Result::KIND_ERROR);
        }
        else if (m_null_safe && value.IsNull())
        {
            return Result();
        }
        else if (value.GetAttribute(interpreter, m_id, value))
        {
            return value;
        } else {
            return Result(Result::KIND_ERROR);
        }
    }

    bool AttributeNode::Assign(const Handle<Interpreter>& interpreter,
                               const Value& value) const
    {
        Value receiver = m_receiver->Evaluate(interpreter);

        if (!receiver)
        {
            return false;
        }
        else if (m_null_safe && receiver.IsNull())
        {
            return true;
        } else {
            return receiver.SetAttribute(m_id, value);
        }
    }

    void AttributeNode::Mark()
    {
        Node::Mark();
        if (!m_receiver->IsMarked())
        {
            m_receiver->Mark();
        }
    }

    CallNode::CallNode(const Handle<Node>& receiver,
                       const String& id,
                       const Vector<Handle<Node> >& args,
                       bool null_safe)
        : m_receiver(receiver.Get())
        , m_id(id)
        , m_args(args)
        , m_null_safe(null_safe) {}

    Result CallNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Value value = m_receiver->Evaluate(interpreter);

        if (!value)
        {
            return Result(Result::KIND_ERROR);
        }
        else if (m_null_safe && value.IsNull())
        {
            return Result();
        } else {
            Vector<Value> args;

            args.Reserve(m_args.GetSize());
            for (std::size_t i = 0; i < m_args.GetSize(); ++i)
            {
                Value argument = m_args[i]->Evaluate(interpreter);

                if (!argument)
                {
                    return Result(Result::KIND_ERROR);
                }
                args.PushBack(argument);
            }
            if ((value = value.Call(interpreter, m_id, args)))
            {
                return value;
            } else {
                return Result(Result::KIND_ERROR);
            }
        }
    }

    void CallNode::Mark()
    {
        Node::Mark();
        if (!m_receiver->IsMarked())
        {
            m_receiver->Mark();
        }
        for (std::size_t i = 0; i < m_args.GetSize(); ++i)
        {
            if (!m_args[i]->IsMarked())
            {
                m_args[i]->Mark();
            }
        }
    }

    PrefixNode::PrefixNode(const Handle<Node>& variable, Kind kind)
        : m_variable(variable.Get())
        , m_kind(kind) {}

    Result PrefixNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Value value = m_variable->Evaluate(interpreter);

        if (!value)
        {
            return Result(Result::KIND_ERROR);
        }
        value = value.Call(interpreter, m_kind == INCREMENT ? "__inc__" : "__dec__");
        if (!value || !m_variable->Assign(interpreter, value))
        {
            return Result(Result::KIND_ERROR);
        }

        return value;
    }

    void PrefixNode::Mark()
    {
        Node::Mark();
        if (!m_variable->IsMarked())
        {
            m_variable->Mark();
        }
    }

    PostfixNode::PostfixNode(const Handle<Node>& variable, Kind kind)
        : m_variable(variable.Get())
        , m_kind(kind) {}

    Result PostfixNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Value value = m_variable->Evaluate(interpreter);
        Value result;

        if (!value)
        {
            return Result(Result::KIND_ERROR);
        }
        result = value.Call(interpreter, m_kind == INCREMENT ? "__inc__" : "__dec__");
        if (!result || !m_variable->Assign(interpreter, result))
        {
            return Result(Result::KIND_ERROR);
        }

        return value;
    }

    void PostfixNode::Mark()
    {
        Node::Mark();
        if (!m_variable->IsMarked())
        {
            m_variable->Mark();
        }
    }

    SubscriptNode::SubscriptNode(const Handle<Node>& container,
                                 const Handle<Node>& index)
        : m_container(container.Get())
        , m_index(index.Get()) {}

    bool SubscriptNode::IsVariable() const
    {
        return true;
    }

    Result SubscriptNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Value container;
        Value index;
        Value result;

        if (!(container = m_container->Evaluate(interpreter))
            || !(index = m_index->Evaluate(interpreter))
            || !(result = container.Call(interpreter, "__getitem__", index)))
        {
            return Result(Result::KIND_ERROR);
        } else {
            return result;
        }
    }

    bool SubscriptNode::Assign(const Handle<Interpreter>& interpreter,
                               const Value& value) const
    {
        Value container;
        Value index;
        Vector<Value> args;

        if (!(container = m_container->Evaluate(interpreter))
            || !(index = m_index->Evaluate(interpreter)))
        {
            return false;
        }
        args.Reserve(2);
        args.PushBack(index);
        args.PushBack(value);

        return container.Call(interpreter, "__setitem__", args);
    }

    void SubscriptNode::Mark()
    {
        Node::Mark();
        if (!m_container->IsMarked())
        {
            m_container->Mark();
        }
        if (!m_index->IsMarked())
        {
            m_index->Mark();
        }
    }

    AssignNode::AssignNode(const Handle<Node>& variable,
                           const Handle<Node>& value)
        : m_variable(variable.Get())
        , m_value(value.Get()) {}

    Result AssignNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Value value = m_value->Evaluate(interpreter);

        if (value && m_variable->Assign(interpreter, value))
        {
            return value;
        } else {
            return Result(Result::KIND_ERROR);
        }
    }

    void AssignNode::Mark()
    {
        Node::Mark();
        if (!m_variable->IsMarked())
        {
            m_variable->Mark();
        }
        if (!m_value->IsMarked())
        {
            m_value->Mark();
        }
    }

    IdentifierNode::IdentifierNode(const String& id)
        : m_id(id) {}

    bool IdentifierNode::IsVariable() const
    {
        return true;
    }

    Result IdentifierNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Value value;

        for (Handle<Scope> scope = interpreter->GetScope();
             scope;
             scope = scope->GetParent())
        {
            if (scope->GetVariable(m_id, value))
            {
                return value;
            }
        }
        interpreter->Throw(interpreter->eNameError,
                           "Name '"
                           + m_id
                           + "' is not defined");

        return Result(Result::KIND_ERROR);
    }

    bool IdentifierNode::Assign(const Handle<Interpreter>& interpreter,
                                const Value& value) const
    {
        Handle<Scope> scope;

        // First go through the scope chain and see if some scope already has
        // the variable.
        for (scope = interpreter->GetScope(); scope; scope = scope->GetParent())
        {
            if (scope->HasVariable(m_id))
            {
                scope->SetVariable(m_id, value);

                return true;
            }
        }

        // If no scope has variable with given identifier, create a new variable
        // at the topmost scope.
        if ((scope = interpreter->GetScope()))
        {
            scope->SetVariable(m_id, value);

            return true;
        }

        interpreter->Throw(interpreter->eNameError,
                           "Name '"
                           + m_id
                           + "' is not defined");

        return false;
    }

    bool IdentifierNode::AssignLocal(const Handle<Interpreter>& interpreter,
                                     const Value& value) const
    {
        Handle<Scope> scope = interpreter->GetScope();

        if (scope)
        {
            scope->SetVariable(m_id, value);

            return true;
        }

        interpreter->Throw(interpreter->eNameError,
                           "Name '"
                           + m_id
                           + "' is not defined");

        return false;
    }

    ListNode::ListNode(const Vector<Handle<Node> >& elements)
        : m_elements(elements) {}

    bool ListNode::IsVariable() const
    {
        if (m_elements.IsEmpty())
        {
            return false;
        }
        for (std::size_t i = 0; i < m_elements.GetSize(); ++i)
        {
            if (!m_elements[i]->IsVariable())
            {
                return false;
            }
        }

        return true;
    }

    Result ListNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Handle<ListObject> list = new ListObject(interpreter->cList);
        
        for (std::size_t i = 0; i < m_elements.GetSize(); ++i)
        {
            Value value = m_elements[i]->Evaluate(interpreter);

            if (value)
            {
                list->Append(value);
            } else {
                return Result(Result::KIND_ERROR);
            }
        }

        return Value(list);
    }

    bool ListNode::Assign(const Handle<Interpreter>& interpreter, const Value& value) const
    {
        Value iterator = value.Call(interpreter, "__iter__");
        Value element;
        std::size_t index = 0;

        if (!iterator)
        {
            return false;
        }
        while (iterator.GetNext(interpreter, element))
        {
            if (index < m_elements.GetSize())
            {
                if (!m_elements[index++]->Assign(interpreter, element))
                {
                    return false;
                }
            } else {
                return true;
            }
        }

        return !interpreter->HasException();
    }

    void ListNode::Mark()
    {
        Node::Mark();
        for (std::size_t i = 0; i < m_elements.GetSize(); ++i)
        {
            Node* node = m_elements[i];

            if (!node->IsMarked())
            {
                node->Mark();
            }
        }
    }

    MapNode::MapNode(const Vector<Pair<Handle<Node> > >& entries)
        : m_entries(entries) {}

    Result MapNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Handle<MapObject> map = new MapObject(interpreter->cMap);
        Value key;
        Value value;
        i64 hash;

        for (std::size_t i = 0; i < m_entries.GetSize(); ++i)
        {
            const Pair<Node*>& entry = m_entries[i];

            if (!(key = entry.GetKey()->Evaluate(interpreter))
                || !(value = entry.GetValue()->Evaluate(interpreter))
                || !key.GetHash(interpreter, hash))
            {
                return Result(Result::KIND_ERROR);
            }
            map->Insert(hash, key, value);
        }

        return Value(map);
    }

    void MapNode::Mark()
    {
        Node::Mark();
        for (std::size_t i = 0; i < m_entries.GetSize(); ++i)
        {
            const Pair<Node*>& entry = m_entries[i];

            if (!entry.GetKey()->IsMarked())
            {
                entry.GetKey()->Mark();
            }
            if (!entry.GetValue()->IsMarked())
            {
                entry.GetValue()->Mark();
            }
        }
    }

    RangeNode::RangeNode(const Handle<Node>& begin,
                         const Handle<Node>& end,
                         bool exclusive)
        : m_begin(begin.Get())
        , m_end(end.Get())
        , m_exclusive(exclusive) {}

    Result RangeNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Value begin;
        Value end;
        
        if (!(begin = m_begin->Evaluate(interpreter))
            || !(end = m_end->Evaluate(interpreter)))
        {
            return Result(Result::KIND_ERROR);
        }

        return Value(new RangeObject(interpreter, begin, end, m_exclusive));
    }

    void RangeNode::Mark()
    {
        Node::Mark();
        if (!m_begin->IsMarked())
        {
            m_begin->Mark();
        }
        if (!m_end->IsMarked())
        {
            m_end->Mark();
        }
    }

    FunctionNode::FunctionNode(const Vector<Handle<Parameter> >& parameters,
                               const Vector<Handle<Node> >& nodes)
        : m_parameters(parameters)
        , m_nodes(nodes) {}

    Result FunctionNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        return Value(FunctionObject::NewScripted(interpreter, m_parameters, m_nodes));
    }

    void FunctionNode::Mark()
    {
        Node::Mark();
        for (std::size_t i = 0; i < m_parameters.GetSize(); ++i)
        {
            if (!m_parameters[i]->IsMarked())
            {
                m_parameters[i]->Mark();
            }
        }
        for (std::size_t i = 0; i < m_nodes.GetSize(); ++i)
        {
            if (!m_nodes[i]->IsMarked())
            {
                m_nodes[i]->Mark();
            }
        }
    }
}
