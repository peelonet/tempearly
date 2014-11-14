#include "interpreter.h"
#include "api/iterator.h"
#include "api/list.h"
#include "api/map.h"
#include "api/set.h"
#include "core/stringbuilder.h"

namespace tempearly
{
    MapObject::MapObject(const Handle<Class>& cls, std::size_t bucket_size)
        : Object(cls)
        , m_bucket_size(bucket_size)
        , m_bucket(new Entry*[m_bucket_size])
        , m_front(0)
        , m_back(0)
        , m_size(0)
    {
        for (std::size_t i = 0; i < m_bucket_size; ++i)
        {
            m_bucket[i] = 0;
        }
    }

    MapObject::~MapObject()
    {
        delete[] m_bucket;
    }

    Value MapObject::Find(i64 hash) const
    {
        std::size_t index = static_cast<std::size_t>(hash % m_bucket_size);

        for (Entry* entry = m_bucket[index]; entry; entry = entry->m_child)
        {
            if (entry->m_hash == hash)
            {
                return entry->m_value;
            }
        }

        return Value();
    }

    void MapObject::Insert(i64 hash, const Value& key, const Value& value)
    {
        std::size_t index = static_cast<std::size_t>(hash % m_bucket_size);
        Handle<Entry> entry;

        for (entry = m_bucket[index]; entry; entry = entry->m_child)
        {
            if (entry->m_hash == hash)
            {
                entry->m_key = key;
                entry->m_value = value;
                return;
            }
        }
        entry = new Entry(hash, key, value);
        if ((entry->m_prev = m_back))
        {
            m_back->m_next = entry;
        } else {
            m_front = entry;
        }
        m_back = entry;
        entry->m_child = m_bucket[index];
        m_bucket[index] = entry;
        ++m_size;
    }

    void MapObject::Clear()
    {
        for (std::size_t i = 0; i < m_bucket_size; ++i)
        {
            m_bucket[i] = 0;
        }
        m_front = m_back = 0;
        m_size = 0;
    }

    void MapObject::Mark()
    {
        Object::Mark();
        if (m_front && !m_front->IsMarked())
        {
            m_front->Mark();
        }
    }

    MapObject::Entry::Entry(i64 hash, const Value& key, const Value& value)
        : m_hash(hash)
        , m_key(key)
        , m_value(value)
        , m_next(0)
        , m_prev(0)
        , m_child(0) {}

