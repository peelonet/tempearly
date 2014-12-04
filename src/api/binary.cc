#include "interpreter.h"
#include "utils.h"
#include "api/iterator.h"
#include "api/range.h"
#include "core/bytestring.h"
#include "core/random.h"

namespace tempearly
{
    /**
     * Binary.__call__(arguments...) => Binary
     *
     * Constructs binary data from objects given as arguments. Objects must be
     * either numbers or other binary objects.
     */
    TEMPEARLY_NATIVE_METHOD(bin_s_call)
    {
        Vector<byte> result;

        result.Reserve(args.GetSize());
        for (std::size_t i = 0; i < args.GetSize(); ++i)
        {
            const Value& value = args[i];

            if (value.IsBinary())
            {
                const ByteString& b = value.AsBinary();

                result.PushBack(b.GetBytes(), b.GetLength());
            } else {
                i64 number;

                if (!value.AsInt(interpreter, number))
                {
                    return;
                }
                else if (number < 0 || number > 255)
                {
                    interpreter->Throw(interpreter->eValueError,
                                       "Value out of bounds: "
                                       + Utils::ToString(number));
                    return;
                }
                result.PushBack(static_cast<byte>(number));
            }
        }
        frame->SetReturnValue(Value::NewBinary(ByteString(result.GetData(), result.GetSize())));
    }

    /**
     * Binary.rand(length) => Binary
     *
     * Generates random binary data which contains given length of bytes.
     *
     * Throws: ValueError - If length is zero or below zero.
     */
    TEMPEARLY_NATIVE_METHOD(bin_s_rand)
    {
        i64 length;
        Vector<byte> result;

        if (!args[0].AsInt(interpreter, length))
        {
            return;
        }
        else if (length == 0)
        {
            interpreter->Throw(interpreter->eValueError, "Length cannot be zero");
            return;
        }
        else if (length < 0)
        {
            interpreter->Throw(interpreter->eValueError, "Length cannot be less than one");
            return;
        }
        result.Reserve(length);
        for (i64 i = 0; i < length; ++i)
        {
            result.PushBack(Random::NextU8());
        }
        frame->SetReturnValue(Value::NewBinary(ByteString(result.GetData(), result.GetSize())));
    }

    /**
     * Binary#length() => Int
     *
     * Returns the number of bytes which the binary data contains.
     */
    TEMPEARLY_NATIVE_METHOD(bin_length)
    {
        frame->SetReturnValue(Value::NewInt(args[0].AsBinary().GetLength()));
    }

    /**
     * Binary#chop() => Binary
     *
     * Removes trailing byte from binary data and returns result.
     */
    TEMPEARLY_NATIVE_METHOD(bin_chop)
    {
        const ByteString& b = args[0].AsBinary();

        if (b.IsEmpty())
        {
            frame->SetReturnValue(args[0]);
        } else {
            frame->SetReturnValue(Value::NewBinary(ByteString(b.GetBytes(), b.GetLength() - 1)));
        }
    }

    /**
     * Binary#chomp() => Binary
     *
     * Removes trailing new line from the binary data if present and returns
     * result. All possible new line combinations are supported.
     */
    TEMPEARLY_NATIVE_METHOD(bin_chomp)
    {
        const ByteString& b = args[0].AsBinary();

        if (!b.IsEmpty())
        {
            const std::size_t length = b.GetLength();

            if (length > 1 && b[length - 2] == '\r' && b[length - 1] == '\n')
            {
                frame->SetReturnValue(Value::NewBinary(ByteString(b.GetBytes(), length - 2)));
                return;
            }
            else if (b[length - 1] == '\n' || b[length - 1] == '\r')
            {
                frame->SetReturnValue(Value::NewBinary(ByteString(b.GetBytes(), length - 1)));
                return;
            }
        }
        frame->SetReturnValue(args[0]);
    }

