#include "interpreter.h"
#include "api/iterator.h"
#include "api/range.h"
#include "core/stringbuilder.h"

namespace tempearly
{
    RangeObject::RangeObject(const Handle<Interpreter>& interpreter,
                             const Value& begin,
                             const Value& end,
                             bool exclusive)
        : Object(interpreter->cRange)
        , m_begin(begin)
        , m_end(end)
        , m_exclusive(exclusive) {}

    void RangeObject::Mark()
    {
        Object::Mark();
        m_begin.Mark();
        m_end.Mark();
    }

    /**
     * Range.__call__(begin, end, exclusive = false) => Range
     *
     * Constructs new range object from objects given as arguments.
     */
    TEMPEARLY_NATIVE_METHOD(range_s_call)
    {
        const Value& begin = args[0];
        const Value& end = args[1];
        bool exclusive = false;

        if (args.size() > 2)
        {
            if (!args[2].AsBool(interpreter, exclusive))
            {
                return Value();
            }
        }

        return Value::NewObject(new RangeObject(interpreter, begin, end, exclusive));
    }

    /**
     * Range#begin() => Object
     *
     * Returns first value of the range.
     *
     *     (1..4).begin()   #=> 1
     */
    TEMPEARLY_NATIVE_METHOD(range_begin)
    {
        return args[0].As<RangeObject>()->GetBegin();
    }

    /**
     * Range#end() => Object
     *
     * Returns last value of the range.
     *
     *     (1..4).end()     #=> 4
     */
    TEMPEARLY_NATIVE_METHOD(range_end)
    {
        return args[0].As<RangeObject>()->GetEnd();
    }

    /**
     * Range#is_exclusive() => Bool
     *
     * Returns true if range excludes it's end value.
     *
     *     (1..4).is_exclusive()  #=> false
     *     (1...4).is_exclusive() #=> true
     */
    TEMPEARLY_NATIVE_METHOD(range_is_exclusive)
    {
        return Value::NewBool(args[0].As<RangeObject>()->IsExclusive());
    }

    namespace
    {
        class IntRangeIterator : public IteratorObject
        {
        public:
            explicit IntRangeIterator(const Handle<Interpreter>& interpreter,
                                      i64 begin,
                                      i64 end,
                                      bool exclusive)
                : IteratorObject(interpreter->cIterator)
                , m_current(begin)
                , m_end(end)
                , m_exclusive(exclusive) {}

            Result Generate(const Handle<Interpreter>& interpreter)
            {
                if (m_current < m_end || (!m_exclusive && m_current == m_end))
                {
                    return Result(Result::KIND_SUCCESS, Value::NewInt(m_current++));
                } else {
                    return Result(Result::KIND_BREAK);
                }
            }

        private:
            i64 m_current;
            const i64 m_end;
            const bool m_exclusive;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(IntRangeIterator);
        };

        class RangeIterator : public IteratorObject
        {
        public:
            explicit RangeIterator(const Handle<Interpreter>& interpreter,
                                   const Value& begin,
                                   const Value& end,
                                   bool exclusive)
                : IteratorObject(interpreter->cIterator)
                , m_current(begin)
                , m_end(end)
                , m_exclusive(exclusive) {}

            Result Generate(const Handle<Interpreter>& interpreter)
            {
                Value cmp;
                bool b;

                if (!m_current)
                {
                    return Result(Result::KIND_BREAK);
                }
                else if (!(cmp = m_current.Call(interpreter, "__lt__", m_end))
                            || !cmp.AsBool(interpreter, b))
                {
                    return Result(Result::KIND_ERROR);
                }
                else if (b)
                {
                    Value current = m_current;
                    Value next = m_current.Call(interpreter, "__inc__");

                    if (!next)
                    {
                        return Result(Result::KIND_ERROR);
                    }
                    m_current = next;

                    return Result(Result::KIND_SUCCESS, current);
                }
                m_current.Clear();
                if (m_exclusive)
                {
                    return Result(Result::KIND_BREAK);
                } else {
                    return Result(Result::KIND_SUCCESS, m_end);
                }
            }

            void Mark()
            {
                IteratorObject::Mark();
                m_current.Mark();
                m_end.Mark();
            }

        private:
            Value m_current;
            const Value m_end;
            const bool m_exclusive;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(RangeIterator);
        };
    }

    /**
     * Range#__iter__() => Iterator
     *
     * Returns an iterator which can be used to traverse each value in the
     * range.
     */
    TEMPEARLY_NATIVE_METHOD(range_iter)
    {
        Handle<RangeObject> range = args[0].As<RangeObject>();
        const Value& begin = range->GetBegin();
        const Value& end = range->GetEnd();
        Handle<IteratorObject> iterator;

        if (begin.IsInt() && end.IsInt())
        {
            iterator = new IntRangeIterator(interpreter,
                                            begin.AsInt(),
                                            end.AsInt(),
                                            range->IsExclusive());
        } else {
            iterator = new RangeIterator(interpreter,
                                         begin,
                                         end,
                                         range->IsExclusive());
        }

        return Value::NewObject(iterator);
    }

    /**
     * Range#__str__() => String
     *
     * Returns textual presentation of the range.
     */
    TEMPEARLY_NATIVE_METHOD(range_str)
    {
        Handle<RangeObject> range = args[0].As<RangeObject>();
        StringBuilder buffer;

        if (!range->HasFlag(CountedObject::FLAG_INSPECTING))
        {
            String string;

            range->SetFlag(CountedObject::FLAG_INSPECTING);
            if (!range->GetBegin().ToString(interpreter, string))
            {
                range->UnsetFlag(CountedObject::FLAG_INSPECTING);

                return Value();
            }
            buffer << string << '.' << '.';
            if (range->IsExclusive())
            {
                buffer << '.';
            }
            if (!range->GetEnd().ToString(interpreter, string))
            {
                range->UnsetFlag(CountedObject::FLAG_INSPECTING);

                return Value();
            }
            range->UnsetFlag(CountedObject::FLAG_INSPECTING);
            buffer << string;
        }

        return Value::NewString(buffer.ToString());
    }

    void init_range(Interpreter* i)
    {
        i->cRange = i->AddClass("Range", i->cIterable);

        i->cRange->SetAllocator(Class::kNoAlloc);

        i->cRange->AddStaticMethod(i, "__call__", -3, range_s_call);

        i->cRange->AddMethod(i, "begin", 0, range_begin);
        i->cRange->AddMethod(i, "end", 0, range_end);
        i->cRange->AddMethod(i, "is_exclusive", 0, range_is_exclusive);

        i->cRange->AddMethod(i, "__iter__", 0, range_iter);

        // Conversion methods
        i->cRange->AddMethod(i, "__str__", 0, range_str);
    }
}
