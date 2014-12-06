#include "interpreter.h"
#include "api/iterator.h"
#include "api/list.h"
#include "api/range.h"

namespace tempearly
{
    ListObject::ListObject(const Handle<Class>& cls)
        : Object(cls)
        , m_size(0)
        , m_front(0)
        , m_back(0) {}

    Handle<ListObject::Link> ListObject::At(std::size_t index) const
    {
        for (Handle<Link> link = m_front; link; link = link->m_next)
        {
            if (!index--)
            {
                return link;
            }
        }

        return Handle<Link>();
    }

    void ListObject::Append(const Value& value)
    {
        Handle<Link> link = new Link(value);

        if ((link->m_prev = m_back))
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

                link->m_prev = back;
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

            if ((b->m_prev = m_back))
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

        if ((link->m_next = m_front))
        {
            m_front->m_prev = link;
        } else {
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

                link->m_prev = back;
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

    void ListObject::Insert(std::size_t index, const Value& value)
    {
        Handle<Link> position = At(index);

        if (position)
        {
            Handle<Link> link = new Link(value);

            link->m_prev = position;
            if (position->m_next)
            {
                position->m_next->m_prev = link;
            } else {
                m_back = link;
            }
            position->m_next = link;
            ++m_size;
        } else {
            Append(value);
        }
    }

    void ListObject::Erase(std::size_t index)
    {
        Handle<Link> link = At(index);

        if (!link)
        {
            return;
        }
        if (link->m_next && link->m_prev)
        {
            link->m_next->m_prev = link->m_prev;
            link->m_prev->m_next = link->m_next;
        }
        else if (link->m_next)
        {
            link->m_next->m_prev = 0;
            m_front = link->m_next;
        }
        else if (link->m_prev)
        {
            link->m_prev->m_next = 0;
            m_back = link->m_prev;
        } else {
            m_front = m_back = 0;
        }
        --m_size;
    }

    bool ListObject::Erase(std::size_t index, Value& slot)
    {
        Handle<Link> link = At(index);

        if (!link)
        {
            return false;
        }
        if (link->m_next && link->m_prev)
        {
            link->m_next->m_prev = link->m_prev;
            link->m_prev->m_next = link->m_next;
        }
        else if (link->m_next)
        {
            link->m_next->m_prev = 0;
            m_front = link->m_next;
        }
        else if (link->m_prev)
        {
            link->m_prev->m_next = 0;
            m_back = link->m_prev;
        } else {
            m_front = m_back = 0;
        }
        --m_size;
        slot = link->m_value;

        return true;
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
        , m_next(0)
        , m_prev(0) {}

    void ListObject::Link::Mark()
    {
        CountedObject::Mark();
        m_value.Mark();
        if (m_next && !m_next->IsMarked())
        {
            m_next->Mark();
        }
        if (m_prev && !m_prev->IsMarked())
        {
            m_prev->Mark();
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
     * List#insert(index, object)
     *
     * Inserts an value at given position.
     */
    TEMPEARLY_NATIVE_METHOD(list_insert)
    {
        Handle<ListObject> list = args[0].As<ListObject>();
        i64 index;

        if (!args[1].AsInt(interpreter, index))
        {
            return;
        }
        if (index < 0)
        {
            index += list->GetSize();
        }
        list->Insert(static_cast<std::size_t>(index), args[2]);
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

    /**
     * List#index(object) => Int
     *
     * Returns index of element from the list which value is equal to value
     * given as argument. If no element is found, an ValueError is thrown
     * instead.
     */
    TEMPEARLY_NATIVE_METHOD(list_index)
    {
        const Value& needle = args[1];
        bool result;
        i64 index = 0;

        for (Handle<ListObject::Link> link = args[0].As<ListObject>()->GetFront(); link; link = link->GetNext())
        {
            if (!link->GetValue().Equals(interpreter, needle, result))
            {
                return;
            }
            else if (result)
            {
                frame->SetReturnValue(Value::NewInt(index));
                return;
            }
            ++index;
        }
        interpreter->Throw(interpreter->eValueError, "Value is not in the list");
    }

    /**
     * List#remove(object)
     *
     * Removes the first element from the list which value is equal to value
     * given as argument. If no element is found, an ValueError is thrown
     * instead.
     */
    TEMPEARLY_NATIVE_METHOD(list_remove)
    {
        Handle<ListObject> list = args[0].As<ListObject>();
        const Value& needle = args[1];
        bool result;
        std::size_t index = 0;

        for (Handle<ListObject::Link> link = list->GetFront(); link; link = link->GetNext())
        {
            if (!link->GetValue().Equals(interpreter, needle, result))
            {
                return;
            }
            else if (result)
            {
                list->Erase(index);
                return;
            }
            ++index;
        }
        interpreter->Throw(interpreter->eValueError, "Value is not in the list");
    }

    /**
     * List#pop(index) => Object
     *
     * Removes element from given position in the list, and returns it's value.
     * If no index is specified, the last element from the list is removed and
     * it's value is returned.
     *
     * If index is out of bounds, IndexError is thrown instead.
     */
    TEMPEARLY_NATIVE_METHOD(list_pop)
    {
        Handle<ListObject> list = args[0].As<ListObject>();
        Value value;

        if (args.GetSize() > 1)
        {
            i64 index;

            if (!args[1].AsInt(interpreter, index))
            {
                return;
            }
            if (index < 0)
            {
                index += list->GetSize();
            }
            if (!list->Erase(static_cast<std::size_t>(index), value))
            {
                interpreter->Throw(interpreter->eIndexError, "List index out of bounds");
                return;
            }
        } else {
            Handle<ListObject::Link> link = list->GetBack();

            if (!link)
            {
                interpreter->Throw(interpreter->eIndexError, "List is empty");
                return;
            }
            value = link->GetValue();
            list->Erase(list->GetSize() - 1);
        }
        frame->SetReturnValue(value);
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
     * List#__mul__(n) => List
     *
     * Repeats list n times.
     *
     *     [1, 2] * 3;  #=> [1, 2, 1, 2, 1, 2]
     */
    TEMPEARLY_NATIVE_METHOD(list_mul)
    {
        Handle<ListObject> original = args[0].As<ListObject>();
        Handle<ListObject> result;
        i64 n;

        if (!args[1].AsInt(interpreter, n))
        {
            return;
        }
        result = new ListObject(interpreter->cList);
        while (n-- > 0)
        {
            result->Append(original);
        }
        frame->SetReturnValue(Value(result));
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

    /**
     * List#__getitem__(index) => Object
     *
     * List element retrieval. Two types of indexes are supported.
     *
     * If an number is given as an index, an element from that specified index
     * is returned. If index is out of bounds, an IndexError is thrown instead.
     *
     * If a range object is given as an index, it's beginning and ending values
     * are used as slice indexes which slice a portion of the list and returns
     * it as a new list.
     *
     * Negative indexes count backwards, for example -1 is index of the last
     * element in the list.
     */
    TEMPEARLY_NATIVE_METHOD(list_getitem)
    {
        Handle<ListObject> list = args[0].As<ListObject>();
        Handle<ListObject::Link> link;

        if (args[1].IsRange())
        {
            Handle<ListObject> result;
            Handle<RangeObject> range = args[1].As<RangeObject>();
            i64 begin;
            i64 end;

            if (!range->GetBegin().AsInt(interpreter, begin) || !range->GetEnd().AsInt(interpreter, end))
            {
                return;
            }
            if (range->IsExclusive())
            {
                --end;
            }
            if (begin < 0)
            {
                begin += list->GetSize();
            }
            if (end < 0)
            {
                end += list->GetSize();
            }
            result = new ListObject(interpreter->cList);
            for (link = list->At(begin); link && begin < end; link = link->GetNext(), ++begin)
            {
                result->Append(link->GetValue());
            }
            frame->SetReturnValue(Value(result));
        } else {
            i64 index;

            if (!args[1].AsInt(interpreter, index))
            {
                return;
            }
            if (index < 0)
            {
                index += list->GetSize();
            }
            if ((link = list->At(static_cast<std::size_t>(index))))
            {
                frame->SetReturnValue(link->GetValue());
            } else {
                interpreter->Throw(interpreter->eIndexError, "List index out of bounds");
            }
        }
    }

    /**
     * List#__setitem__(index, object)
     *
     * Element assignment. Replaces value from given index with given object.
     * Negative indexes count backwards. If index is out of bounds, IndexError
     * is thrown instead.
     */
    TEMPEARLY_NATIVE_METHOD(list_setitem)
    {
        Handle<ListObject> list = args[0].As<ListObject>();
        Handle<ListObject::Link> link;
        i64 index;

        if (!args[1].AsInt(interpreter, index))
        {
            return;
        }
        if (index < 0)
        {
            index += list->GetSize();
        }
        if ((link = list->At(static_cast<std::size_t>(index))))
        {
            link->SetValue(args[2]);
        } else {
            interpreter->Throw(interpreter->eIndexError, "List index out of bounds");
        }
    }

    void init_list(Interpreter* i)
    {
        i->cList = i->AddClass("List", i->cIterable);

        i->cList->SetAllocator(list_alloc);

        i->cList->AddMethod(i, "__init__", -1, list_init);

        i->cList->AddMethod(i, "size", 0, list_size);

        i->cList->AddMethod(i, "append", -1, list_append);
        i->cList->AddMethod(i, "prepend", -1, list_prepend);
        i->cList->AddMethod(i, "insert", 2, list_insert);
        i->cList->AddMethod(i, "clear", 0, list_clear);
        i->cList->AddMethod(i, "index", 1, list_index);
        i->cList->AddMethod(i, "remove", 1, list_remove);
        i->cList->AddMethod(i, "pop", -1, list_pop);

        i->cList->AddMethod(i, "__iter__", 0, list_iter);

        // Operators
        i->cList->AddMethod(i, "__add__", 1, list_add);
        i->cList->AddMethod(i, "__mul__", 1, list_mul);

        // Conversion methods
        i->cList->AddMethod(i, "__bool__", 0, list_bool);
        i->cList->AddMethodAlias(i, "__str__", "join");

        i->cList->AddMethod(i, "__getitem__", 1, list_getitem);
        i->cList->AddMethod(i, "__setitem__", 2, list_setitem);
    }
}
