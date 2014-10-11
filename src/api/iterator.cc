#include "interpreter.h"
#include "api/iterator.h"

namespace tempearly
{
    IteratorObject::IteratorObject(const Handle<Class>& cls)
        : Object(cls) {}

    Value IteratorObject::Peek(const Handle<Interpreter>& interpreter)
    {
        if (m_pushback.empty())
        {
            Result result = Generate(interpreter);

            switch (result.GetKind())
            {
                case Result::KIND_SUCCESS:
                    if (result.HasValue())
                    {
                        m_pushback.push_back(result.GetValue());
                    } else {
                        m_pushback.push_back(Value::NullValue());
                    }
                    break;

                case Result::KIND_BREAK:
                    if (!interpreter->HasException())
                    {
                        interpreter->Throw(interpreter->eStopIteration,
                                           "Iteration reached end");
                    }

                default:
                    return Value();
            }
        }

        return m_pushback.back();
    }

    Value IteratorObject::Next(const Handle<Interpreter>& interpreter)
    {
        if (m_pushback.empty())
        {
            Result result = Generate(interpreter);

            switch (result.GetKind())
            {
                case Result::KIND_SUCCESS:
                    if (result.HasValue())
                    {
                        return result.GetValue();
                    } else {
                        return Value::NullValue();
                    }

                case Result::KIND_BREAK:
                    if (!interpreter->HasException())
                    {
                        interpreter->Throw(interpreter->eStopIteration,
                                           "Iteration reached end");
                    }

                default:
                    return Value();
            }
        } else {
            Value value = m_pushback.back();

            m_pushback.pop_back();

            return value;
        }
    }

    void IteratorObject::Feed(const Value& value)
    {
        if (value)
        {
            m_pushback.push_back(value);
        }
    }

    void IteratorObject::Mark()
    {
        Object::Mark();
        for (std::size_t i = 0; i < m_pushback.size(); ++i)
        {
            m_pushback[i].Mark();
        }
    }

    /**
     * Iterator#next() => Object
     *
     * Returns next object from the iteration or throws StopIteration exception
     * if there are no more elements to iterate.
     *
     *     i = [1, 2, 3].iterator();
     *     3.times()  {
     *       i.next();
     *     };
     *     i.next(); # Throws StopIteration
     */
    TEMPEARLY_NATIVE_METHOD(iter_next)
    {
        return args[0].As<IteratorObject>()->Next(interpreter);
    }

    /**
     * Iterator#feed(object...)
     *
     * Provides pushback functionality for the iterator. Objects can be pushed
     * back to the iterator and they are returned in LILO(Last In, Last Out)
     * order.
     *
     *     i = [1, 2, 3].__iter__();
     *     i.next();    #=> 1
     *     i.feed(1);
     *     i.next();    #=> 1
     *     i.next();    #=> 2
     */
    TEMPEARLY_NATIVE_METHOD(iter_feed)
    {
        Handle<IteratorObject> iterator = args[0].As<IteratorObject>();

        for (std::size_t i = 1; i < args.size(); ++i)
        {
            iterator->Feed(args[i]);
        }

        return args[0];
    }

    /**
     * Iterator#peek() => Object
     *
     * Returns next element from iteration without moving iterator forwards,
     * or throws an StopIteration exception if there are no more elements to
     * iterate.
     *
     *     i = [1, 2, 3].__iter__();
     *     i.peek();    #=> 1
     *     i.next();    #=> 1
     *     i.next();    #=> 2
     */
    TEMPEARLY_NATIVE_METHOD(iter_peek)
    {
        return args[0].As<IteratorObject>()->Peek(interpreter);
    }

    /**
     * Iterator#__iter__() => Iterator
     *
     * Returns the iterator object itself.
     */
    TEMPEARLY_NATIVE_METHOD(iter_iter)
    {
        return args[0];
    }

    /**
     * Iterator#__bool__() => Bool
     *
     * Boolean representation of iterator. Iterators evaluate as true when they
     * still have uniterated elements.
     */
    TEMPEARLY_NATIVE_METHOD(iter_bool)
    {
        if (args[0].As<IteratorObject>()->Peek(interpreter))
        {
            return Value::NewBool(true);
        }
        else if (interpreter->GetException()->IsInstance(interpreter, interpreter->eStopIteration))
        {
            interpreter->ClearException();

            return Value::NewBool(false);
        } else {
            return Value();
        }
    }

    void init_iterator(Interpreter* i)
    {
        i->cIterator = i->AddClass("Iterator", i->cObject);

        i->cIterator->SetAllocator(Class::kNoAlloc);

        i->cIterator->AddMethod(i, "__init__", -1, iter_feed);

        i->cIterator->AddMethod(i, "next", 0, iter_next);
        i->cIterator->AddMethod(i, "feed", -1, iter_feed);
        i->cIterator->AddMethod(i, "peek", 0, iter_peek);

        i->cIterator->AddMethod(i, "__iter__", 0, iter_iter);

        i->cIterator->AddMethod(i, "__bool__", 0, iter_bool);

        //TODO:i->cIterator->AddAlias(i, "__lsh__", "feed");
        //TODO:i->cIterator->AddAlias(i, "__call__", "next");
    }
}
