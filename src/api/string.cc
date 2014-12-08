#include <cctype>

#include "interpreter.h"
#include "api/iterator.h"
#include "api/list.h"
#include "api/range.h"
#include "core/bytestring.h"
#include "core/random.h"
#include "core/stringbuilder.h"
#include "io/stream.h"
#include "json/parser.h"

namespace tempearly
{
    /**
     * String.__call__(arguments...) => String
     *
     * Constructs string from objects given as arguments.
     *
     *     String(1, 2, 3) => "123"
     */
    TEMPEARLY_NATIVE_METHOD(str_s_call)
    {
        const std::size_t size = args.GetSize();

        if (size == 1 && args[0].IsString())
        {
            frame->SetReturnValue(args[0]);
        } else {
            StringBuilder result;
            String slot;

            for (std::size_t i = 0; i < size; ++i)
            {
                if (!args[i].ToString(interpreter, slot))
                {
                    return;
                }
                result << slot;
            }
            frame->SetReturnValue(Value::NewString(result.ToString()));
        }
    }

    /**
     * String.rand(length) => String
     *
     * Generates random string which contains digits 0-9 and alphabetic
     * characters a-z.
     *
     * Throws: ValueError - If length is zero or below zero.
     */
    TEMPEARLY_NATIVE_METHOD(str_s_rand)
    {
        static const char input[] = "0123456789abcdefghijklmnopqrstuvwxyz";
        i64 length;

        if (args[0].AsInt(interpreter, length))
        {
            if (length == 0)
            {
                interpreter->Throw(interpreter->eValueError, "Length cannot be zero");
            }
            else if (length < 0)
            {
                interpreter->Throw(interpreter->eValueError, "Length cannot be less than one");
            } else {
                StringBuilder buffer(length);

                while (length-- > 0)
                {
                    buffer.Append(input[Random::NextU64() % 36]);
                }
                frame->SetReturnValue(Value::NewString(buffer.ToString()));
            }
        }
    }

