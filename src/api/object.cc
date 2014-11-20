#include "interpreter.h"
#include "api/class.h"
#include "api/object.h"
#include "core/stringbuilder.h"

namespace tempearly
{
    Object::Object(const Handle<Class>& cls)
        : m_class(cls.Get())
        , m_attributes(0) {}

    Object::~Object()
    {
        if (m_attributes)
        {
            delete m_attributes;
        }
    }

    Dictionary<Value> Object::GetAllAttributes() const
    {
        if (m_attributes)
        {
            return *m_attributes;
        } else {
            return Dictionary<Value>();
        }
    }

    bool Object::HasAttribute(const String& id) const
    {
        return m_attributes && m_attributes->Find(id);
    }

    bool Object::GetAttribute(const String& id, Value& value) const
    {
        if (m_attributes)
        {
            const AttributeMap::Entry* entry = m_attributes->Find(id);

            if (entry)
            {
                value = entry->GetValue();

                return true;
            }
        }

        return false;
    }

    void Object::SetAttribute(const String& id, const Value& value)
    {
        if (!m_attributes)
        {
            m_attributes = new AttributeMap();
        }
        m_attributes->Insert(id, value);
    }

    void Object::Mark()
    {
        CoreObject::Mark();
        if (m_class && !m_class->IsMarked())
        {
            m_class->Mark();
        }
        if (m_attributes)
        {
            for (const AttributeMap::Entry* entry = m_attributes->GetFront(); entry; entry = entry->GetNext())
            {
                entry->GetValue().Mark();
            }
        }
    }

    /**
     * Object#__init__()
     *
     * Works as constructor or initializer for the object.
     */
    TEMPEARLY_NATIVE_METHOD(obj_init)
    {
        return Value::NullValue();
    }

    /**
     * Object#__hash__() => Int
     *
     * Calculates hash code for the object. Hash codes are used by various
     * methods and containers to identify objects from each other. Default
     * hash code is calculated by casting the pointer of the object into
     * integer.
     */
    TEMPEARLY_NATIVE_METHOD(obj_hash)
    {
        if (args[0].Is(Value::KIND_OBJECT))
        {
            Handle<CoreObject> object = args[0].AsObject();

            return Value::NewInt(reinterpret_cast<u64>(object.Get()));
        }

        return Value::NewInt(0);
    }

    /**
     * Object#__bool__() => Bool
     *
     * Boolean representation of the object.
     */
    TEMPEARLY_NATIVE_METHOD(obj_bool)
    {
        const Value& self = args[0];

        if (self.IsBool())
        {
            return self;
        } else {
            return Value::NewBool(!self.IsNull());
        }
    }

    /**
     * Object#__str__() => String
     *
     * String representation of the object.
     */
    TEMPEARLY_NATIVE_METHOD(obj_str)
    {
        const Value& self = args[0];

        if (self.IsString())
        {
            return self;
        } else {
            return Value::NewString("<object>");
        }
    }

    /**
     * Object#as_json() => String
     *
     * Converts object into JSON object literal and returns result. Resulting
     * object literal will contain all attributes which the object has as keys
     * and values.
     */
    TEMPEARLY_NATIVE_METHOD(obj_as_json)
    {
        StringBuilder buffer;
        const Value& self = args[0];

        buffer << '{';
        if (!self.HasFlag(CountedObject::FLAG_INSPECTING))
        {
            const Dictionary<Value> attributes = self.GetAllAttributes();
            bool first = true;

            self.SetFlag(CountedObject::FLAG_INSPECTING);
            for (const Dictionary<Value>::Entry* entry = attributes.GetFront(); entry; entry = entry->GetNext())
            {
                Value result;
                String value;

                if (!(result = entry->GetValue().Call(interpreter, "as_json")) || !result.AsString(interpreter, value))
                {
                    self.UnsetFlag(CountedObject::FLAG_INSPECTING);

                    return Value();
                }
                if (first)
                {
                    first = false;
                } else {
                    buffer << ',';
                }
                buffer << '"' << entry->GetName().EscapeJavaScript() << '"' << ':' << value;
            }
            self.UnsetFlag(CountedObject::FLAG_INSPECTING);
        }
        buffer << '}';

        return Value::NewString(buffer.ToString());
    }

