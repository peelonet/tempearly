#include "interpreter.h"
#include "api/iterator.h"
#include "api/set.h"

namespace tempearly
{
    SetObject::SetObject(const Handle<Class>& cls, std::size_t bucket_size)
        : Object(cls)
        , m_bucket_size(bucket_size)
        , m_bucket(new Entry*[m_bucket_size])
        , m_size(0)
        , m_front(0)
        , m_back(0)
    {
        for (std::size_t i = 0; i < m_bucket_size; ++i)
        {
            m_bucket[i] = 0;
        }
    }

    SetObject::~SetObject()
    {
        delete[] m_bucket;
    }

    Handle<SetObject::Entry> SetObject::Find(i64 hash) const
    {
        const std::size_t index = static_cast<std::size_t>(hash % m_bucket_size);

        for (Entry* e = m_bucket[index]; e; e = e->m_child)
        {
            if (e->m_hash == hash)
            {
                return e;
            }
        }

        return Handle<Entry>();
    }

    void SetObject::Add(i64 hash, const Value& value)
    {
        const std::size_t index = static_cast<std::size_t>(hash % m_bucket_size);
        Handle<Entry> e;

        for (e = m_bucket[index]; e; e = e->m_child)
        {
            if (e->m_hash == hash)
            {
                e->m_value = value;
                return;
            }
        }
        e = new Entry(hash, value);
        if ((e->m_prev = m_back))
        {
            m_back->m_next = e;
        } else {
            m_front = e;
        }
        m_back = e;
        e->m_child = m_bucket[index];
        m_bucket[index] = e;
        ++m_size;
    }

    void SetObject::Clear()
    {
        for (std::size_t i = 0; i < m_bucket_size; ++i)
        {
            m_bucket[i] = 0;
        }
        m_front = m_back = 0;
        m_size = 0;
    }

    void SetObject::Mark()
    {
        Object::Mark();
        if (m_front && !m_front->IsMarked())
        {
            m_front->Mark();
        }
    }

    SetObject::Entry::Entry(i64 hash, const Value& value)
        : m_hash(hash)
        , m_value(value)
        , m_next(0)
        , m_prev(0)
        , m_child(0) {}

