#include "interpreter.h"
#include "api/iterator.h"
#include "api/list.h"

namespace tempearly
{
    ListObject::ListObject(const Handle<Class>& cls)
        : Object(cls)
        , m_size(0)
        , m_front(0)
        , m_back(0) {}

    void ListObject::Append(const Value& value)
    {
        Handle<Link> link = new Link(value);

        if (m_back)
        {
            m_back->m_next = link;
        } else {
            m_front = link;
        }
        m_back = link;
        ++m_size;
    }

    void ListObject::Append(const Vector<Value>& vector)
    {
        if (!vector.IsEmpty())
        {
            Handle<Link> front = new Link(vector[0]);
            Handle<Link> back = front;

            for (std::size_t i = 1; i < vector.GetSize(); ++i)
            {
                Handle<Link> link = new Link(vector[i]);

                back->m_next = link;
                back = link;
            }
            if (m_back)
            {
                m_back->m_next = front;
            } else {
                m_front = front;
            }
            m_back = back;
            m_size += vector.GetSize();
        }
    }

    void ListObject::Append(const Handle<ListObject>& that)
    {
        if (!that || this == that.Get())
        {
            return;
        }
        for (const Link* a = that->m_front; a; a = a->m_next)
        {
            Handle<Link> b = new Link(a->m_value);

            if (m_back)
            {
                m_back->m_next = b;
            } else {
                m_front = b;
            }
            m_back = b;
        }
        m_size += that->m_size;
    }

    void ListObject::Prepend(const Value& value)
    {
        Handle<Link> link = new Link(value);

        if (!(link->m_next = m_front))
        {
            m_back = link;
        }
        m_front = link;
        ++m_size;
    }

    void ListObject::Prepend(const Vector<Value>& vector)
    {
        if (!vector.IsEmpty())
        {
            Handle<Link> front = new Link(vector[0]);
            Handle<Link> back = front;

            for (std::size_t i = 1; i < vector.GetSize(); ++i)
            {
                Handle<Link> link = new Link(vector[i]);

                back->m_next = link;
                back = link;
            }
            if (m_front)
            {
                back->m_next = m_front;
            } else {
                m_back = back;
            }
            m_front = front;
            m_size += vector.GetSize();
        }
    }

    void ListObject::Clear()
    {
        m_front = m_back = 0;
        m_size = 0;
    }

    void ListObject::Mark()
    {
        Object::Mark();
        if (m_front && !m_front->IsMarked())
        {
            m_front->Mark();
        }
    }

    ListObject::Link::Link(const Value& value)
        : m_value(value)
        , m_next(0) {}

    void ListObject::Link::Mark()
    {
        CountedObject::Mark();
        m_value.Mark();
        if (m_next && !m_next->IsMarked())
        {
            m_next->Mark();
        }
    }

    static Handle<CoreObject> list_alloc(const Handle<Interpreter>& interpreter,
                                         const Handle<Class>& cls)
    {
        return new ListObject(cls);
    }

    /**
     * List#__init__(object...)
     *
     * Initializes list by inserting objects given as arguments into it.
     */
    TEMPEARLY_NATIVE_METHOD(list_init)
    {
        Handle<ListObject> list = args[0].As<ListObject>();

        if (!list->IsEmpty())
        {
            list->Clear();
        }
        if (args.GetSize() > 1)
        {
            list->Append(args.SubVector(1));
        }
    }

    /**
     * List#size() => Int
     *
     * Returns the number of elements stored in the list.
     */
    TEMPEARLY_NATIVE_METHOD(list_size)
    {
        frame->SetReturnValue(Value::NewInt(args[0].As<ListObject>()->GetSize()));
    }

    /**
     * List#append(object...)
     *
     * Inserts all objects given as arguments at the end of the list.
     *
     *     ["a"].append("b", "c")  #=> ["a", "b", "c"]
     */
    TEMPEARLY_NATIVE_METHOD(list_append)
    {
        if (args.GetSize() > 1)
        {
            args[0].As<ListObject>()->Append(args.SubVector(1));
        }
        frame->SetReturnValue(args[0]);
    }

