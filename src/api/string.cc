#include "interpreter.h"
#include "api/iterator.h"
#include "api/list.h"
#include "core/stringbuilder.h"

namespace tempearly
{
    /**
     * String#length() => Int
     *
     * Returns length of the string.
     *
     *     "foo bar".length() #=> 7
     */
    TEMPEARLY_NATIVE_METHOD(str_length)
    {
        return Value::NewInt(args[0].AsString().GetLength());
    }

    /**
     * String#lines() => List
     *
     * Returns a list where each line in the string is presented as an
     * individual string. All possible new line combinations are supported.
     */
    TEMPEARLY_NATIVE_METHOD(str_lines)
    {
        const String& s = args[0].AsString();
        Handle<ListObject> list = new ListObject(interpreter->cList);
        std::size_t begin = 0;
        std::size_t end = 0;

        for (std::size_t i = 0; i < s.GetLength(); ++i)
        {
            if (i + 1 < s.GetLength() && s[i] == '\r' && s[i + 1] == '\n')
            {
                list->Append(Value::NewString(s.SubString(begin, end - begin)));
                begin = end = i + 2;
                ++i;
            }
            else if (s[i] == '\n' || s[i] == '\r')
            {
                list->Append(Value::NewString(s.SubString(begin, end - begin)));
                begin = end = i + 1;
            } else {
                ++end;
            }
        }
        if (end - begin)
        {
            list->Append(Value::NewString(s.SubString(begin)));
        }

        return Value::NewObject(list);
    }

    /**
     * String#runes() => List
     *
     * Returns a list where each character of the string is presented as
     * integer value (Unicode code point).
     *
     *     "foo".runes()  #=> [102, 111, 111]
     */
    TEMPEARLY_NATIVE_METHOD(str_runes)
    {
        const String& s = args[0].AsString();
        Handle<ListObject> list = new ListObject(interpreter->cList);

        for (std::size_t i = 0; i < s.GetLength(); ++i)
        {
            list->Append(Value::NewInt(s[i]));
        }

        return Value::NewObject(list);
    }

    /**
     * String#words() => List
     *
     * Extracts whitespace separated words from the string and returns them in
     * a list.
     *
     *     "foo bar baz".words()  #=> ["foo", "bar", "baz"]
     */
    TEMPEARLY_NATIVE_METHOD(str_words)
    {
        const String& s = args[0].AsString();
        Handle<ListObject> list = new ListObject(interpreter->cList);
        std::size_t begin = 0;
        std::size_t end = 0;

        for (std::size_t i = 0; i < s.GetLength(); ++i)
        {
            if (std::isspace(s[i]))
            {
                if (end - begin > 0)
                {
                    list->Append(Value::NewString(s.SubString(begin, end - begin)));
                }
                begin = end = i + 1;
            } else {
                ++end;
            }
        }
        if (end - begin > 0)
        {
            list->Append(Value::NewString(s.SubString(begin)));
        }

        return Value::NewObject(list);
    }

    /**
     * String#capitalize() => String
     *
     * Returns a copy of string where first character is converted into upper
     * case and remaining characters are converted into lower case.
     *
     *     "foo".capitalize() #=> "Foo"
     */
    TEMPEARLY_NATIVE_METHOD(str_capitalize)
    {
        const String& s = args[0].AsString();

        if (!s.IsEmpty())
        {
            StringBuilder buffer(s.GetLength());

            buffer.Append(String::ToUpper(s[0]));
            for (std::size_t i = 1; i < s.GetLength(); ++i)
            {
                buffer.Append(String::ToLower(s[i]));
            }

            return Value::NewString(buffer.ToString());
        }

        return args[0];
    }

    /**
     * String#chomp() => String
     *
     * Removes trailing new line from the string if present and returns result.
     * All possible new line combinations are supported.
     *
     *     "foo\r\n".chomp()    #=> "foo"
     *     "foo\n".chomp()      #=> "foo"
     *     "foo".chomp()        #=> "foo"
     */
    TEMPEARLY_NATIVE_METHOD(str_chomp)
    {
        const String& s = args[0].AsString();

        if (!s.IsEmpty())
        {
            const std::size_t len = s.GetLength();

            if (len > 1 && s[len - 2] == '\r' && s[len - 1] == '\n')
            {
                return Value::NewString(s.SubString(0, len - 2));
            }
            else if (s[len - 1] == '\n' || s[len - 1] == '\r')
            {
                return Value::NewString(s.SubString(0, len - 1));
            }
        }

        return args[0];
    }

    /**
     * String#chop() => String
     *
     * Removes trailing character from the string and returns result.
     *
     *     "foo\n".chop()   #=> "foo"
     *     "foo".chop()     #=> "fo"
     */
    TEMPEARLY_NATIVE_METHOD(str_chop)
    {
        const String& s = args[0].AsString();

        if (s.IsEmpty())
        {
            return args[0];
        } else {
            return Value::NewString(s.SubString(0, s.GetLength() - 1));
        }
    }