    /**
     * Binary#reverse() => Binary
     *
     * Returns reversed copy of binary data.
     */
    TEMPEARLY_NATIVE_METHOD(bin_reverse)
    {
        const ByteString& b = args[0].AsBinary();

        if (!b.IsEmpty())
        {
            Vector<byte> result;

            result.Reserve(b.GetLength());
            for (std::size_t i = b.GetLength(); i > 0; --i)
            {
                result.PushBack(b[i - 1]);
            }
            frame->SetReturnValue(Value::NewBinary(ByteString(result.GetData(), result.GetSize())));
        } else {
            frame->SetReturnValue(args[0]);
        }
    }

    /**
     * Binary#__hash__() => Int
     *
     * Calculates hash code for the binary data with Jenkins hash.
     */
    TEMPEARLY_NATIVE_METHOD(bin_hash)
    {
        const ByteString& b = args[0].AsBinary();
        i64 hash = 0;

        for (std::size_t i = 0; i < b.GetLength(); ++i)
        {
            hash += b[i];
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }
        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);
        frame->SetReturnValue(Value::NewInt(hash));
    }

    namespace
    {
        class BinaryIterator : public IteratorObject
        {
        public:
            explicit BinaryIterator(const Handle<Interpreter>& interpreter, const ByteString& bytes)
                : IteratorObject(interpreter->cIterator)
                , m_bytes(bytes)
                , m_index(0) {}

            Result Generate(const Handle<Interpreter>& interpreter)
            {
                if (m_index < m_bytes.GetLength())
                {
                    return Value::NewInt(m_bytes[m_index++]);
                } else {
                    return Result(Result::KIND_BREAK);
                }
            }

        private:
            const ByteString m_bytes;
            std::size_t m_index;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(BinaryIterator);
        };
    }

    /**
     * Binary#__iter__() => Iterator
     *
     * Returns an iterator which iterates over each byte from the binary data.
     */
    TEMPEARLY_NATIVE_METHOD(bin_iter)
    {
        const ByteString& b = args[0].AsBinary();
        Handle<IteratorObject> iterator;

        if (b.IsEmpty())
        {
            iterator = interpreter->GetEmptyIterator();
        } else {
            iterator = new BinaryIterator(interpreter, b);
        }
        frame->SetReturnValue(Value(iterator));
    }

    /**
     * Binary#__bool__() => Bool
     *
     * Boolean representation of binary data. Binary data evaluates as true
     * when it's not empty.
     */
    TEMPEARLY_NATIVE_METHOD(bin_bool)
    {
        frame->SetReturnValue(Value::NewBool(!args[0].AsBinary().IsEmpty()));
    }

    /**
     * Binary#__add__(other) => Binary
     *
     * Concatenates contents of two binary strings.
     *
     * Throws: TypeError - If object given as argument is not binary data.
     */
    TEMPEARLY_NATIVE_METHOD(bin_add)
    {
        const ByteString& a = args[0].AsBinary();
        ByteString b;

        if (!args[1].AsBinary(interpreter, b))
        {
            return;
        }
        if (a.IsEmpty())
        {
            frame->SetReturnValue(args[1]);
        }
        else if (b.IsEmpty())
        {
            frame->SetReturnValue(args[0]);
        } else {
            frame->SetReturnValue(Value::NewBinary(a + b));
        }
    }

    /**
     * Binary#__mul__(count) => Binary
     *
     * Repeats binary data <i>count</i> times and returns result.
     *
     * Throws: ValueError - If <i>count</i> is negative.
     */
    TEMPEARLY_NATIVE_METHOD(bin_mul)
    {
        i64 count;

        if (args[1].AsInt(interpreter, count))
        {
            const ByteString& b = args[0].AsBinary();

            if (count < 0)
            {
                interpreter->Throw(interpreter->eValueError, "Negative multiplier");
                return;
            }
            else if (count == 1 || b.IsEmpty())
            {
                frame->SetReturnValue(args[0]);
                return;
            } else {
                Vector<byte> result;

                result.Reserve(b.GetLength() * count);
                for (i64 i = 0; i < count; ++i)
                {
                    result.PushBack(b.GetBytes(), b.GetLength());
                }
                frame->SetReturnValue(Value::NewBinary(ByteString(result.GetData(), result.GetSize())));
                return;
            }
        }
    }

    /**
     * Binary#__eq__(other) => Bool
     *
     * Tests whether two binary objects are equal.
     */
    TEMPEARLY_NATIVE_METHOD(bin_eq)
    {
        const Value& self = args[0];
        const Value& operand = args[1];

        if (operand.IsBinary())
        {
            frame->SetReturnValue(Value::NewBool(self.AsBinary().Equals(operand.AsBinary())));
        } else {
            frame->SetReturnValue(Value::NewBool(false));
        }
    }

    /**
     * Binary#__lt__(other) => Bool
     *
     * Compares two binary datas lexicographically and returns true if receiving
     * value is less than value given as argument.
     *
     * Throws: TypeError - If other object is not instance of Binary.
     */
    TEMPEARLY_NATIVE_METHOD(bin_lt)
    {
        const Value& self = args[0];
        const Value& operand = args[1];

        if (operand.IsBinary())
        {
            frame->SetReturnValue(Value::NewBool(self.AsBinary().Compare(operand.AsBinary()) < 0));
        } else {
            interpreter->Throw(
                interpreter->eTypeError,
                "Cannot compare '" + operand.GetClass(interpreter)->GetName() + "' with 'Binary'"
            );
        }
    }

    /**
     * Binary#__getitem__(index) => Int
     * Binary#__getitem__(range) => Binary
     *
     * If a number is given as argument, returns byte from that index.
     *
     * If a range is given as argument, returns subsection of binary data with
     * range begin and end values as beginning and ending indexes.
     *
     * Negative indexes count backwards, e.g. -1 is index of the last byte in
     * the binary data.
     *
     * Throws: IndexError - If any of the indexes is out of bounds.
     */
    TEMPEARLY_NATIVE_METHOD(bin_getitem)
    {
        const ByteString& b = args[0].AsBinary();
        const std::size_t length = b.GetLength();
        const Value& argument = args[1];

        if (argument.IsRange())
        {
            Handle<RangeObject> range = argument.As<RangeObject>();
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
                begin += length;
            }
            if (end < 0)
            {
                end += length;
            }
            if (begin < 0 || begin >= static_cast<i64>(length) || end < 0 || end >= static_cast<i64>(length))
            {
                interpreter->Throw(interpreter->eIndexError, "Index out of bounds");
                return;
            }
            frame->SetReturnValue(Value::NewBinary(ByteString(b.GetBytes() + begin, end - begin)));
        } else {
            i64 index;

            if (!argument.AsInt(interpreter, index))
            {
                return;
            }
            if (index < 0)
            {
                index += length;
            }
            if (index < 0 || index >= static_cast<i64>(length))
            {
                interpreter->Throw(interpreter->eIndexError, "Index out of bounds");
                return;
            }
            frame->SetReturnValue(Value::NewInt(b[index]));
        }
    }

    void init_binary(Interpreter* i)
    {
        Handle<Class> cBinary = i->AddClass("Binary", i->cIterable);

        i->cBinary = cBinary;
        cBinary->SetAllocator(Class::kNoAlloc);

        i->cBinary->AddStaticMethod(i, "__call__", -1, bin_s_call);
        i->cBinary->AddStaticMethod(i, "rand", 1, bin_s_rand);

        i->cBinary->AddMethod(i, "length", 0, bin_length);

        // Manipulation methods
        i->cBinary->AddMethod(i, "chop", 0, bin_chop);
        i->cBinary->AddMethod(i, "chomp", 0, bin_chomp);
        i->cBinary->AddMethod(i, "reverse", 0, bin_reverse);

        i->cBinary->AddMethod(i, "__hash__", 0, bin_hash);
        i->cBinary->AddMethod(i, "__iter__", 0, bin_iter);

        // Conversion methods
        i->cBinary->AddMethod(i, "__bool__", 0, bin_bool);

        // Operators
        i->cBinary->AddMethod(i, "__add__", 1, bin_add);
        i->cBinary->AddMethod(i, "__mul__", 1, bin_mul);
        i->cBinary->AddMethod(i, "__eq__", 1, bin_eq);
        i->cBinary->AddMethod(i, "__lt__", 1, bin_lt);
        i->cBinary->AddMethod(i, "__getitem__", 1, bin_getitem);

        i->cBinary->AddMethodAlias(i, "__str__", "join");
    }
}