    void SetObject::Entry::Mark()
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
        if (m_child && !m_child->IsMarked())
        {
            m_child->Mark();
        }
    }

    static Handle<CoreObject> set_alloc(const Handle<Interpreter>& interpreter,
                                        const Handle<Class>& cls)
    {
        return new SetObject(cls);
    }

    /**
     * Set#__init__(object...)
     *
     * Initializes set by inserting objects given as arguments into it.
     */
    TEMPEARLY_NATIVE_METHOD(set_init)
    {
        Handle<SetObject> set = args[0].As<SetObject>();

        if (!set->IsEmpty())
        {
            set->Clear();
        }
        for (std::size_t i = 1; i < args.GetSize(); ++i)
        {
            const Value& object = args[i];
            i64 hash;

            if (!object.GetHash(interpreter, hash))
            {
                return Value();
            }
            set->Add(hash, object);
        }

        return Value::NullValue();
    }

    /**
     * Set#size() => Int
     *
     * Returns the number of elements stored in the set.
     */
    TEMPEARLY_NATIVE_METHOD(set_size)
    {
        return Value::NewInt(args[0].As<SetObject>()->GetSize());
    }

    namespace
    {
        class SetIterator : public IteratorObject
        {
        public:
            explicit SetIterator(const Handle<Class>& cls, const Handle<SetObject>& set)
                : IteratorObject(cls)
                , m_entry(set->GetFront()) {}

            Result Generate(const Handle<Interpreter>& interpreter)
            {
                if (m_entry)
                {
                    const Value& value = m_entry->GetValue();

                    m_entry = m_entry->GetNext();

                    return Result(Result::KIND_SUCCESS, value);
                }

                return Result(Result::KIND_BREAK);
            }

            void Mark()
            {
                IteratorObject::Mark();
                if (m_entry && !m_entry->IsMarked())
                {
                    m_entry->Mark();
                }
            }

        private:
            SetObject::Entry* m_entry;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(SetIterator);
        };
    }

    /**
     * Set#__iter__() => Iterator
     *
     * Returns an iterator which can be used to iterate each entry in the set.
     */
    TEMPEARLY_NATIVE_METHOD(set_iter)
    {
        Handle<SetObject> set = args[0].As<SetObject>();
        Handle<IteratorObject> iterator;

        if (set->IsEmpty())
        {
            iterator = interpreter->GetEmptyIterator();
        } else {
            iterator = new SetIterator(interpreter->cIterator, set);
        }

        return Value::NewObject(iterator);
    }

    /**
     * Set#__hash__() => String
     *
     * Calculates hash code for the set. Two sets with identical contents will
     * produce same hash code.
     */
    TEMPEARLY_NATIVE_METHOD(set_hash)
    {
        Handle<SetObject> set = args[0].As<SetObject>();
        i64 hash = 0;

        if (!set->HasFlag(CountedObject::FLAG_INSPECTING))
        {
            set->SetFlag(CountedObject::FLAG_INSPECTING);
            for (Handle<SetObject::Entry> entry = set->GetFront();
                 entry;
                 entry = entry->GetNext())
            {
                hash += entry->GetHash();
            }
            set->UnsetFlag(Object::FLAG_INSPECTING);
        }

        return Value::NewInt(hash);
    }

    /**
     * Set#has(object) => Bool
     *
     * Returns true if set contains given object. Objects are identified by
     * their hash codes.
     */
    TEMPEARLY_NATIVE_METHOD(set_has)
    {
        i64 hash;

        if (args[1].GetHash(interpreter, hash))
        {
            return Value::NewBool(args[0].As<SetObject>()->Find(hash));
        } else {
            return Value();
        }
    }

    /**
     * Set#add(object...)
     *
     * Inserts all objects given as arguments into the set.
     */
    TEMPEARLY_NATIVE_METHOD(set_add)
    {
        Handle<SetObject> set = args[0].As<SetObject>();

        for (std::size_t i = 1; i < args.GetSize(); ++i)
        {
            const Value& object = args[i];
            i64 hash;

            if (!object.GetHash(interpreter, hash))
            {
                return Value();
            }
            set->Add(hash, object);
        }

        return args[0];
    }

    /**
     * Set#clear()
     *
     * Removes all elements from the set.
     */
    TEMPEARLY_NATIVE_METHOD(set_clear)
    {
        args[0].As<SetObject>()->Clear();

        return args[0];
    }

    /**
     * Set#__add__(collection) => Set
     *
     * Returns a new set which contains items from the list with items included
     * from the object given as argument.
     *
     *     a = Set(1, 2, 3);
     *     a += [4, 5];
     *     a;               #=> {1, 2, 3, 4, 5}
     */
    TEMPEARLY_NATIVE_METHOD(set_add_oper)
    {
        Handle<SetObject> original = args[0].As<SetObject>();
        Handle<SetObject> result;
        Value iterator = args[1].Call(interpreter, "__iter__");
        Value element;
        i64 hash;

        if (!iterator)
        {
            return Value();
        }
        result = new SetObject(interpreter->cSet, original);
        while (iterator.GetNext(interpreter, element))
        {
            if (!element.GetHash(interpreter, hash))
            {
                return Value();
            }
            result->Add(hash, element);
        }
        if (interpreter->HasException())
        {
            return Value();
        }

        return Value::NewObject(result);
    }

    /**
     * Set#__bool__() => Bool
     *
     * Boolean representation of set. Sets evaluate as true when they are not
     * empty.
     */
    TEMPEARLY_NATIVE_METHOD(set_bool)
    {
        return Value::NewBool(!args[0].As<SetObject>()->IsEmpty());
    }

    void init_set(Interpreter* i)
    {
        i->cSet = i->AddClass("Set", i->cIterable);

        i->cSet->SetAllocator(set_alloc);

        i->cSet->AddMethod(i, "__init__", -1, set_init);

        i->cSet->AddMethod(i, "size", 0, set_size);

        i->cSet->AddMethod(i, "__iter__", 0, set_iter);
        i->cSet->AddMethod(i, "__hash__", 0, set_hash);

        i->cSet->AddMethod(i, "has", 1, set_has);
        i->cSet->AddMethod(i, "add", -1, set_add);
        i->cSet->AddMethod(i, "clear", 0, set_clear);

        // Operators
        i->cSet->AddMethod(i, "__add__", 1, set_add_oper);
        i->cSet->AddMethod(i, "__lsh__", 1, set_add);

        // Conversion methods
        i->cSet->AddMethod(i, "__bool__", 0, set_bool);
        i->cSet->AddMethodAlias(i, "__str__", "join");
    }
}
