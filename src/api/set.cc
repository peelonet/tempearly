#include "interpreter.h"
#include "api/iterator.h"
#include "api/set.h"

namespace tempearly
{
    SetObject::SetObject(const Handle<Class>& cls, std::size_t bucket_size)
        : Object(cls)
        , m_bucket_size(bucket_size)
        , m_bucket(Memory::Allocate<Entry*>(m_bucket_size))
        , m_size(0)
        , m_front(nullptr)
        , m_back(nullptr)
    {
        for (std::size_t i = 0; i < m_bucket_size; ++i)
        {
            m_bucket[i] = nullptr;
        }
    }

    SetObject::~SetObject()
    {
        Memory::Unallocate<Entry*>(m_bucket);
    }

    bool SetObject::Has(i64 hash) const
    {
        const std::size_t index = static_cast<std::size_t>(hash % m_bucket_size);

        for (Entry* e = m_bucket[index]; e; e = e->m_child)
        {
            if (e->m_hash == hash)
            {
                return true;
            }
        }

        return false;
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
    
    void SetObject::Add(const Handle<SetObject>& that)
    {
        if (this == that.Get())
        {
            return;
        }
        for (Entry* a = that->m_front; a; a = a->m_next)
        {
            const std::size_t index = static_cast<std::size_t>(a->m_hash % m_bucket_size);
            bool found = false;

            for (Handle<Entry> b = m_bucket[index]; b; b = b->m_child)
            {
                if (b->m_hash == a->m_hash)
                {
                    b->m_value = a->m_value;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                Handle<Entry> b = new Entry(a->m_hash, a->m_value);

                if ((b->m_prev = m_back))
                {
                    m_back->m_next = b;
                } else {
                    m_front = b;
                }
                m_back = b;
                b->m_child = m_bucket[index];
                m_bucket[index] = b;
                ++m_size;
            }
        }
    }

    bool SetObject::Remove(i64 hash)
    {
        const std::size_t index = static_cast<std::size_t>(hash % m_bucket_size);
        Entry* entry = m_bucket[index];

        if (!entry)
        {
            return false;
        }
        if (entry->m_hash == hash)
        {
            m_bucket[index] = entry->m_child;
            if (entry->m_next && entry->m_prev)
            {
                entry->m_next->m_prev = entry->m_prev;
                entry->m_prev->m_next = entry->m_next;
            }
            else if (entry->m_next)
            {
                entry->m_next->m_prev = nullptr;
                m_front = entry->m_next;
            }
            else if (entry->m_prev)
            {
                entry->m_prev->m_next = nullptr;
                m_back = entry->m_prev;
            } else {
                m_front = m_back = nullptr;
            }
            --m_size;

            return true;
        }
        for (; entry->m_child; entry = entry->m_child)
        {
            if (entry->m_child->m_hash == hash)
            {
                Entry* child = entry->m_child;

                entry->m_child = entry->m_child->m_child;
                if (child->m_next && child->m_prev)
                {
                    child->m_next->m_prev = child->m_prev;
                    child->m_prev->m_next = child->m_next;
                }
                else if (child->m_next)
                {
                    child->m_next->m_prev = nullptr;
                    m_front = child->m_next;
                }
                else if (child->m_prev)
                {
                    child->m_prev->m_next = nullptr;
                    m_back = child->m_prev;
                } else {
                    m_front = m_back = nullptr;
                }
                --m_size;

                return true;
            }
        }

        return false;
    }

    void SetObject::Clear()
    {
        for (std::size_t i = 0; i < m_bucket_size; ++i)
        {
            m_bucket[i] = nullptr;
        }
        m_front = m_back = nullptr;
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
        , m_next(nullptr)
        , m_prev(nullptr)
        , m_child(nullptr) {}

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
                return;
            }
            set->Add(hash, object);
        }
    }