    /**
     * String#lower() => String
     *
     * Converts string into lower case.
     *
     *     "FOO".lower()    #=> "foo"
     */
    TEMPEARLY_NATIVE_METHOD(str_lower)
    {
        const String& s = args[0].AsString();

        for (std::size_t i = 0; i < s.GetLength(); ++i)
        {
            if (String::IsUpper(s[i]))
            {
                StringBuilder buffer(s.GetLength());

                if (i > 0)
                {
                    buffer.Append(s.GetRunes(), i);
                }
                for (std::size_t j = i; j < s.GetLength(); ++j)
                {
                    buffer.Append(String::ToLower(s[j]));
                }

                return Value::NewString(buffer.ToString());
            }
        }

        return args[0];
    }

    /**
     * String#reverse() => String
     *
     * Returns reversed copy of the string.
     *
     *     "foobar".reverse()   #=> "raboof"
     */
    TEMPEARLY_NATIVE_METHOD(str_reverse)
    {
        const String& s = args[0].AsString();

        if (!s.IsEmpty())
        {
            StringBuilder buffer(s.GetLength());

            for (std::size_t i = s.GetLength(); i > 0; --i)
            {
                buffer.Append(s[i - 1]);
            }

            return Value::NewString(buffer.ToString());
        }

        return args[0];
    }

    /**
     * String#swapcase() => String
     *
     * Returns a copy of string where each upper case character is converted
     * into lower case and vice versa.
     *
     *     "fooBAR".swapcase()  #=> "FOObar"
     */
    TEMPEARLY_NATIVE_METHOD(str_swapcase)
    {
        const String& s = args[0].AsString();

        if (!s.IsEmpty())
        {
            StringBuilder buffer(s.GetLength());

            for (std::size_t i = 0; i < s.GetLength(); ++i)
            {
                rune c = s[i];

                if (String::IsLower(c))
                {
                    c = String::ToUpper(c);
                }
                else if (String::IsUpper(c))
                {
                    c = String::ToLower(c);
                }
                buffer.Append(c);
            }

            return Value::NewString(buffer.ToString());
        }

        return args[0];
    }

    /**
     * String#titleize() => String
     *
     * Like String#capitalize but processes every whitespace separated word on
     * the string.
     *
     *     "foo bar".titleize()     #=> "Foo Bar"
     */
    TEMPEARLY_NATIVE_METHOD(str_titleize)
    {
        const String& s = args[0].AsString();

        if (!s.IsEmpty())
        {
            StringBuilder buffer(s.GetLength());
            std::size_t begin = 0;
            std::size_t end = 0;

            for (std::size_t i = 0; i < s.GetLength(); ++i)
            {
                rune c = s[i];

                if (std::isspace(c))
                {
                    if (end - begin)
                    {
                        buffer.Append(String::ToUpper(s[begin]));
                        if (end - begin > 1)
                        {
                            buffer.Append(s.SubString(begin + 1, end - begin - 1));
                        }
                    }
                    buffer.Append(c);
                    begin = end = i + 1;
                } else {
                    ++end;
                }
            }
            if (end - begin)
            {
                buffer.Append(String::ToUpper(s[begin]));
                if (end - begin > 1)
                {
                    buffer.Append(s.SubString(begin + 1, end - begin - 1));
                }
            }

            return Value::NewString(buffer.ToString());
        }

        return args[0];
    }

    /**
     * String#trim() => String
     *
     * Returns a copy of string where whitespace from beginning and end of the
     * string is removed.
     *
     *     "  foo  ".trim()     #=> "foo"
     */
    TEMPEARLY_NATIVE_METHOD(str_trim)
    {
        const String& s = args[0].AsString();
        std::size_t i, j;

        for (i = 0; i < s.GetLength(); ++i)
        {
            if (!std::isspace(s[i]))
            {
                break;
            }
        }
        for (j = s.GetLength(); j != 0; --j)
        {
            if (!std::isspace(s[j - 1]))
            {
                break;
            }
        }
        if (i != 0 || j != s.GetLength())
        {
            return Value::NewString(s.SubString(i, j - i));
        }

        return args[0];
    }

    /**
     * String#upper() => String
     *
     * Converts string into upper case.
     *
     *     "foo".lower()    #=> "FOO"
     */
    TEMPEARLY_NATIVE_METHOD(str_upper)
    {
        const String& s = args[0].AsString();

        for (std::size_t i = 0; i < s.GetLength(); ++i)
        {
            if (String::IsLower(s[i]))
            {
                StringBuilder buffer(s.GetLength());

                if (i > 0)
                {
                    buffer.Append(s.GetRunes(), i);
                }
                for (std::size_t j = i; j < s.GetLength(); ++j)
                {
                    buffer.Append(String::ToUpper(s[j]));
                }

                return Value::NewString(buffer.ToString());
            }
        }

        return args[0];
    }

    /**
     * String#__hash__() => Int
     *
     * Generates hash code from contents of the string.
     *
     *     "foo".__hash__() #=> 101574
     */
    TEMPEARLY_NATIVE_METHOD(str_hash)
    {
        return Value::NewInt(args[0].AsString().HashCode());
    }

