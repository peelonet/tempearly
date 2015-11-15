#include "interpreter.h"
#include "core/bytestring.h"
#include "io/stream.h"
#include "sapi/repl/request.h"
#include "sapi/repl/response.h"
#include "script/parser.h"

using namespace tempearly;

static ByteString repl_read_expr(int&);
static void repl_read_line(int&, Vector<byte>&);
static void repl_show_exception(const Handle<Interpreter>&);
static void count_open_chars(const Vector<byte>&, Vector<byte>&);

int main(int argc, char** argv)
{
    Handle<Interpreter> interpreter;
    int line_counter = 0;

    interpreter = new Interpreter(new ReplRequest(), new ReplResponse());
    interpreter->Initialize();
    interpreter->PushFrame();
    for (;;)
    {
        const ByteString line = repl_read_expr(line_counter);
        Handle<ScriptParser> parser;
        Handle<Script> script;

        // Skip empty lines.
        if (line.IsEmpty())
        {
            continue;
        }
        parser = new ScriptParser(line.AsStream());
        if ((script = parser->CompileExpression()))
        {
            Handle<Object> result;

            if (script->Evaluate(interpreter, result))
            {
                // Skip null values.
                if (!result->IsNull())
                {
                    String repr;

                    if (result->ToString(interpreter, repr))
                    {
                        std::printf("=> %s\n", repr.Encode().c_str());
                    } else {
                        repl_show_exception(interpreter);
                    }
                }
            } else {
                repl_show_exception(interpreter);
            }
        } else {
            std::printf(
                "SyntaxError: %s\n",
                parser->GetErrorMessage().Encode().c_str()
            );
        }
    }

    return EXIT_SUCCESS;
}

static ByteString repl_read_expr(int& line_counter)
{
    Vector<byte> buffer;
    Vector<byte> line;
    Vector<byte> open_chars;

    for (;;)
    {
        repl_read_line(line_counter, line);
        count_open_chars(line, open_chars);
        if (!buffer.IsEmpty())
        {
            buffer.PushBack(static_cast<byte>('\n'));
        }
        buffer.PushBack(line.GetData(), line.GetSize());
        line.Clear();
        if (open_chars.IsEmpty())
        {
            return ByteString(buffer.GetData(), buffer.GetSize());
        }
    }
}

static void repl_read_line(int& line_counter, Vector<byte>& buffer)
{
    std::printf("tempearly:%03d> ", ++line_counter);
    for (;;)
    {
        const int c = std::fgetc(stdin);

        if (c == EOF)
        {
            std::exit(EXIT_SUCCESS);
        }
        else if (c == '\n')
        {
            return;
        } else {
            buffer.PushBack(c);
        }
    }
}

static void repl_show_exception(const Handle<Interpreter>& interpreter)
{
    const Handle<ExceptionObject> exception(interpreter->GetException());

    if (!exception)
    {
        return;
    }
    interpreter->ClearException();
    std::printf(
        "%s: %s\n",
        exception->GetClass(interpreter)->GetName().Encode().c_str(),
        exception->GetMessage().Encode().c_str()
    );
    for (Handle<Frame> frame = exception->GetFrame();
         frame;
         frame = frame->GetPrevious())
    {
        const Handle<FunctionObject> function = frame->GetFunction();

        std::printf(
            "\t%s\n",
            function ? function->GetName().Encode().c_str() : "<eval>"
        );
    }
}

static void count_open_chars(const Vector<byte>& input,
                             Vector<byte>& open_chars)
{
    const std::size_t size = input.GetSize();

    for (std::size_t i = 0; i < size; ++i)
    {
        if (!open_chars.IsEmpty())
        {
            const byte b = open_chars.GetBack();

            if (b == '"' || b == '\'')
            {
                while (i < size)
                {
                    if (input[i] == b)
                    {
                        open_chars.Erase(open_chars.GetSize() - 1);
                        ++i;
                        break;
                    }
                    else if (input[i] == '\\' && i + 1 < size)
                    {
                        if (input[++i] == b)
                        {
                            ++i;
                        }
                    } else {
                        ++i;
                    }
                }
                if (i >= size)
                {
                    break;
                }
            }
            else if (b == '*')
            {
                while (i < size)
                {
                    if (input[i] == '*'
                        && i + 1 < size
                        && input[i + 1] == '/')
                    {
                        open_chars.Erase(open_chars.GetSize() - 1);
                        ++i;
                        break;
                    } else {
                        ++i;
                    }
                }
            }
        }
        switch (input[i])
        {
            case '#':
                return;

            case '/':
                if (i + 1 < size && input[i + 1] == '*')
                {
                    open_chars.PushBack('*');
                    ++i;
                }
                break;

            case '(': open_chars.PushBack(')'); break;
            case '[': open_chars.PushBack(']'); break;
            case '{': open_chars.PushBack('}'); break;

            case ')': case ']': case '}':
                if (!open_chars.IsEmpty() && open_chars.GetBack() == input[i])
                {
                    open_chars.Erase(open_chars.GetSize() - 1);
                }
                break;

            case '\'': case '"':
            {
                const byte separator = input[i];

                open_chars.PushBack(separator);
                ++i;
                while (i < size)
                {
                    if (input[i] == separator)
                    {
                        open_chars.Erase(open_chars.GetSize() - 1);
                        break;
                    }
                    else if (input[i] == '\\')
                    {
                        if (++i < size && input[i] == '\'')
                        {
                            ++i;
                        }
                    } else {
                        ++i;
                    }
                }
            }
        }
    }
}