    /**
     * Set#size() => Int
     *
     * Returns the number of elements stored in the set.
     */
    TEMPEARLY_NATIVE_METHOD(set_size)
    {
        frame->SetReturnValue(Value::NewInt(args[0].As<SetObject>()->GetSize()));
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
        frame->SetReturnValue(Value(iterator));
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
        frame->SetReturnValue(Value::NewInt(hash));
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
            frame->SetReturnValue(Value::NewBool(args[0].As<SetObject>()->Has(hash)));
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
                return;
            }
            set->Add(hash, object);
        }
        frame->SetReturnValue(args[0]);
    }

    /**
     * Set#remove(object)
     *
     * Removes given object from the set. Throws KeyError if object is not in
     * the set.
     */
    TEMPEARLY_NATIVE_METHOD(set_remove)
    {
        i64 hash;

        if (!args[1].GetHash(interpreter, hash))
        {
            return;
        }
        else if (!args[0].As<SetObject>()->Remove(hash))
        {
            String repr;

            if (args[1].ToString(interpreter, repr))
            {
                interpreter->Throw(interpreter->eKeyError, repr);
            }
        }
    }

    /**
     * Set#discard(object) => Bool
     *
     * Removes given object from the set if it's present. Returns a boolean
     * indicating whether the object was removed from the set or not.
     */
    TEMPEARLY_NATIVE_METHOD(set_discard)
    {
        i64 hash;

        if (args[1].GetHash(interpreter, hash))
        {
            frame->SetReturnValue(Value::NewBool(args[0].As<SetObject>()->Remove(hash)));
        }
    }

    /**
     * Set#pop() => Object
     *
     * Removes and returns an arbitrary element from the set (usually the most
     * recently added).
     *
     * Throws: KeyError - If set is empty.
     */
    TEMPEARLY_NATIVE_METHOD(set_pop)
    {
        Handle<SetObject> set = args[0].As<SetObject>();
        Handle<SetObject::Entry> entry = set->GetBack();

        if (entry && set->Remove(entry->GetHash()))
        {
            frame->SetReturnValue(entry->GetValue());
        } else {
            interpreter->Throw(interpreter->eKeyError, "Set is empty");
        }
    }

    /**
     * Set#clear()
     *
     * Removes all elements from the set.
     */
    TEMPEARLY_NATIVE_METHOD(set_clear)
    {
        args[0].As<SetObject>()->Clear();
        frame->SetReturnValue(args[0]);
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
        Value iterator;
        Value element;
        i64 hash;

        if (!args[1].CallMethod(interpreter, iterator, "__iter__"))
        {
            return;
        }
        result = new SetObject(interpreter->cSet);
        result->Add(original);
        while (iterator.GetNext(interpreter, element))
        {
            if (!element.GetHash(interpreter, hash))
            {
                return;
            }
            result->Add(hash, element);
        }
        if (!interpreter->HasException())
        {
            frame->SetReturnValue(Value(result));
        }
    }

    /**
     * Set#__sub__(collection) => Set
     *
     * Returns a copy of the set with items from given collection excluded from
     * it.
     *
     *     a = Set(1, 2, 3);
     *     a -= [1];
     *     a;               #=> {2, 3}
     */
    TEMPEARLY_NATIVE_METHOD(set_sub)
    {
        Handle<SetObject> original = args[0].As<SetObject>();
        Handle<SetObject> result;
        Value iterator;
        Value element;
        i64 hash;

        if (!args[1].CallMethod(interpreter, iterator, "__iter__"))
        {
            return;
        }
        result = new SetObject(interpreter->cSet);
        result->Add(original);
        while (iterator.GetNext(interpreter, element))
        {
            if (!element.GetHash(interpreter, hash))
            {
                return;
            }
            result->Remove(hash);
        }
        if (!interpreter->HasException())
        {
            frame->SetReturnValue(Value(result));
        }
    }

    /**
     * Set#__and__(collection) => Set
     *
     * Returns new set containing elements common to the set and given
     * collection of objects.
     */
    TEMPEARLY_NATIVE_METHOD(set_and)
    {
        Handle<SetObject> original = args[0].As<SetObject>();
        Handle<SetObject> result;
        Value iterator;
        Value element;
        i64 hash;

        if (!args[1].CallMethod(interpreter, iterator, "__iter__"))
        {
            return;
        }
        result = new SetObject(interpreter->cSet);
        while (iterator.GetNext(interpreter, element))
        {
            if (!element.GetHash(interpreter, hash))
            {
                return;
            }
            else if (original->Has(hash))
            {
                result->Add(hash, element);
            }
        }
        if (!interpreter->HasException())
        {
            frame->SetReturnValue(Value(result));
        }
    }

    /**
     * Set#__bool__() => Bool
     *
     * Boolean representation of set. Sets evaluate as true when they are not
     * empty.
     */
    TEMPEARLY_NATIVE_METHOD(set_bool)
    {
        frame->SetReturnValue(Value::NewBool(!args[0].As<SetObject>()->IsEmpty()));
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
        i->cSet->AddMethod(i, "remove", 1, set_remove);
        i->cSet->AddMethod(i, "discard", 1, set_discard);
        i->cSet->AddMethod(i, "pop", 0, set_pop);
        i->cSet->AddMethod(i, "clear", 0, set_clear);

        // Operators
        i->cSet->AddMethod(i, "__add__", 1, set_add_oper);
        i->cSet->AddMethod(i, "__sub__", 1, set_sub);
        i->cSet->AddMethod(i, "__and__", 1, set_and);
        i->cSet->AddMethodAlias(i, "__lsh__", "add");

        // Conversion methods
        i->cSet->AddMethod(i, "__bool__", 0, set_bool);
        i->cSet->AddMethodAlias(i, "__str__", "join");
    }
}
