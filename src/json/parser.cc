#include <cctype>

#include "interpreter.h"
#include "utils.h"
#include "api/list.h"
#include "api/map.h"
#include "core/stringbuilder.h"
#include "json/parser.h"

namespace tempearly
{
    static bool parse_value(const Handle<JsonParser>&, const Handle<Interpreter>&, Value&);
    static bool parse_array(const Handle<JsonParser>&, const Handle<Interpreter>&, Value&);
    static bool parse_object(const Handle<JsonParser>&, const Handle<Interpreter>&, Value&);
    static bool parse_string(const Handle<JsonParser>&, const Handle<Interpreter>&, Value&);
    static bool parse_number(const Handle<JsonParser>&, const Handle<Interpreter>&, Value&, int);

    JsonParser::JsonParser(const Handle<Stream>& stream)
        : Parser(stream) {}

    bool JsonParser::ParseValue(const Handle<Interpreter>& interpreter, Value& slot)
    {
        return parse_value(this, interpreter, slot);
    }

    static bool parse_value(const Handle<JsonParser>& parser, const Handle<Interpreter>& interpreter, Value& slot)
    {
        int r;

        parser->SkipWhitespace();
        switch (r = parser->ReadRune())
        {
            case -1:
                parser->SetErrorMessage("Unexpected end of input; Missing JSON value");
                return false;

            case 't':
                if (parser->ReadRune('r') && parser->ReadRune('u') && parser->ReadRune('e'))
                {
                    slot = Value::NewBool(true);

                    return true;
                } else {
                    parser->SetErrorMessage("Unexpected identifier; Missing 'true'");

                    return false;
                }

            case 'f':
                if (parser->ReadRune('a') && parser->ReadRune('l') && parser->ReadRune('s') && parser->ReadRune('e'))
                {
                    slot = Value::NewBool(false);

                    return true;
                } else {
                    parser->SetErrorMessage("Unexpected identifier; Missing 'false'");

                    return false;
                }

            case 'n':
                if (parser->ReadRune('u') && parser->ReadRune('l') && parser->ReadRune('l'))
                {
                    slot = Value::NullValue();

                    return true;
                } else {
                    parser->SetErrorMessage("Unexpected identifier; Missing 'null'");

                    return false;
                }

            case '[':
                return parse_array(parser, interpreter, slot);

            case '{':
                return parse_object(parser, interpreter, slot);

            case '"':
                return parse_string(parser, interpreter, slot);

            case '-': case '0': case '1': case '2':
            case '3': case '4': case '5': case '6':
            case '7': case '8': case '9':
                return parse_number(parser, interpreter, slot, r);

            default:
                parser->SetErrorMessage("Unexpected input");
                return false;
        }
    }

    static bool parse_array(const Handle<JsonParser>& parser, const Handle<Interpreter>& interpreter, Value& slot)
    {
        Handle<ListObject> list = new ListObject(interpreter->cList);
        Value value;

        for (;;)
        {
            parser->SkipWhitespace();
            if (parser->ReadRune(']'))
            {
                slot = list;

                return true;
            }
            else if (!parse_value(parser, interpreter, value))
            {
                return false;
            }
            list->Append(value);
            parser->SkipWhitespace();
            if (parser->ReadRune(','))
            {
                continue;
            }
            else if (!parser->ReadRune(']'))
            {
                parser->SetErrorMessage("Unterminated array; Missing ']'");

                return false;
            }
            slot = list;

            return true;
        }
    }

    static bool parse_object(const Handle<JsonParser>& parser, const Handle<Interpreter>& interpreter, Value& slot)
    {
        Handle<MapObject> map = new MapObject(interpreter->cMap);
        Value key;
        Value value;
        i64 hash;

        for (;;)
        {
            parser->SkipWhitespace();
            if (parser->ReadRune('}'))
            {
                slot = map;

                return true;
            }
            else if (!parser->ReadRune('"'))
            {
                parser->SetErrorMessage("Missing string literal");

                return false;
            }
            else if (!parse_string(parser, interpreter, key) || !key.GetHash(interpreter, hash))
            {
                return false;
            }
            parser->SkipWhitespace();
            if (!parser->ReadRune(':'))
            {
                parser->SetErrorMessage("Missing ':'");

                return false;
            }
            parser->SkipWhitespace();
            if (!parse_value(parser, interpreter, value))
            {
                return false;
            }
            map->Insert(hash, key, value);
            parser->SkipWhitespace();
            if (parser->ReadRune(','))
            {
                continue;
            }
            else if (!parser->ReadRune('}'))
            {
                parser->SetErrorMessage("Unterminated object; Missing '}'");

                return false;
            }
            slot = map;

            return true;
        }
    }