    /**
     * String#length() => Int
     *
     * Returns length of the string.
     *
     *     "foo bar".length() #=> 7
     */
    TEMPEARLY_NATIVE_METHOD(str_length)
    {
        frame->SetReturnValue(Value::NewInt(args[0].AsString().GetLength()));
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
        frame->SetReturnValue(Value(list));
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
        frame->SetReturnValue(Value(list));
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
        frame->SetReturnValue(Value(list));
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
            frame->SetReturnValue(Value::NewString(buffer.ToString()));
        } else {
            frame->SetReturnValue(args[0]);
        }
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
                frame->SetReturnValue(Value::NewString(s.SubString(0, len - 2)));
                return;
            }
            else if (s[len - 1] == '\n' || s[len - 1] == '\r')
            {
                frame->SetReturnValue(Value::NewString(s.SubString(0, len - 1)));
                return;
            }
        }
        frame->SetReturnValue(args[0]);
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
            frame->SetReturnValue(args[0]);
        } else {
            frame->SetReturnValue(Value::NewString(s.SubString(0, s.GetLength() - 1)));
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
                frame->SetReturnValue(Value::NewString(buffer.ToString()));
                return;
            }
        }
        frame->SetReturnValue(args[0]);
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
            frame->SetReturnValue(Value::NewString(buffer.ToString()));
        } else {
            frame->SetReturnValue(args[0]);
        }
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
            frame->SetReturnValue(Value::NewString(buffer.ToString()));
        } else {
            frame->SetReturnValue(args[0]);
        }
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
            frame->SetReturnValue(Value::NewString(buffer.ToString()));
        } else {
            frame->SetReturnValue(args[0]);
        }
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
            frame->SetReturnValue(Value::NewString(s.SubString(i, j - i)));
        } else {
            frame->SetReturnValue(args[0]);
        }
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
                frame->SetReturnValue(Value::NewString(buffer.ToString()));
                return;
            }
        }
        frame->SetReturnValue(args[0]);
    }

    /**
     * String#has(substring) => Bool
     *
     * Returns true if given substring exists somewhere in the string.
     *
     *     "foobar".has("foo")  #=> true
     */
    TEMPEARLY_NATIVE_METHOD(str_has)
    {
        const String& s = args[0].AsString();
        String substring;

        if (!args[1].AsString(interpreter, substring))
        {
            return;
        }
        if (substring.IsEmpty())
        {
            frame->SetReturnValue(Value::NewBool(true));
        }
        else if (s.IsEmpty())
        {
            frame->SetReturnValue(Value::NewBool(false));
        }
        else if (substring.GetLength() == 1)
        {
            frame->SetReturnValue(Value::NewBool(s.IndexOf(substring[0]) != String::npos));
        } else {
            const std::size_t length1 = s.GetLength();
            const std::size_t length2 = substring.GetLength();

            if (length2 > length1)
            {
                frame->SetReturnValue(Value::NewBool(false));
            }
            else if (length2 == length1)
            {
                frame->SetReturnValue(Value::NewBool(s == substring));
            } else {
                for (std::size_t i = 0; i < length1; ++i)
                {
                    bool found = true;

                    if (i + length2 > length1)
                    {
                        break;
                    }
                    for (std::size_t j = 0; j < length2; ++j)
                    {
                        if (s[i + j] != substring[j])
                        {
                            found = false;
                            break;
                        }
                    }
                    if (found)
                    {
                        frame->SetReturnValue(Value::NewBool(true));
                        return;
                    }
                }
                frame->SetReturnValue(Value::NewBool(false));
            }
        }
    }

    /**
     * String#startswith(substring) => Bool
     *
     * Returns true if given substring exists in the beginning of the string.
     *
     *     "foobar".startswith("foo")   #=> true
     *     "foobar".startswith("bar")   #=> false
     */
    TEMPEARLY_NATIVE_METHOD(str_startswith)
    {
        const String& s = args[0].AsString();
        String substring;

        if (!args[1].AsString(interpreter, substring))
        {
            return;
        }
        if (substring.IsEmpty())
        {
            frame->SetReturnValue(Value::NewBool(true));
        }
        else if (s.IsEmpty())
        {
            frame->SetReturnValue(Value::NewBool(false));
        }
        else if (substring.GetLength() == 1)
        {
            frame->SetReturnValue(Value::NewBool(s[0] == substring[0]));
        } else {
            const std::size_t length1 = s.GetLength();
            const std::size_t length2 = substring.GetLength();

            if (length2 > length1)
            {
                frame->SetReturnValue(Value::NewBool(false));
            }
            else if (length2 == length1)
            {
                frame->SetReturnValue(Value::NewBool(s == substring));
            } else {
                for (std::size_t i = 0; i < length2; ++i)
                {
                    if (s[i] != substring[i])
                    {
                        frame->SetReturnValue(Value::NewBool(false));
                        return;
                    }
                }
                frame->SetReturnValue(Value::NewBool(true));
            }
        }
    }

    /**
     * String#endswith(substring) => Bool
     *
     * Returns true if given substring exists at the end of the string.
     *
     *     "foobar".endswith("bar")     #=> true
     *     "foobar".endswith("foo")     #=> false
     */
    TEMPEARLY_NATIVE_METHOD(str_endswith)
    {
        const String& s = args[0].AsString();
        String substring;

        if (!args[1].AsString(interpreter, substring))
        {
            return;
        }
        if (substring.IsEmpty())
        {
            frame->SetReturnValue(Value::NewBool(true));
        }
        else if (s.IsEmpty())
        {
            frame->SetReturnValue(Value::NewBool(false));
        }
        else if (substring.GetLength() == 1)
        {
            frame->SetReturnValue(Value::NewBool(s.GetBack() == substring[0]));
        } else {
            const std::size_t length1 = s.GetLength();
            const std::size_t length2 = substring.GetLength();

            if (length2 > length1)
            {
                frame->SetReturnValue(Value::NewBool(false));
            }
            else if (length2 == length1)
            {
                frame->SetReturnValue(Value::NewBool(s == substring));
            } else {
                for (std::size_t i = 0; i < length2; ++i)
                {
                    if (s[length1 - length2 + i] != substring[i])
                    {
                        frame->SetReturnValue(Value::NewBool(false));
                        return;
                    }
                }
                frame->SetReturnValue(Value::NewBool(true));
            }
        }
    }

    /**
     * String#index(substring) => Int
     *
     * Searches for given substring from the string and returns it's beginning
     * offset. If substring is not found, ValueError is thrown instead.
     */
    TEMPEARLY_NATIVE_METHOD(str_index)
    {
        const String& s = args[0].AsString();
        String substring;

        if (!args[1].AsString(interpreter, substring))
        {
            return;
        }
        if (substring.IsEmpty())
        {
            frame->SetReturnValue(Value::NewInt(0));
        }
        else if (s.IsEmpty())
        {
            interpreter->Throw(interpreter->eValueError, "Substring not found");
        }
        else if (substring.GetLength() == 1)
        {
            std::size_t index = s.IndexOf(substring[0]);

            if (index == String::npos)
            {
                interpreter->Throw(interpreter->eValueError, "Substring not found");
            } else {
                frame->SetReturnValue(Value::NewInt(static_cast<i64>(index)));
            }
        } else {
            const std::size_t length1 = s.GetLength();
            const std::size_t length2 = substring.GetLength();

            if (length2 > length1)
            {
                interpreter->Throw(interpreter->eValueError, "Substring not found");
            }
            else if (length2 == length1)
            {
                if (s == substring)
                {
                    frame->SetReturnValue(Value::NewInt(0));
                } else {
                    interpreter->Throw(interpreter->eValueError, "Substring not found");
                }
            } else {
                for (std::size_t i = 0; i < length1; ++i)
                {
                    bool found = true;

                    if (i + length2 > length1)
                    {
                        break;
                    }
                    for (std::size_t j = 0; j < length2; ++j)
                    {
                        if (s[i + j] != substring[j])
                        {
                            found = false;
                            break;
                        }
                    }
                    if (found)
                    {
                        frame->SetReturnValue(Value::NewInt(i));
                        return;
                    }
                }
                interpreter->Throw(interpreter->eValueError, "Substring not found");
            }
        }
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
        frame->SetReturnValue(Value::NewInt(args[0].AsString().HashCode()));
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
                    return Result(Result::KIND_SUCCESS, Value::NewString(m_string.SubString(m_index++, 1)));
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
        frame->SetReturnValue(Value(iterator));
    }

    /**
     * String#__bool__() => Bool
     *
     * Boolean representation of the string. Strings evaluate as true if they
     * are not empty.
     */
    TEMPEARLY_NATIVE_METHOD(str_bool)
    {
        frame->SetReturnValue(Value::NewBool(!args[0].AsString().IsEmpty()));
    }

    /**
     * String#as_json() => String
     *
     * Converts string into JSON string literal and returns result.
     */
    TEMPEARLY_NATIVE_METHOD(str_as_json)
    {
        StringBuilder buffer;

        buffer << '"' << args[0].AsString().EscapeJavaScript() << '"';
        frame->SetReturnValue(Value::NewString(buffer.ToString()));
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
                frame->SetReturnValue(operand);
            }
            else if (b.IsEmpty())
            {
                frame->SetReturnValue(args[0]);
            } else {
                frame->SetReturnValue(Value::NewString(a + b));
            }
        } else {
            interpreter->Throw(interpreter->eValueError, "String value required");
        }
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
                interpreter->Throw(interpreter->eValueError, "Negative multiplier");
            }
            else if (count == 1 || string.IsEmpty())
            {
                frame->SetReturnValue(args[0]);
            } else {
                StringBuilder buffer(string.GetLength() * count);

                while (count-- > 0)
                {
                    buffer.Append(string);
                }
                frame->SetReturnValue(Value::NewString(buffer.ToString()));
            }
        }
    }

    /**
     * String#__eq__(other) => Bool
     *
     * Compares two strings against each other and returns true if they are
     * equal.
     */
    TEMPEARLY_NATIVE_METHOD(str_eq)
    {
        const Value& self = args[0];
        const Value& operand = args[1];

        frame->SetReturnValue(
            Value::NewBool(
                operand.IsString() ? self.AsString().Equals(operand.AsString()) : false
            )
        );
    }

    /**
     * String#__lt__(other) => Bool
     *
     * Compares two strings lexicographically against each other and returns
     * true if receiving value is less than value given as argument.
     *
     * Throws: TypeError - If value given as argument is not instance of
     * String.
     */
    TEMPEARLY_NATIVE_METHOD(str_lt)
    {
        const Value& self = args[0];
        const Value& operand = args[1];

        if (operand.IsString())
        {
            frame->SetReturnValue(Value::NewBool(self.AsString().Compare(operand.AsString()) < 0));
        } else {
            interpreter->Throw(
                interpreter->eTypeError,
                "Cannot compare '" + operand.GetClass(interpreter)->GetName() + "' with 'String'"
            );
        }
    }

    /**
     * String#__getitem__(index) => String
     *
     * If a number is given as argument, rune from that specified index is
     * returned as a substring. If the index is out of bounds, an IndexError
     * is thrown.
     *
     * If range is given instead of a number, a substring from specified range
     * is returned.
     *
     * Negative indexes count backwards, e.g. -1 is the last character in
     * the string.
     */
    TEMPEARLY_NATIVE_METHOD(str_getitem)
    {
        const String& s = args[0].AsString();

        if (args[1].IsRange())
        {
            Handle<RangeObject> range = args[1].As<RangeObject>();
            i64 begin;
            i64 end;

            if (!range->GetBegin().AsInt(interpreter, begin) || !range->GetEnd().AsInt(interpreter, end))
            {
                return;
            }
            if (begin < 0)
            {
                begin += s.GetLength();
            }
            if (end < 0)
            {
                end += s.GetLength();
            }
            frame->SetReturnValue(Value::NewString(s.SubString(begin, end - begin)));
        } else {
            i64 index;

            if (!args[1].AsInt(interpreter, index))
            {
                return;
            }
            else if (index < 0)
            {
                index += s.GetLength();
            }
            if (index < 0 || index >= s.GetLength())
            {
                interpreter->Throw(interpreter->eIndexError, "String index out of bounds");
                return;
            }
            frame->SetReturnValue(Value::NewString(s.SubString(index, 1)));
        }
    }

    /**
     * String#parse_json() => Object
     *
     * Attempts to parse contents of the string as JSON and returns value
     * contained by that JSON.
     *
     * Throws: ValueError - If string cannot be parsed as JSON.
     */
    TEMPEARLY_NATIVE_METHOD(str_parse_json)
    {
        Handle<Stream> stream = args[0].AsString().Encode().AsStream();
        Handle<JsonParser> parser = new JsonParser(stream);
        Value value;

        if (parser->ParseValue(interpreter, value))
        {
            frame->SetReturnValue(value);
        }
        else if (!interpreter->HasException())
        {
            interpreter->Throw(interpreter->eValueError, parser->GetErrorMessage());
        }
    }

    void init_string(Interpreter* i)
    {
        Handle<Class> cString = i->AddClass("String", i->cIterable);

        i->cString = cString;

        cString->SetAllocator(Class::kNoAlloc);

        cString->AddStaticMethod(i, "__call__", -1, str_s_call);
        cString->AddStaticMethod(i, "rand", 1, str_s_rand);

        cString->AddMethod(i, "length", 0, str_length);
        cString->AddMethod(i, "lines", 0, str_lines);
        cString->AddMethod(i, "runes", 0, str_runes);
        cString->AddMethod(i, "words", 0, str_words);

        // Manipulation methods.
        cString->AddMethod(i, "capitalize", 0, str_capitalize);
        cString->AddMethod(i, "chomp", 0, str_chomp);
        cString->AddMethod(i, "chop", 0, str_chop);
        cString->AddMethod(i, "lower", 0, str_lower);
        cString->AddMethod(i, "reverse", 0, str_reverse);
        cString->AddMethod(i, "swapcase", 0, str_swapcase);
        cString->AddMethod(i, "titleize", 0, str_titleize);
        cString->AddMethod(i, "trim", 0, str_trim);
        cString->AddMethod(i, "upper", 0, str_upper);

        // Search methods.
        cString->AddMethod(i, "has", 1, str_has);
        cString->AddMethod(i, "startswith", 1, str_startswith);
        cString->AddMethod(i, "endswith", 1, str_endswith);
        cString->AddMethod(i, "index", 1, str_index);

        cString->AddMethod(i, "__hash__", 0, str_hash);
        cString->AddMethod(i, "__iter__", 0, str_iter);

        // Conversion methods.
        cString->AddMethod(i, "__bool__", 0, str_bool);
        cString->AddMethod(i, "as_json", 0, str_as_json);

        // Operators.
        cString->AddMethod(i, "__add__", 1, str_add);
        cString->AddMethod(i, "__mul__", 1, str_mul);
        cString->AddMethod(i, "__eq__", 1, str_eq);
        cString->AddMethod(i, "__lt__", 1, str_lt);

        cString->AddMethod(i, "__getitem__", 1, str_getitem);

        cString->AddMethod(i, "parse_json", 0, str_parse_json);
    }
}
