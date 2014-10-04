#include "interpreter.h"
#include "node.h"

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
        } else {
            // TODO: throw exception

            return Value();
        }
    }

    bool Node::Assign(const Handle<Interpreter>& interpreter,
                      const Value& value) const
    {
        interpreter->Throw(interpreter->eSyntaxError,
                           "Node is not assignable");

        return false;
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
                    // TODO: html escape the string
                }
                interpreter->response->Write(string);

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

    ValueNode::ValueNode(const Value& value)
        : m_value(value) {}

    Result ValueNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        return Result(Result::KIND_SUCCESS, m_value);
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
            return Result(Result::KIND_SUCCESS, condition);
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
            return Result(Result::KIND_SUCCESS, condition);
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
            return Result(Result::KIND_SUCCESS, Value::NewBool(!b));
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
            return Result(Result::KIND_SUCCESS, value);
        } else {
            return Result(Result::KIND_ERROR);
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
                       const std::vector<Handle<Node> >& args,
                       bool null_safe)
        : m_receiver(receiver.Get())
        , m_id(id)
        , m_args(args.begin(), args.end())
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
            std::vector<Value> args;

            args.reserve(m_args.size());
            for (std::size_t i = 0; i < m_args.size(); ++i)
            {
                Value argument = m_args[i]->Evaluate(interpreter);

                if (!argument)
                {
                    return Result(Result::KIND_ERROR);
                }
                args.push_back(argument);
            }
            if ((value = value.Call(interpreter, m_id, args)))
            {
                return Result(Result::KIND_SUCCESS, value);
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
        for (std::size_t i = 0; i < m_args.size(); ++i)
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

        return Result(Result::KIND_SUCCESS, value);
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

        return Result(Result::KIND_SUCCESS, value);
    }

    void PostfixNode::Mark()
    {
        Node::Mark();
        if (!m_variable->IsMarked())
        {
            m_variable->Mark();
        }
    }
}
