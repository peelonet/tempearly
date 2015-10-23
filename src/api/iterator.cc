#include "interpreter.h"
#include "api/iterator.h"

namespace tempearly
{
    IteratorObject::IteratorObject(const Handle<Class>& cls)
        : CustomObject(cls) {}

    bool IteratorObject::Peek(const Handle<Interpreter>& interpreter)
    {
        if (m_pushback.IsEmpty())
        {
            const Result result = Generate(interpreter);

            switch (result.GetKind())
            {
                case Result::KIND_SUCCESS:
                    m_pushback.PushBack(result.GetValue());
                    return true;

                case Result::KIND_BREAK:
                    if (!interpreter->HasException())
                    {
                        interpreter->Throw(
                            interpreter->eStopIteration,
                            "Iteration reached end"
                        );
                    }

                default:
                    return false;
            }
        }

        return true;
    }

    bool IteratorObject::Peek(const Handle<Interpreter>& interpreter,
                              Handle<Object>& slot)
    {
        if (m_pushback.IsEmpty())
        {
            const Result result = Generate(interpreter);

            switch (result.GetKind())
            {
                case Result::KIND_SUCCESS:
                    m_pushback.PushBack(result.GetValue());
                    slot = result.GetValue();
                    return true;

                case Result::KIND_BREAK:
                    if (!interpreter->HasException())
                    {
                        interpreter->Throw(
                            interpreter->eStopIteration,
                            "Iteration reached end"
                        );
                    }

                default:
                    return false;
            }
        }
        slot = m_pushback.GetBack();

        return true;
    }

    bool IteratorObject::Next(const Handle<Interpreter>& interpreter,
                              Handle<Object>& slot)
    {
        if (m_pushback.IsEmpty())
        {
            Result result = Generate(interpreter);

            switch (result.GetKind())
            {
                case Result::KIND_SUCCESS:
                    slot = result.GetValue();
                    return true;

                case Result::KIND_BREAK:
                    if (!interpreter->HasException())
                    {
                        interpreter->Throw(interpreter->eStopIteration,
                                           "Iteration reached end");
                    }

                default:
                    return false;
            }
        } else {
            slot = m_pushback.GetBack();
            m_pushback.Erase(m_pushback.GetSize() - 1);

            return true;
        }
    }

    void IteratorObject::Feed(const Handle<Object>& value)
    {
        m_pushback.PushBack(value);
    }

    void IteratorObject::Mark()
    {
        Object::Mark();
        for (std::size_t i = 0; i < m_pushback.GetSize(); ++i)
        {
            if (!m_pushback[i]->IsMarked())
            {
                m_pushback[i]->Mark();
            }
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
        Handle<Object> slot;

        if (args[0].As<IteratorObject>()->Next(interpreter, slot))
        {
            frame->SetReturnValue(slot);
        }
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

        for (std::size_t i = 1; i < args.GetSize(); ++i)
        {
            iterator->Feed(args[i]);
        }
        frame->SetReturnValue(args[0]);
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
        Handle<Object> result;

        if (args[0].As<IteratorObject>()->Peek(interpreter, result))
        {
            frame->SetReturnValue(result);
        }
    }

    /**
     * Iterator#__iter__() => Iterator
     *
     * Returns the iterator object itself.
     */
    TEMPEARLY_NATIVE_METHOD(iter_iter)
    {
        frame->SetReturnValue(args[0]);
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
            frame->SetReturnValue(Object::NewBool(true));
        }
        else if (interpreter->HasException(interpreter->eStopIteration))
        {
            interpreter->ClearException();
            frame->SetReturnValue(Object::NewBool(false));
        }
    }

    void init_iterator(Interpreter* i)
    {
        i->cIterator = i->AddClass("Iterator", i->cIterable);

        i->cIterator->SetAllocator(Class::kNoAlloc);

        i->cIterator->AddMethod(i, "__init__", -1, iter_feed);

        i->cIterator->AddMethod(i, "next", 0, iter_next);
        i->cIterator->AddMethod(i, "feed", -1, iter_feed);
        i->cIterator->AddMethod(i, "peek", 0, iter_peek);

        i->cIterator->AddMethod(i, "__iter__", 0, iter_iter);

        i->cIterator->AddMethod(i, "__bool__", 0, iter_bool);

        i->cIterator->AddMethodAlias(i, "__lsh__", "feed");
        i->cIterator->AddMethodAlias(i, "__call__", "next");
    }
}