    /**
     * List#prepend(object...)
     *
     * Inserts all objects given as arguments at the beginning of the list.
     *
     *     ["c"].append("a", "b")  #=> ["a", "b", "c"]
     */
    TEMPEARLY_NATIVE_METHOD(list_prepend)
    {
        if (args.GetSize() > 1)
        {
            args[0].As<ListObject>()->Prepend(args.SubVector(1));
        }
        frame->SetReturnValue(args[0]);
    }

    /**
     * List#clear()
     *
     * Removes all elements from the list.
     */
    TEMPEARLY_NATIVE_METHOD(list_clear)
    {
        args[0].As<ListObject>()->Clear();
        frame->SetReturnValue(args[0]);
    }

    namespace
    {
        class ListIterator : public IteratorObject
        {
        public:
            explicit ListIterator(const Handle<Class>& cls,
                                  const Handle<ListObject>& list)
                : IteratorObject(cls)
                , m_link(list->GetFront()) {}

            Result Generate(const Handle<Interpreter>& interpreter)
            {
                if (m_link)
                {
                    Value value = m_link->GetValue();

                    m_link = m_link->GetNext();

                    return Result(Result::KIND_SUCCESS, value);
                }

                return Result(Result::KIND_BREAK);
            }

            void Mark()
            {
                IteratorObject::Mark();
                if (m_link && !m_link->IsMarked())
                {
                    m_link->Mark();
                }
            }

        private:
            ListObject::Link* m_link;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ListIterator);
        };
    }

    /**
     * List#__iter__() => Iterator
     *
     * Returns an iterator which traverses over each element in the list.
     */
    TEMPEARLY_NATIVE_METHOD(list_iter)
    {
        Handle<ListObject> list = args[0].As<ListObject>();
        Handle<IteratorObject> iterator;

        if (list->IsEmpty())
        {
            iterator = interpreter->GetEmptyIterator();
        } else {
            iterator = new ListIterator(interpreter->cIterator, list);
        }
        frame->SetReturnValue(Value(iterator));
    }

    /**
     * List#__add__(collection) => List
     *
     * Returns a new list which contains items from the list with items
     * included from the object given as argument.
     *
     *     a = [1, 2, 3];
     *     a += [4, 5];
     *     a;               #=> [1, 2, 3, 4, 5]
     */
    TEMPEARLY_NATIVE_METHOD(list_add)
    {
        Handle<ListObject> original = args[0].As<ListObject>();
        Handle<ListObject> result;
        Value iterator = args[1].Call(interpreter, "__iter__");
        Value element;

        if (!iterator)
        {
            return;
        }
        result = new ListObject(interpreter->cList);
        result->Append(original);
        while (iterator.GetNext(interpreter, element))
        {
            result->Append(element);
        }
        if (!interpreter->HasException())
        {
            frame->SetReturnValue(Value(result));
        }
    }

    /**
     * List#__bool__() => Bool
     *
     * Boolean representation of list. Non-empty lists evaluate as true.
     */
    TEMPEARLY_NATIVE_METHOD(list_bool)
    {
        frame->SetReturnValue(Value::NewBool(!args[0].As<ListObject>()->IsEmpty()));
    }

    void init_list(Interpreter* i)
    {
        i->cList = i->AddClass("List", i->cIterable);

        i->cList->SetAllocator(list_alloc);

        i->cList->AddMethod(i, "__init__", -1, list_init);

        i->cList->AddMethod(i, "size", 0, list_size);

        i->cList->AddMethod(i, "append", -1, list_append);
        i->cList->AddMethod(i, "prepend", -1, list_prepend);
        i->cList->AddMethod(i, "clear", 0, list_clear);

        i->cList->AddMethod(i, "__iter__", 0, list_iter);

        // Operators
        i->cList->AddMethod(i, "__add__", 1, list_add);

        // Conversion methods
        i->cList->AddMethod(i, "__bool__", 0, list_bool);
        i->cList->AddMethodAlias(i, "__str__", "join");
    }
}
