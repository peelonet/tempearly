#include "interpreter.h"
#include "node.h"
#include "parameter.h"
#include "api/list.h"
#include "api/map.h"
#include "api/range.h"

namespace tempearly
{
    Node::Node() {}

    bool Node::Evaluate(const Handle<Interpreter>& interpreter,
                        Handle<Object>& slot) const
    {
        Result result = Execute(interpreter);

        if (result.Is(Result::KIND_SUCCESS))
        {
            if (result.HasValue())
            {
                slot = result.GetValue();
            } else {
                slot = Object::NewNull();
            }

            return true;
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

        return false;
    }

    bool Node::Assign(const Handle<Interpreter>& interpreter,
                      const Handle<Object>& value) const
    {
        interpreter->Throw(interpreter->eSyntaxError,
                           "Node is not assignable");

        return false;
    }

    bool Node::AssignLocal(const Handle<Interpreter>& interpreter,
                           const Handle<Object>& value) const
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
        interpreter->GetResponse()->Write(m_content);

        return Result();
    }

    ExpressionNode::ExpressionNode(const Handle<Node>& expression, bool escape)
        : m_expression(expression.Get())
        , m_escape(escape) {}

    Result ExpressionNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Handle<Object> value;

        if (m_expression->Evaluate(interpreter, value))
        {
            String string;

            if (value->ToString(interpreter, string))
            {
                if (m_escape)
                {
                    string = string.EscapeXml();
                }
                interpreter->GetResponse()->Write(string);

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

    BlockNode::BlockNode(const Vector<Handle<Node>>& nodes)
        : m_nodes(nodes) {}

    Result BlockNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        for (std::size_t i = 0; i < m_nodes.GetSize(); ++i)
        {
            const Result result = m_nodes[i]->Execute(interpreter);

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
        Handle<Object> condition;
        bool b;

        if (!m_condition->Evaluate(interpreter, condition)
            || !condition->ToBool(interpreter, b))
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
        Handle<Object> condition;
        bool b;

        if (!m_condition->Evaluate(interpreter, condition)
            || !condition->ToBool(interpreter, b))
        {
            return Result(Result::KIND_ERROR);
        }
        while (b)
        {
            const Result result = m_statement->Execute(interpreter);

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
            if (!m_condition->Evaluate(interpreter, condition)
                || !condition->ToBool(interpreter, b))
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
                     const Handle<Node>& statement,
                     const Handle<Node>& else_statement)
        : m_variable(variable.Get())
        , m_collection(collection.Get())
        , m_statement(statement.Get())
        , m_else_statement(else_statement) {}

    Result ForNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Handle<Object> collection;
        Handle<Object> iterator;
        Handle<Object> element;

        if (!m_collection->Evaluate(interpreter, collection)
            || !collection->CallMethod(interpreter, iterator, "__iter__"))
        {
            return Result(Result::KIND_ERROR);
        }
        if (iterator->GetNext(interpreter, element))
        {
            do
            {
                if (m_variable->AssignLocal(interpreter, element))
                {
                    const Result result = m_statement->Execute(interpreter);

                    switch (result.GetKind())
                    {
                        case Result::KIND_SUCCESS:
                        case Result::KIND_CONTINUE:
                            break;

                        case Result::KIND_BREAK:
                            return Result();

                        default:
                            return result;
                    }
                } else {
                    return Result(Result::KIND_ERROR);
                }
            }
            while (iterator->GetNext(interpreter, element));
            if (interpreter->HasException())
            {
                return Result(Result::KIND_ERROR);
            } else {
                return Result();
            }
        }
        else if (interpreter->HasException())
        {
            return Result(Result::KIND_ERROR);
        }
        else if (m_else_statement)
        {
            return m_else_statement->Execute(interpreter);
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
        if (m_else_statement && !m_else_statement->IsMarked())
        {
            m_else_statement->Mark();
        }
    }

    CatchNode::CatchNode(const Handle<TypeHint>& type, const Handle<Node>& variable, const Handle<Node>& statement)
        : m_type(type.Get())
        , m_variable(variable.Get())
        , m_statement(statement.Get()) {}

    bool CatchNode::IsCatch(const Handle<Interpreter>& interpreter,
                            const Handle<Object>& exception,
                            bool& slot) const
    {
        if (m_type)
        {
            return m_type->Accepts(interpreter, exception, slot);
        } else {
            return slot = true;
        }
    }

    Result CatchNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        if (m_variable && !m_variable->Assign(interpreter, interpreter->GetCaughtException()))
        {
            return Result(Result::KIND_ERROR);
        } else {
            return m_statement->Execute(interpreter);
        }
    }

    void CatchNode::Mark()
    {
        Node::Mark();
        if (m_type && !m_type->IsMarked())
        {
            m_type->Mark();
        }
        if (m_variable && !m_variable->IsMarked())
        {
            m_variable->Mark();
        }
        if (!m_statement->IsMarked())
        {
            m_statement->Mark();
        }
    }

    TryNode::TryNode(const Handle<Node>& statement,
                     const Vector<Handle<CatchNode> >& catches,
                     const Handle<Node>& else_statement,
                     const Handle<Node>& finally_statement)
        : m_statement(statement.Get())
        , m_catches(catches)
        , m_else_statement(else_statement.Get())
        , m_finally_statement(finally_statement.Get()) {}

    Result TryNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Result result = m_statement->Execute(interpreter);

        if (result.Is(Result::KIND_ERROR))
        {
            const Handle<ExceptionObject> exception = interpreter->GetException();
            bool caught;

            for (std::size_t i = 0; i < m_catches.GetSize(); ++i)
            {
                Handle<CatchNode> c = m_catches[i];

                if (!c->IsCatch(interpreter, exception, caught))
                {
                    break;
                }
                else if (caught)
                {
                    interpreter->SetCaughtException(exception);
                    interpreter->ClearException();
                    result = c->Execute(interpreter);
                    interpreter->ClearCaughtException();
                    break;
                }
            }
        }
        else if (m_else_statement)
        {
            result = m_else_statement->Execute(interpreter);
        }
        if (m_finally_statement)
        {
            Result finally_result = m_finally_statement->Execute(interpreter);

            if (finally_result.Is(Result::KIND_ERROR))
            {
                return finally_result;
            }
        }

        return result;
    }

    void TryNode::Mark()
    {
        Node::Mark();
        if (!m_statement->IsMarked())
        {
            m_statement->Mark();
        }
        for (std::size_t i = 0; i < m_catches.GetSize(); ++i)
        {
            if (!m_catches[i]->IsMarked())
            {
                m_catches[i]->Mark();
            }
        }
        if (m_else_statement && m_else_statement->IsMarked())
        {
            m_else_statement->Mark();
        }
        if (m_finally_statement && m_finally_statement->IsMarked())
        {
            m_finally_statement->Mark();
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
            Handle<Object> value;

            if (m_value->Evaluate(interpreter, value))
            {
                return value;
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

    ThrowNode::ThrowNode(const Handle<Node>& exception)
        : m_exception(exception.Get()) {}

    Result ThrowNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Handle<Object> exception;

        if (m_exception)
        {
            if (!m_exception->Evaluate(interpreter, exception))
            {
                return Result(Result::KIND_ERROR);
            }
            else if (!exception->IsInstance(interpreter, interpreter->cException))
            {
                interpreter->Throw(
                    interpreter->eTypeError,
                    "Cannot throw instance of '"
                    + exception->GetClass(interpreter)->GetName()
                    + "'"
                );

                return Result(Result::KIND_ERROR);
            }
        } else {
            exception = interpreter->GetCaughtException();
            if (!exception)
            {
                interpreter->Throw(interpreter->eStateError, "No previously caught exception");

                return Result(Result::KIND_ERROR);
            }
            interpreter->ClearCaughtException();
        }
        interpreter->SetException(exception.As<ExceptionObject>());

        return Result(Result::KIND_ERROR);
    }

    void ThrowNode::Mark()
    {
        Node::Mark();
        if (m_exception && !m_exception->IsMarked())
        {
            m_exception->Mark();
        }
    }

    ValueNode::ValueNode(const Handle<Object>& value)
        : m_value(value) {}

    Result ValueNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        return Handle<Object>(m_value);
    }

    void ValueNode::Mark()
    {
        Node::Mark();
        if (!m_value->IsMarked())
        {
            m_value->Mark();
        }
    }

    AndNode::AndNode(const Handle<Node>& left, const Handle<Node>& right)
        : m_left(left.Get())
        , m_right(right.Get()) {}

    Result AndNode::Execute(const Handle<Interpreter>& interpreter) const
    {
        Handle<Object> condition;
        bool b;

        if (!m_left->Evaluate(interpreter, condition)
            || !condition->ToBool(interpreter, b))
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
        Handle<Object> condition;
        bool b;

        if (!m_left->Evaluate(interpreter, condition)
            || !condition->ToBool(interpreter, b))
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
        Handle<Object> condition;
        bool b;

        if (!m_condition->Evaluate(interpreter, condition)
            || !condition->ToBool(interpreter, b))
        {
            return Result(Result::KIND_ERROR);
        } else {
            return Object::NewBool(!b);
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
        Handle<Object> value;

        if (!m_receiver->Evaluate(interpreter, value))
        {
            return Result(Result::KIND_ERROR);
        }
        else if (m_null_safe && value->IsNull())
        {
            return Result();
        }
        else if (value->GetAttribute(interpreter, m_id, value))
        {
            return value;
        } else {
            return Result(Result::KIND_ERROR);
        }
    }

    bool AttributeNode::Assign(const Handle<Interpreter>& interpreter,
                               const Handle<Object>& value) const
    {
        Handle<Object> receiver;

        if (!m_receiver->Evaluate(interpreter, receiver))
        {
            return false;
        }
        else if (m_null_safe && receiver->IsNull())
        {
            return true;
        } else {
            return receiver->SetOwnAttribute(m_id, value);
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
        Handle<Object> value;

        if (!m_receiver->Evaluate(interpreter, value))
        {
            return Result(Result::KIND_ERROR);
        }
        else if (m_null_safe && value->IsNull())
        {
            return Result();
        } else {
            Vector<Handle<Object>> args;

            args.Reserve(m_args.GetSize());
            for (std::size_t i = 0; i < m_args.GetSize(); ++i)
            {
                Handle<Object> argument;

                if (!m_args[i]->Evaluate(interpreter, argument))
                {
                    return Result(Result::KIND_ERROR);
                }
                args.PushBack(argument);
            }
            if (value->CallMethod(interpreter, value, m_id, args))
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
        Handle<Object> value;

        if (!m_variable->Evaluate(interpreter, value))
        {
            return Result(Result::KIND_ERROR);
        }
        if (!value->CallMethod(interpreter, value, m_kind == INCREMENT ? "__inc__" : "__dec__")
            || !m_variable->Assign(interpreter, value))
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
        Handle<Object> value;
        Handle<Object> result;

        if (!m_variable->Evaluate(interpreter, value))
        {
            return Result(Result::KIND_ERROR);
        }
        if (!value->CallMethod(interpreter, result, m_kind == INCREMENT ? "__inc__" : "__dec__")
            || !m_variable->Assign(interpreter, result))
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
        Handle<Object> container;
        Handle<Object> index;
        Handle<Object> result;

        if (!m_container->Evaluate(interpreter, container)
            || !m_index->Evaluate(interpreter, index)
            || !container->CallMethod(interpreter, result, "__getitem__", index))
        {
            return Result(Result::KIND_ERROR);
        } else {
            return result;
        }
    }

    bool SubscriptNode::Assign(const Handle<Interpreter>& interpreter,
                               const Handle<Object>& value) const
    {
        Handle<Object> container;
        Handle<Object> index;
        Vector<Handle<Object>> args;

        if (!m_container->Evaluate(interpreter, container)
            || !m_index->Evaluate(interpreter, index))
        {
            return false;
        }
        args.Reserve(2);
        args.PushBack(index);
        args.PushBack(value);

        return container->CallMethod(interpreter, "__setitem__", args);
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
        Handle<Object> value;

        if (m_value->Evaluate(interpreter, value)
            && m_variable->Assign(interpreter, value))
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
        Handle<Object> value;

        for (Handle<Frame> frame = interpreter->GetFrame(); frame; frame = frame->GetEnclosingFrame())
        {
            if (frame->GetLocalVariable(m_id, value))
            {
                return value;
            }
        }
        if (interpreter->GetGlobalVariable(m_id, value))
        {
            return value;
        }
        interpreter->Throw(interpreter->eNameError, "Name '" + m_id + "' is not defined");

        return Result(Result::KIND_ERROR);
    }

    bool IdentifierNode::Assign(const Handle<Interpreter>& interpreter,
                                const Handle<Object>& value) const
    {
        Handle<Frame> frame;

        // First go through the scope chain and see if some scope already has
        // the variable.
        for (frame = interpreter->GetFrame(); frame; frame = frame->GetEnclosingFrame())
        {
            if (frame->ReplaceLocalVariable(m_id, value))
            {
                return true;
            }
        }

        // If no scope has variable with given identifier, create a new variable
        // at the topmost scope.
        if ((frame = interpreter->GetFrame()))
        {
            frame->SetLocalVariable(m_id, value);

            return true;
        }

        interpreter->Throw(interpreter->eNameError, "Name '" + m_id + "' is not defined");

        return false;
    }

    bool IdentifierNode::AssignLocal(const Handle<Interpreter>& interpreter,
                                     const Handle<Object>& value) const
    {
        Handle<Frame> frame = interpreter->GetFrame();

        if (frame)
        {
            frame->SetLocalVariable(m_id, value);

            return true;
        }

        interpreter->Throw(interpreter->eNameError, "Name '" + m_id + "' is not defined");

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
            Handle<Object> value;

            if (m_elements[i]->Evaluate(interpreter, value))
            {
                list->Append(value);
            } else {
                return Result(Result::KIND_ERROR);
            }
        }

        return Result(Result::KIND_SUCCESS, list);
    }

    bool ListNode::Assign(const Handle<Interpreter>& interpreter,
                          const Handle<Object>& value) const
    {
        Handle<Object> iterator;
        Handle<Object> element;
        std::size_t index = 0;

        if (!value->CallMethod(interpreter, iterator, "__iter__"))
        {
            return false;
        }
        while (iterator->GetNext(interpreter, element))
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
        Handle<Object> key;
        Handle<Object> value;
        i64 hash;

        for (std::size_t i = 0; i < m_entries.GetSize(); ++i)
        {
            const Pair<Node*>& entry = m_entries[i];

            if (!entry.GetKey()->Evaluate(interpreter, key)
                || !entry.GetValue()->Evaluate(interpreter, value)
                || !key->GetHash(interpreter, hash))
            {
                return Result(Result::KIND_ERROR);
            }
            map->Insert(hash, key, value);
        }

        return Result(Result::KIND_SUCCESS, map);
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
        Handle<Object> begin;
        Handle<Object> end;
        
        if (!m_begin->Evaluate(interpreter, begin)
            || !m_end->Evaluate(interpreter, end))
        {
            return Result(Result::KIND_ERROR);
        }

        return Result(
            Result::KIND_SUCCESS,
            new RangeObject(interpreter, begin, end, m_exclusive)
        );
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
        return Result(
            Result::KIND_SUCCESS,
            FunctionObject::NewScripted(interpreter, m_parameters, m_nodes)
        );
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