    static bool parse_string(const Handle<JsonParser>& parser, const Handle<Interpreter>& interpreter, Value& slot)
    {
        StringBuilder buffer;

        for (;;)
        {
            int r = parser->ReadRune();

            if (r < 0)
            {
                parser->SetErrorMessage("Unterminated string; Missing '\"'");

                return false;
            }
            else if (r == '"')
            {
                slot = Value::NewString(buffer.ToString());

                return true;
            }
            else if (r == '\\')
            {
                switch (r = parser->ReadRune())
                {
                    case '"':
                    case '\\':
                    case '/':
                        buffer << r;
                        break;

                    case 'b':
                        buffer << '\010';
                        break;

                    case 'f':
                        buffer << '\f';
                        break;

                    case 'n':
                        buffer << '\n';
                        break;

                    case 'r':
                        buffer << '\r';
                        break;

                    case 't':
                        buffer << '\t';
                        break;

                    case 'u':
                    {
                        rune result = 0;

                        for (int i = 0; i < 4; ++i)
                        {
                            if (!std::isxdigit(r = parser->ReadRune()))
                            {
                                parser->SetErrorMessage("Malformed escape sequence");

                                return false;
                            }
                            if (r >= 'A' && r <= 'F')
                            {
                                result = result * 16 + (r - 'A' + 10);
                            }
                            else if (r >= 'a' && r <= 'f')
                            {
                                result = result * 16 + (r - 'a' + 10);
                            } else {
                                result = result * 16 + (r - '0');
                            }
                        }
                        buffer << result;
                        break;
                    }

                    default:
                        parser->SetErrorMessage("Malformed escape sequence");
                        return false;
                }
            } else {
                buffer << r;
            }
        }
    }

    static bool parse_number(const Handle<JsonParser>& parser,
                             const Handle<Interpreter>& interpreter,
                             Value& slot,
                             int initial)
    {
        StringBuilder buffer;
        bool is_float = false;
        int r;

        buffer << initial;
        if (initial == '-')
        {
            if (!std::isdigit(r = parser->ReadRune()))
            {
                parser->SetErrorMessage("Missing number after '-'");

                return false;
            }
            buffer << r;
        } else {
            r = initial;
        }
        if (r == '0')
        {
            if (!parser->PeekRune('.'))
            {
                slot = Value::NewInt(0);

                return true;
            }
        } else {
            for (;;)
            {
                if (!std::isdigit(parser->PeekRune()))
                {
                    break;
                }
                buffer << parser->ReadRune();
            }
        }
        if (parser->ReadRune('.'))
        {
            is_float = true;
            buffer << '.';
            for (;;)
            {
                if (!std::isdigit(parser->PeekRune()))
                {
                    break;
                }
                buffer << parser->ReadRune();
            }
        }
        if (parser->ReadRune('e') || parser->ReadRune('E'))
        {
            is_float = true;
            buffer << 'e';
            if (parser->PeekRune('+') || parser->PeekRune('-'))
            {
                buffer << parser->ReadRune();
            }
            if (!std::isdigit(r = parser->ReadRune()))
            {
                parser->SetErrorMessage("Invalid exponent");

                return false;
            }
            buffer << r;
            for (;;)
            {
                if (!std::isdigit(parser->PeekRune()))
                {
                    break;
                }
                buffer << parser->ReadRune();
            }
        }
        if (is_float)
        {
            double number;

            if (!Utils::ParseFloat(buffer.ToString(), number))
            {
                parser->SetErrorMessage("Float overflow/underflow");

                return false;
            }
            slot = Value::NewFloat(number);
        } else {
            i64 number;

            if (!Utils::ParseInt(buffer.ToString(), number, 10))
            {
                parser->SetErrorMessage("Integer overflow/underflow");

                return false;
            }
            slot = Value::NewInt(number);
        }

        return true;
    }
}