    namespace
    {
        class StringIterator : public IteratorObject
        {
        public:
            explicit StringIterator(const Handle<Class>& cls,
                                    const String& string)
                : IteratorObject(cls)
                , m_string(string)
                , m_index(0) {}

            Result Generate(const Handle<Interpreter>& interpreter)
            {
                if (m_index < m_string.GetLength())
                {
                    return Result(Result::KIND_SUCCESS,
                                  Value::NewString(m_string.SubString(m_index++, 1)));
                } else {
                    return Result(Result::KIND_BREAK);
                }
            }

        private:
            const String m_string;
            std::size_t m_index;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(StringIterator);
        };
    }

    /**
     * String#__iter__() => Iterator
     *
     * Returns an iterator which iterates over each character in the string.
     */
    TEMPEARLY_NATIVE_METHOD(str_iter)
    {
        const String& s = args[0].AsString();
        Handle<IteratorObject> iterator;

        if (s.IsEmpty())
        {
            iterator = interpreter->GetEmptyIterator();
        } else {
            iterator = new StringIterator(interpreter->cIterator, s);
        }

        return Value::NewObject(iterator);
    }

    /**
     * String#__bool__() => Bool
     *
     * Boolean representation of the string. Strings evaluate as true if they
     * are not empty.
     */
    TEMPEARLY_NATIVE_METHOD(str_bool)
    {
        return Value::NewBool(!args[0].AsString().IsEmpty());
    }

    /**
     * String#__add__(other) => String
     *
     * Concatenates two strings.
     *
     *     "foo" + "bar"    #=> "foobar"
     *
     * Throws: ValueError - If object given as operand is not a string.
     */
    TEMPEARLY_NATIVE_METHOD(str_add)
    {
        const Value& operand = args[1];

        if (operand.IsString())
        {
            const String& a = args[0].AsString();
            const String& b = operand.AsString();

            if (a.IsEmpty())
            {
                return operand;
            }
            else if (b.IsEmpty())
            {
                return args[0];
            } else {
                return Value::NewString(a + b);
            }
        }
        interpreter->Throw(interpreter->eValueError, "String value required");

        return Value();
    }

    /**
     * String#__mul__(count) => String
     *
     * Repeats string +count+ times.
     *
     *     "ho! " * 3   #=> "ho! ho! ho! "
     *
     * Throws: ValueError - If count is negative.
     */
    TEMPEARLY_NATIVE_METHOD(str_mul)
    {
        i64 count;

        if (args[1].AsInt(interpreter, count))
        {
            const String& string = args[0].AsString();

            if (count < 0)
            {
                interpreter->Throw(interpreter->eValueError,
                                   "Negative multiplier");
            }
            else if (count == 1 || string.IsEmpty())
            {
                return args[0];
            } else {
                StringBuilder buffer(string.GetLength() * count);

                while (count-- > 0)
                {
                    buffer.Append(string);
                }

                return Value::NewString(buffer.ToString());
            }
        }

        return Value();
    }

    /**
     * String#__cmp__(other) => Int
     *
     * Compares two strings lexicographically against each other.
     */
    TEMPEARLY_NATIVE_METHOD(str_cmp)
    {
        const Value& operand = args[1];

        if (operand.IsString())
        {
            const String& a = args[0].AsString();
            const String& b = operand.AsString();

            return Value::NewInt(a.Compare(b));
        }
        interpreter->Throw(interpreter->eTypeError,
                           "Values are not comparable");

        return Value();
    }

    void init_string(Interpreter* i)
    {
        i->cString = i->AddClass("String", i->cObject);

        i->cString->AddMethod(i, "length", 0, str_length);
        i->cString->AddMethod(i, "lines", 0, str_lines);
        i->cString->AddMethod(i, "runes", 0, str_runes);
        i->cString->AddMethod(i, "words", 0, str_words);

        // Manipulation methods.
        i->cString->AddMethod(i, "capitalize", 0, str_capitalize);
        i->cString->AddMethod(i, "chomp", 0, str_chomp);
        i->cString->AddMethod(i, "chop", 0, str_chop);
        i->cString->AddMethod(i, "lower", 0, str_lower);
        i->cString->AddMethod(i, "reverse", 0, str_reverse);
        i->cString->AddMethod(i, "swapcase", 0, str_swapcase);
        i->cString->AddMethod(i, "titleize", 0, str_titleize);
        i->cString->AddMethod(i, "trim", 0, str_trim);
        i->cString->AddMethod(i, "upper", 0, str_upper);

        i->cString->AddMethod(i, "__hash__", 0, str_hash);
        i->cString->AddMethod(i, "__iter__", 0, str_iter);

        // Conversion methods.
        i->cString->AddMethod(i, "__bool__", 0, str_bool);

        // Operators.
        i->cString->AddMethod(i, "__add__", 1, str_add);
        i->cString->AddMethod(i, "__mul__", 1, str_mul);
        i->cString->AddMethod(i, "__cmp__", 1, str_cmp);
    }
}