    void MapObject::Entry::Mark()
    {
        CountedObject::Mark();
        m_key.Mark();
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

    static Handle<CoreObject> map_alloc(const Handle<Interpreter>& interpreter,
                                        const Handle<Class>& cls)
    {
        return new MapObject(cls);
    }

    /**
     * Map#size() => Int
     *
     * Returns the number of key-value entries stored in the map.
     */
    TEMPEARLY_NATIVE_METHOD(map_size)
    {
        return Value::NewInt(args[0].As<MapObject>()->GetSize());
    }

    /**
     * Map#keys() => Set
     *
     * Returns each key from the map in a set.
     *
     *     [foo: "bar"].keys()  #=> {"foo"}
     */
    TEMPEARLY_NATIVE_METHOD(map_keys)
    {
        Handle<MapObject> map = args[0].As<MapObject>();
        Handle<SetObject> set = new SetObject(interpreter->cSet);

        for (Handle<MapObject::Entry> e = map->GetFront(); e; e = e->GetNext())
        {
            set->Add(e->GetHash(), e->GetKey());
        }

        return Value(set);
    }

    /**
     * Map#values() => List
     *
     * Returns each value from the map in a list.
     *
     *     [foo: "bar"].values() #=> {"bar"}
     */
    TEMPEARLY_NATIVE_METHOD(map_values)
    {
        Handle<MapObject> map = args[0].As<MapObject>();
        Handle<ListObject> list = new ListObject(interpreter->cList);

        for (Handle<MapObject::Entry> e = map->GetFront(); e; e = e->GetNext())
        {
            list->Append(e->GetValue());
        }

        return Value(list);
    }

    /**
     * Map#has(object) => Bool
     *
     * Returns true if map has a value for the given key.
     */
    TEMPEARLY_NATIVE_METHOD(map_has)
    {
        i64 hash;

        if (args[1].GetHash(interpreter, hash))
        {
            return Value::NewBool(args[0].As<MapObject>()->Find(hash));
        } else {
            return Value();
        }
    }

    /**
     * Map#get(key, default_value = null) => Object
     *
     * Searches value for the given key from the map and returns it if such
     * exist. Otherwise default value given as optional argument is returned
     * instead.
     */
    TEMPEARLY_NATIVE_METHOD(map_get)
    {
        Handle<MapObject::Entry> entry;
        i64 hash;

        if (args[1].GetHash(interpreter, hash))
        {
            Value value = args[0].As<MapObject>()->Find(hash);

            if (value)
            {
                return value;
            }
            else if (args.GetSize() > 2)
            {
                return args[2];
            } else {
                return Value::NullValue();
            }
        } else {
            return Value();
        }
    }

    /**
     * Map#clear()
     *
     * Removes all entries from the map.
     */
    TEMPEARLY_NATIVE_METHOD(map_clear)
    {
        args[0].As<MapObject>()->Clear();

        return args[0];
    }

    /**
     * Map#join(separator1 = ": ", separator2 = ", ") => String
     *
     * Creates a string where each key-value pair is separated by +separator1+
     * and each value is separated from key by +separator2+.
     *
     *     map = [1: 2, 2: 3];
     *     println(map.join(" => "));
     *
     * Produces:
     *     1 => 2, 2 => 3
     */
    TEMPEARLY_NATIVE_METHOD(map_join)
    {
        Handle<MapObject> map = args[0].As<MapObject>();
        StringBuilder buffer;
        
        if (!map->HasFlag(CountedObject::FLAG_INSPECTING))
        {
            String sep1;
            String sep2;

            if (args.GetSize() > 1)
            {
                if (args[1].IsNull())
                {
                    sep1 = ": ";
                }
                else if (!args[1].AsString(interpreter, sep1))
                {
                    return Value();
                }
                if (args.GetSize() > 2)
                {
                    if (args[2].IsNull())
                    {
                        sep2 = ", ";
                    }
                    else if (!args[2].AsString(interpreter, sep2))
                    {
                        return Value();
                    }
                } else {
                    sep2 = ", ";
                }
            } else {
                sep1 = ": ";
                sep2 = ", ";
            }
            map->SetFlag(CountedObject::FLAG_INSPECTING);
            for (Handle<MapObject::Entry> entry = map->GetFront(); entry; entry = entry->GetNext())
            {
                String string;

                if (entry != map->GetFront())
                {
                    buffer.Append(sep2);
                }
                if (!entry->GetKey().ToString(interpreter, string))
                {
                    map->UnsetFlag(CountedObject::FLAG_INSPECTING);

                    return Value();
                }
                buffer << string << sep1;
                if (!entry->GetValue().ToString(interpreter, string))
                {
                    map->UnsetFlag(CountedObject::FLAG_INSPECTING);

                    return Value();
                }
                buffer.Append(string);
            }
            map->UnsetFlag(CountedObject::FLAG_INSPECTING);
        }

        return Value::NewString(buffer.ToString());
    }

    /**
     * Map#update(other_map)
     *
     * Copies entries from another map into this map.
     */
    TEMPEARLY_NATIVE_METHOD(map_update)
    {
        Handle<MapObject> map = args[0].As<MapObject>();
        Handle<MapObject> other;

        if (!args[1].IsMap())
        {
            interpreter->Throw(interpreter->eValueError, "Map required");

            return Value();
        }
        other = args[1].As<MapObject>();
        for (Handle<MapObject::Entry> entry = other->GetFront(); entry; entry = entry->GetNext())
        {
            map->Insert(entry->GetHash(), entry->GetKey(), entry->GetValue());
        }

        return args[0];
    }

    namespace
    {
        class MapIterator : public IteratorObject
        {
        public:
            explicit MapIterator(const Handle<Interpreter>& interpreter,
                                 const Handle<MapObject>& map)
                : IteratorObject(interpreter->cIterator)
                , m_entry(map->GetFront()) {}

            Result Generate(const Handle<Interpreter>& interpreter)
            {
                if (m_entry)
                {
                    Handle<MapObject::Entry> entry = m_entry;
                    Handle<ListObject> list = new ListObject(interpreter->cList);

                    m_entry = m_entry->GetNext();
                    list->Append(entry->GetKey());
                    list->Append(entry->GetValue());

                    return Result(Result::KIND_SUCCESS, Value(list));
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
            MapObject::Entry* m_entry;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(MapIterator);
        };
    }

    /**
     * Map#__iter__() => Iterator
     *
     * Returns an iterator which traverses through key-value entries in the
     * map, returning each one of them in a list of two objects.
     *
     *     m = [a: 100, b: 200];
     *     i = m.__iter_();
     *     i.next();            #=> ["a", 100]
     */
    TEMPEARLY_NATIVE_METHOD(map_iter)
    {
        Handle<MapObject> map = args[0].As<MapObject>();
        Handle<IteratorObject> iterator;

        if (map->IsEmpty())
        {
            iterator = interpreter->GetEmptyIterator();
        } else {
            iterator = new MapIterator(interpreter, map);
        }

        return Value(iterator);
    }

    /**
     * Map#__getitem__(key) => Object
     *
     * Searches value for the given key from the map and returns it if such
     * exist. Otherwise method __missing__ is executed and it's value is
     * returned, which by default throws instance of KeyError.
     */
    TEMPEARLY_NATIVE_METHOD(map_getitem)
    {
        Handle<MapObject::Entry> entry;
        i64 hash;

        if (args[1].GetHash(interpreter, hash))
        {
            Value value = args[0].As<MapObject>()->Find(hash);

            if (value)
            {
                return value;
            } else {
                return args[0].Call(interpreter, "__missing__", args[1]);
            }
        } else {
            return Value();
        }
    }

    /**
     * Map#__setitem__(key, value)
     *
     * Element assignment. Associates the value given by +value+ with the key
     * given by +key+. +key+ should not have it's value changed while it is
     * used as a key.
     *
     *     map = {a: 100, b: 200}
     *     map["a"] = 9
     *     map["b"] = 4
     *     map              #=> {"a": 9, "b": 4}
     */
    TEMPEARLY_NATIVE_METHOD(map_setitem)
    {
        const Value& key = args[1];
        const Value& value = args[2];
        i64 hash;

        if (!key.GetHash(interpreter, hash))
        {
            return Value();
        }
        args[0].As<MapObject>()->Insert(hash, key, value);

        return args[0];
    }

    /**
     * Map#__missing__(key)
     *
     * This method is invoked when an value is searched from the map which
     * doesn't exist. Default implementation throws KeyError.
     */
    TEMPEARLY_NATIVE_METHOD(map_missing)
    {
        String repr;

        if (args[1].ToString(interpreter, repr))
        {
            interpreter->Throw(interpreter->eKeyError, repr);
        }

        return Value();
    }

    /**
     * Map#__bool__() => Bool
     *
     * Boolean representation of map. Maps evaluate as true when they are not
     * empty.
     */
    TEMPEARLY_NATIVE_METHOD(map_bool)
    {
        return Value::NewBool(!args[0].As<MapObject>()->IsEmpty());
    }

    /**
     * Map#as_json() => String
     *
     * Converts map into JSON object literal and returns result.
     */
    TEMPEARLY_NATIVE_METHOD(map_as_json)
    {
        StringBuilder buffer;
        Handle<MapObject> map = args[0].As<MapObject>();

        buffer << '{';
        if (!map->HasFlag(CountedObject::FLAG_INSPECTING))
        {
            bool first = true;

            map->SetFlag(CountedObject::FLAG_INSPECTING);
            for (Handle<MapObject::Entry> entry = map->GetFront(); entry; entry = entry->GetNext())
            {
                Value result;
                String key;
                String value;

                if (!entry->GetKey().ToString(interpreter, key)
                    || !(result = entry->GetValue().Call(interpreter, "as_json"))
                    || !result.AsString(interpreter, value))
                {
                    map->UnsetFlag(CountedObject::FLAG_INSPECTING);

                    return Value();
                }
                if (first)
                {
                    first = false;
                } else {
                    buffer << ',';
                }
                buffer << '"' << key.EscapeJavaScript() << '"' << ':' << value;
            }
            map->UnsetFlag(CountedObject::FLAG_INSPECTING);
        }
        buffer << '}';

        return Value::NewString(buffer.ToString());
    }

    /**
     * Map#__add__(map) => Map
     *
     * Concatenates two maps into one and returns result.
     */
    TEMPEARLY_NATIVE_METHOD(map_add)
    {
        Handle<MapObject> map = args[0].As<MapObject>();
        Handle<MapObject> other;
        Handle<MapObject> result;

        if (!args[1].IsMap())
        {
            interpreter->Throw(interpreter->eValueError, "Map required");

            return Value();
        }
        result = new MapObject(interpreter->cMap);
        for (Handle<MapObject::Entry> entry = map->GetFront(); entry; entry = entry->GetNext())
        {
            result->Insert(entry->GetHash(), entry->GetKey(), entry->GetValue());
        }
        other = args[1].As<MapObject>();
        for (Handle<MapObject::Entry> entry = other->GetFront(); entry; entry = entry->GetNext())
        {
            result->Insert(entry->GetHash(), entry->GetKey(), entry->GetValue());
        }

        return Value(result);
    }

    void init_map(Interpreter* i)
    {
        Handle<Class> cMap = i->AddClass("Map", i->cIterable);

        i->cMap = cMap;

        cMap->SetAllocator(map_alloc);

        cMap->AddMethod(i, "size", 0, map_size);
        cMap->AddMethod(i, "keys", 0, map_keys);
        cMap->AddMethod(i, "values", 0, map_values);

        cMap->AddMethod(i, "clear", 0, map_clear);
        cMap->AddMethod(i, "has", 1, map_has);
        cMap->AddMethod(i, "get", -2, map_get);
        cMap->AddMethod(i, "join", -1, map_join);
        cMap->AddMethod(i, "update", 1, map_update);

        cMap->AddMethod(i, "__iter__", 0, map_iter);

        cMap->AddMethod(i, "__getitem__", 1, map_getitem);
        cMap->AddMethod(i, "__setitem__", 2, map_setitem);

        cMap->AddMethod(i, "__missing__", 1, map_missing);

        // Conversion methods
        cMap->AddMethod(i, "__bool__", 0, map_bool);
        cMap->AddMethod(i, "as_json", 0, map_as_json);
        cMap->AddMethodAlias(i, "__str__", "join");

        i->cMap->AddMethod(i, "__add__", 1, map_add);
    }
}
