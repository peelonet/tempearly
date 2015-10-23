#include "interpreter.h"
#include "api/class.h"
#include "core/stringbuilder.h"

namespace tempearly
{
    /**
     * Object#__init__()
     *
     * Works as constructor or initializer for the object.
     */
    TEMPEARLY_NATIVE_METHOD(obj_init) {}

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
        frame->SetReturnValue(Object::NewInt(reinterpret_cast<u64>(args[0].Get())));
    }

    /**
     * Object#__bool__() => Bool
     *
     * Boolean representation of the object.
     */
    TEMPEARLY_NATIVE_METHOD(obj_bool)
    {
        const Handle<Object>& receiver = args[0];

        if (receiver->IsBool())
        {
            frame->SetReturnValue(receiver);
        } else {
            frame->SetReturnValue(Object::NewBool(!receiver->IsNull()));
        }
    }

    /**
     * Object#__str__() => String
     *
     * String representation of the object.
     */
    TEMPEARLY_NATIVE_METHOD(obj_str)
    {
        const Handle<Object>& receiver = args[0];

        if (receiver->IsString())
        {
            frame->SetReturnValue(receiver);
        } else {
            frame->SetReturnValue(Object::NewString("<object>"));
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
        const Handle<Object>& self = args[0];

        buffer << '{';
        if (!self->HasFlag(CountedObject::FLAG_INSPECTING))
        {
            const Dictionary<Handle<Object>> attributes = self->GetOwnAttributes();
            bool first = true;

            self->SetFlag(CountedObject::FLAG_INSPECTING);
            for (const Dictionary<Handle<Object>>::Entry* entry = attributes.GetFront();
                 entry;
                 entry = entry->GetNext())
            {
                Handle<Object> result;
                String value;

                if (!entry->GetValue()->CallMethod(interpreter, result, "as_json")
                    || !result->AsString(interpreter, value))
                {
                    self->UnsetFlag(CountedObject::FLAG_INSPECTING);
                    return;
                }
                if (first)
                {
                    first = false;
                } else {
                    buffer << ',';
                }
                buffer << '"' << entry->GetName().EscapeJavaScript() << '"' << ':' << value;
            }
            self->UnsetFlag(CountedObject::FLAG_INSPECTING);
        }
        buffer << '}';
        frame->SetReturnValue(Object::NewString(buffer.ToString()));
    }

    /**
     * Object#__eq__(other) => Bool
     *
     * Magic method used for equality comparison. Default implementation does
     * not test equality, but rather whether the two objects are same instance.
     */
    TEMPEARLY_NATIVE_METHOD(obj_eq)
    {
        const Handle<Object>& self = args[0];
        const Handle<Object>& operand = args[1];
        bool result;

        if (self->IsNull())
        {
            result = operand->IsNull();
        }
        else if (self->IsBool())
        {
            result = operand->IsBool() && self->AsBool() == operand->AsBool();
        } else {
            result = self.Get() == operand.Get();
        }
        frame->SetReturnValue(Object::NewBool(result));
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
        const Handle<Object>& self = args[0];
        const Handle<Object>& operand = args[1];
        bool slot;

        if (!self->IsLessThan(interpreter, operand, slot))
        {
            return;
        }
        else if (slot)
        {
            frame->SetReturnValue(Object::NewBool(false));
        }
        else if (self->Equals(interpreter, operand, slot))
        {
            frame->SetReturnValue(Object::NewBool(!slot));
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
        const Handle<Object>& self = args[0];
        const Handle<Object>& operand = args[1];
        bool slot;

        if (!self->IsLessThan(interpreter, operand, slot))
        {
            return;
        }
        else if (slot)
        {
            frame->SetReturnValue(Object::NewBool(true));
        }
        else if (self->Equals(interpreter, operand, slot))
        {
            frame->SetReturnValue(Object::NewBool(slot));
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
        const Handle<Object>& self = args[0];
        const Handle<Object>& operand = args[1];
        bool slot;

        if (!self->IsLessThan(interpreter, operand, slot))
        {
            return;
        }
        else if (slot)
        {
            frame->SetReturnValue(Object::NewBool(false));
        }
        else if (self->Equals(interpreter, operand, slot))
        {
            frame->SetReturnValue(Object::NewBool(slot));
        }
    }

    void init_object(Interpreter* i)
    {
        Handle<Class> cObject = i->AddClass("Object", Handle<Class>());
        Handle<Class> cFunction = i->AddClass("Function", cObject);

        i->cObject = cObject.Get();
        i->cFunction = cFunction.Get();

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