    /**
     * Object#__eq__(other) => Bool
     *
     * Magic method used for equality comparison. Default implementation does
     * not test equality, but rather whether the two objects are same instance.
     */
    TEMPEARLY_NATIVE_METHOD(obj_eq)
    {
        const Value& self = args[0];
        const Value& operand = args[1];
        bool result;

        if (self.Is(Value::KIND_NULL))
        {
            result = operand.Is(Value::KIND_NULL);
        }
        else if (self.Is(Value::KIND_BOOL))
        {
            result = operand.Is(Value::KIND_BOOL) && self.AsBool() == operand.AsBool();
        }
        else if (self.Is(Value::KIND_OBJECT))
        {
            result = self.IsInstance(interpreter, operand.GetClass(interpreter))
                && operand.Is(Value::KIND_OBJECT)
                && self.AsObject() == operand.AsObject();
        } else {
            result = false;
        }

        return Value::NewBool(result);
    }

    /**
     * Object#__gt__(other) => Bool
     *
     * Returns true if receiving object is greater than the object given as
     * argument. Default implementation requires "__lt__" and "__eq__" methods
     * in order to function.
     */
    TEMPEARLY_NATIVE_METHOD(obj_gt)
    {
        const Value& self = args[0];
        const Value& operand = args[1];
        bool slot;

        if (!self.IsLessThan(interpreter, operand, slot))
        {
            return Value();
        }
        else if (slot)
        {
            return Value::NewBool(false);
        }
        else if (!self.Equals(interpreter, operand, slot))
        {
            return Value();
        } else {
            return Value::NewBool(!slot);
        }
    }

    /**
     * Object#__lte__(other) => Bool
     *
     * Returns true if receiving object is less than or equal to object given
     * as argument. Default implementation requires "__lt__" and "__eq__"
     * methods in order to function.
     */
    TEMPEARLY_NATIVE_METHOD(obj_lte)
    {
        const Value& self = args[0];
        const Value& operand = args[1];
        bool slot;

        if (!self.IsLessThan(interpreter, operand, slot))
        {
            return Value();
        }
        else if (slot)
        {
            return Value::NewBool(true);
        }
        else if (!self.Equals(interpreter, operand, slot))
        {
            return Value();
        } else {
            return Value::NewBool(slot);
        }
    }

    /**
     * Object#__gte__(other) => Bool
     *
     * Returns true if receiving object is greater than or equal to object
     * given as argument. Default implementation requires "__lt__" and "__eq__"
     * methods in order to function.
     */
    TEMPEARLY_NATIVE_METHOD(obj_gte)
    {
        const Value& self = args[0];
        const Value& operand = args[1];
        bool slot;

        if (!self.IsLessThan(interpreter, operand, slot))
        {
            return Value();
        }
        else if (slot)
        {
            return Value::NewBool(false);
        }
        else if (!self.Equals(interpreter, operand, slot))
        {
            return Value();
        } else {
            return Value::NewBool(slot);
        }
    }

    void init_object(Interpreter* i)
    {
        Handle<Class> cObject = i->AddClass("Object", Handle<Class>());

        i->cObject = cObject.Get();

        cObject->AddMethod(i, "__init__", 0, obj_init);
        cObject->AddMethod(i, "__hash__", 0, obj_hash);

        // Conversion methods
        cObject->AddMethod(i, "__bool__", 0, obj_bool);
        cObject->AddMethod(i, "__str__", 0, obj_str);
        cObject->AddMethod(i, "as_json", 0, obj_as_json);

        // Comparison operators.
        cObject->AddMethod(i, "__eq__", 1, obj_eq);
        cObject->AddMethod(i, "__gt__", 1, obj_gt);
        cObject->AddMethod(i, "__lte__", 1, obj_lte);
        cObject->AddMethod(i, "__gte__", 1, obj_gte);
    }
}
