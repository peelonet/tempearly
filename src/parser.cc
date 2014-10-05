#include <sstream>

#include "interpreter.h"
#include "parser.h"
#include "utils.h"

namespace tempearly
{
    static bool parse_text_block(const Handle<Interpreter>&, Parser*, std::vector<Handle<Node> >&, bool&);
    static bool parse_script_block(const Handle<Interpreter>&, Parser*, std::vector<Handle<Node> >&, bool&);
    static Handle<Node> parse_stmt(const Handle<Interpreter>&, Parser*);
    static Handle<Node> parse_expr(const Handle<Interpreter>&, Parser*);

    Parser::Parser(FILE* stream)
        : m_stream(stream)
        , m_seen_cr(false)
    {
        m_keywords.Insert("break", Token::KW_BREAK);
        m_keywords.Insert("continue", Token::KW_CONTINUE);
        m_keywords.Insert("do", Token::KW_DO);
        m_keywords.Insert("false", Token::KW_FALSE);
        m_keywords.Insert("for", Token::KW_FOR);
        m_keywords.Insert("if", Token::KW_IF);
        m_keywords.Insert("null", Token::KW_NULL);
        m_keywords.Insert("return", Token::KW_RETURN);
        m_keywords.Insert("throw", Token::KW_THROW);
        m_keywords.Insert("true", Token::KW_TRUE);
        m_keywords.Insert("while", Token::KW_WHILE);
    }

    Parser::~Parser()
    {
        Close();
    }

    void Parser::Close()
    {
        if (m_stream)
        {
            std::fclose(m_stream);
            m_stream = 0;
        }
    }

    int Parser::PeekChar()
    {
        if (m_pushback_chars.empty())
        {
            int c = ReadChar();

            if (c < 0)
            {
                return -1;
            }
            m_pushback_chars.push(c);

            return c;
        }

        return m_pushback_chars.front();
    }

    bool Parser::PeekChar(int c)
    {
        return PeekChar() == c;
    }

    int Parser::ReadChar()
    {
        if (!m_pushback_chars.empty())
        {
            int c = m_pushback_chars.front();

            m_pushback_chars.pop();

            return c;
        }
        if (m_stream)
        {
            int c = std::fgetc(m_stream);

            switch (c)
            {
                case '\r':
                    ++m_token.position.line;
                    m_token.position.column = 0;
                    m_seen_cr = true;
                    break;

                case '\n':
                    if (m_seen_cr)
                    {
                        m_seen_cr = false;
                    } else {
                        ++m_token.position.line;
                        m_token.position.column = 0;
                    }
                    break;

                default:
                    ++m_token.position.column;
                    if (m_seen_cr)
                    {
                        m_seen_cr = false;
                    }
            }

            return c;
        }

        return -1;
    }

    bool Parser::ReadChar(int expected)
    {
        int c = ReadChar();

        if (c == expected)
        {
            return true;
        }
        else if (c > 0)
        {
            m_pushback_chars.push(c);
        }

        return false;
    }

    void Parser::UnreadChar(int c)
    {
        if (c > 0)
        {
            m_pushback_chars.push(c);
        }
    }

    void Parser::SkipChar()
    {
        if (m_pushback_chars.empty())
        {
            ReadChar();
        } else {
            m_pushback_chars.pop();
        }
    }

    const Parser::TokenDescriptor& Parser::PeekToken()
    {
        if (m_pushback_tokens.empty())
        {
            m_pushback_tokens.push(ReadToken());
        }

        return m_pushback_tokens.front();
    }

    bool Parser::PeekToken(Token::Kind expected)
    {
        return PeekToken().kind == expected;
    }

    Parser::TokenDescriptor Parser::ReadToken()
    {
        TokenDescriptor token = m_token;
        int c;

        if (!m_pushback_tokens.empty())
        {
            token = m_pushback_tokens.front();
            m_pushback_tokens.pop();

            return token;
        }
READ_NEXT_CHAR:
        c = ReadChar();
        token.position.line = m_token.position.line;
        token.position.column = m_token.position.column;
        switch (c)
        {
            // End of input.
            case -1:
            case '\004':
            case '\032':
                token.kind = Token::END_OF_INPUT;
                break;

            // Whitespace.
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case '\f':
                goto READ_NEXT_CHAR;

            // Single line comment.
            case '#':
                while ((c = ReadChar()) != '\n' && c != '\r')
                {
                    if (c < 0)
                    {
                        token.kind = Token::END_OF_INPUT;

                        return token;
                    }
                }
                goto READ_NEXT_CHAR;

            case '(':
                token.kind = Token::LPAREN;
                break;

            case ')':
                token.kind = Token::RPAREN;
                break;

            case '[':
                token.kind = Token::LBRACK;
                break;

            case ']':
                token.kind = Token::RBRACK;
                break;

            case '{':
                token.kind = Token::LBRACE;
                break;

            case '}':
                token.kind = Token::RBRACE;
                break;

            case ':':
                token.kind = Token::COLON;
                break;

            case ';':
                token.kind = Token::SEMICOLON;
                break;

            case ',':
                token.kind = Token::COMMA;
                break;

            case '~':
                token.kind = Token::BIT_NOT;
                break;

            case '+':
                if (ReadChar('+'))
                {
                    token.kind = Token::INCREMENT;
                }
                else if (ReadChar('='))
                {
                    token.kind = Token::ASSIGN_ADD;
                } else {
                    token.kind = Token::ADD;
                }
                break;

            case '-':
                if (ReadChar('-'))
                {
                    token.kind = Token::DECREMENT;
                }
                else if (ReadChar('='))
                {
                    token.kind = Token::ASSIGN_SUB;
                } else {
                    token.kind = Token::SUB;
                }
                break;

            case '*':
                if (ReadChar('='))
                {
                    token.kind = Token::ASSIGN_MUL;
                } else {
                    token.kind = Token::MUL;
                }
                break;

            case '%':
                if (ReadChar('='))
                {
                    token.kind = Token::ASSIGN_MOD;
                }
                else if (ReadChar('>'))
                {
                    token.kind = Token::CLOSE_TAG;
                } else {
                    token.kind = Token::MOD;
                }
                break;

            case '/':
                // Single line comment?
                if (ReadChar('/'))
                {
                    while ((c = ReadChar()) != '\n' && c != '\r')
                    {
                        if (c < 0)
                        {
                            token.kind = Token::END_OF_INPUT;

                            return token;
                        }
                    }
                    goto READ_NEXT_CHAR;
                }
                // Multi-line comment?
                else if (ReadChar('*'))
                {
                    unsigned int depth = 1;

                    for (;;)
                    {
                        if ((c = ReadChar()) < 0)
                        {
                            std::stringstream ss;

                            ss << "Unterminated multi-line comment at "
                               << token.position.line
                               << "; Missing `*/'";
                            token.kind = Token::ERROR;
                            token.text = ss.str();

                            return token;
                        }
                        else if (c == '*')
                        {
                            if ((c = ReadChar()) == '/' && --depth == 0)
                            {
                                break;
                            }
                        }
                        else if (c == '/')
                        {
                            if ((c = ReadChar()) == '*')
                            {
                                ++depth;
                            }
                        }
                    }
                    goto READ_NEXT_CHAR;
                }
                else if (ReadChar('='))
                {
                    token.kind = Token::ASSIGN_DIV;
                } else {
                    token.kind = Token::DIV;
                }
                break;

            case '&':
                if (ReadChar('&'))
                {
                    if (ReadChar('='))
                    {
                        token.kind = Token::ASSIGN_AND;
                    } else {
                        token.kind = Token::AND;
                    }
                }
                else if (ReadChar('='))
                {
                    token.kind = Token::ASSIGN_BIT_AND;
                } else {
                    token.kind = Token::BIT_AND;
                }
                break;

            case '|':
                if (ReadChar('|'))
                {
                    if (ReadChar('='))
                    {
                        token.kind = Token::ASSIGN_OR;
                    } else {
                        token.kind = Token::OR;
                    }
                }
                else if (ReadChar('='))
                {
                    token.kind = Token::ASSIGN_BIT_OR;
                } else {
                    token.kind = Token::BIT_OR;
                }
                break;

            case '^':
                if (ReadChar('='))
                {
                    token.kind = Token::ASSIGN_BIT_XOR;
                } else {
                    token.kind = Token::BIT_XOR;
                }
                break;

            case '<':
                if (ReadChar('<'))
                {
                    if (ReadChar('='))
                    {
                        token.kind = Token::ASSIGN_LSH;
                    } else {
                        token.kind = Token::LSH;
                    }
                }
                else if (ReadChar('='))
                {
                    if (ReadChar('>'))
                    {
                        token.kind = Token::CMP;
                    } else {
                        token.kind = Token::LTE;
                    }
                } else {
                    token.kind = Token::LT;
                }
                break;

            case '>':
                if (ReadChar('>'))
                {
                    if (ReadChar('='))
                    {
                        token.kind = Token::ASSIGN_RSH;
                    } else {
                        token.kind = Token::RSH;
                    }
                }
                else if (ReadChar('='))
                {
                    token.kind = Token::GTE;
                } else {
                    token.kind = Token::GT;
                }
                break;

            case '!':
                if (ReadChar('='))
                {
                    token.kind = Token::NE;
                }
                else if (ReadChar('~'))
                {
                    token.kind = Token::NO_MATCH;
                } else {
                    token.kind = Token::NOT;
                }
                break;

            case '=':
                if (ReadChar('='))
                {
                    token.kind = Token::EQ;
                }
                else if (ReadChar('~'))
                {
                    token.kind = Token::MATCH;
                } else {
                    token.kind = Token::ASSIGN;
                }
                break;

            case '.':
                if (ReadChar('.'))
                {
                    if (ReadChar('.'))
                    {
                        token.kind = Token::DOT_DOT_DOT;
                    } else {
                        token.kind = Token::DOT_DOT;
                    }
                } else {
                    token.kind = Token::DOT;
                }
                break;

            case '?':
                if (ReadChar('.'))
                {
                    token.kind = Token::DOT_CONDITIONAL;
                } else {
                    token.kind = Token::CONDITIONAL;
                }
                break;

            case '\'':
            case '"':
            {
                const int separator = c;

                m_buffer.clear();
                for (;;)
                {
                    if ((c = ReadChar()) < 0)
                    {
                        std::stringstream ss;

                        ss << "Unterminated string literal at "
                           << token.position.line
                           << "; missing `"
                           << static_cast<char>(separator)
                           << "'";
                        token.kind = Token::ERROR;
                        token.text = ss.str();

                        return token;
                    }
                    else if (c == separator)
                    {
                        break;
                    }
                    else if (c == '\\')
                    {
                        // TODO: process escape sequence
                    } else {
                        m_buffer.append(1, static_cast<char>(c));
                    }
                }
                token.kind = Token::STRING;
                token.text = m_buffer;
                break;
            }

            case '0':
                m_buffer.assign(1, '0');
                switch (c = ReadChar())
                {
                    // Binary integer literal.
                    case 'b': case 'B':
                        m_buffer.append(1, 'b');
                        while (PeekChar('_') || std::isdigit(PeekChar()))
                        {
                            if ((c = ReadChar()) == '_')
                            {
                                continue;
                            }
                            else if (c != '0' && c != '1')
                            {
                                std::stringstream ss;

                                ss << "Invalid binary digit: "
                                   << static_cast<char>(c);
                                token.kind = Token::ERROR;
                                token.text = ss.str();

                                return token;
                            } else {
                                m_buffer.append(1, static_cast<char>(c));
                            }
                        }
                        token.kind = Token::INT;
                        token.text = m_buffer;
                        break;

                    // Hex integer literal.
                    case 'x': case 'X':
                        m_buffer.append(1, static_cast<char>(c));
                        while (PeekChar('_') || std::isxdigit(PeekChar()))
                        {
                            if ((c = ReadChar()) != '_')
                            {
                                m_buffer.append(1, static_cast<char>(c));
                            }
                        }
                        token.kind = Token::INT;
                        token.text = m_buffer;
                        break;

                    // Octal integer literal.
                    case 'o': case 'O':
                    case '0': case '1':
                    case '2': case '3':
                    case '4': case '5':
                    case '6': case '7':
                        m_buffer.append(1, static_cast<char>(c));
                        while (PeekChar('_') || std::isdigit(PeekChar()))
                        {
                            if ((c = ReadChar()) == '_')
                            {
                                continue;
                            }
                            else if (c > '7')
                            {
                                std::stringstream ss;

                                ss << "Invalid octal digit: "
                                   << static_cast<char>(c);
                                token.kind = Token::ERROR;
                                token.text = ss.str();

                                return token;
                            } else {
                                m_buffer.append(1, static_cast<char>(c));
                            }
                        }
                        token.kind = Token::INT;
                        token.text = m_buffer;
                        break;

                    case '8': case '9':
                    {
                        std::stringstream ss;

                        ss << "Invalid octal digit: " << static_cast<char>(c);
                        token.kind = Token::ERROR;
                        token.text = ss.str();

                        return token;
                    }

                    case 'e': case 'E':
                        goto SCAN_EXPONENT;

                    case '.':
                        goto SCAN_FLOAT;

                    case 'f': case 'F':
                        token.kind = Token::FLOAT;
                        token.text = m_buffer;
                        break;

                    default:
                        UnreadChar(c);
                        token.kind = Token::INT;
                        token.text = m_buffer;
                }
                break;

            case '1': case '2': case '3':
            case '4': case '5': case '6':
            case '7': case '8': case '9':
                m_buffer.assign(1, static_cast<char>(c));
                while (PeekChar('_') || std::isdigit(PeekChar()))
                {
                    if ((c = ReadChar()) != '_')
                    {
                        m_buffer.append(1, static_cast<char>(c));
                    }
                }
                if (PeekChar('.'))
                {
                    SkipChar();
SCAN_FLOAT:
                    m_buffer.append(1, '.');
                    if (std::isdigit(PeekChar()))
                    {
                        m_buffer.append(1, static_cast<char>(ReadChar()));
                        while (PeekChar('_') || std::isdigit(PeekChar()))
                        {
                            if ((c = ReadChar()) != '_')
                            {
                                m_buffer.append(1, static_cast<char>(c));
                            }
                        }
                        if (ReadChar('e') || ReadChar('E'))
                        {
SCAN_EXPONENT:
                            m_buffer.append(1, 'e');
                            if (PeekChar('+') || PeekChar('-'))
                            {
                                m_buffer.append(1, static_cast<char>(ReadChar()));
                                c = ReadChar(); // isdigit() might be a macro
                                if (!std::isdigit(c))
                                {
                                    token.kind = Token::ERROR;
                                    token.text = "Invalid exponent";

                                    return token;
                                }
                                m_buffer.append(1, static_cast<char>(c));
                            }
                            else if (std::isdigit(PeekChar()))
                            {
                                m_buffer.append(1, static_cast<char>(ReadChar()));
                            } else {
                                token.kind = Token::ERROR;
                                token.text = "Invalid exponent";

                                return token;
                            }
                            while (std::isdigit(PeekChar()))
                            {
                                m_buffer.append(1, static_cast<char>(ReadChar()));
                            }
                        }
                        token.kind = Token::FLOAT;
                        token.text = m_buffer;
                    } else {
                        UnreadChar('.');
                        m_buffer.erase(m_buffer.end() - 1);
                    }
                }
                else if (ReadChar('e') || ReadChar('E'))
                {
                    goto SCAN_EXPONENT;
                } else {
                    if (ReadChar('f') || ReadChar('F'))
                    {
                        token.kind = Token::FLOAT;
                    } else {
                        token.kind = Token::INT;
                    }
                    token.text = m_buffer;
                }
                break;

            default:
                if (c == '_' || std::isalpha(c))
                {
                    const Dictionary<Token::Kind>::Entry* entry;

                    m_buffer.assign(1, static_cast<char>(c));
                    while ((c = ReadChar()) == '_' || std::isalnum(c))
                    {
                        m_buffer.append(1, static_cast<char>(c));
                    }
                    UnreadChar(c);
                    if ((entry = m_keywords.Find(m_buffer)))
                    {
                        token.kind = entry->value;
                    } else {
                        token.kind = Token::IDENTIFIER;
                        token.text = m_buffer;
                    }
                } else {
                    token.kind = Token::ERROR;
                    token.text = "Unexpected input";
                }
        }

        return token;
    }

    bool Parser::ReadToken(Token::Kind expected)
    {
        if (PeekToken().kind == expected)
        {
            m_pushback_tokens.pop();

            return true;
        }

        return false;
    }

    void Parser::SkipToken()
    {
        if (!m_pushback_tokens.empty())
        {
            m_pushback_tokens.pop();
        }
    }

    Handle<Script> Parser::Compile(const Handle<Interpreter>& interpreter)
    {
        std::vector<Handle<Node> > nodes;

        // Skip leading shebang if such exists
        if (ReadChar('#'))
        {
            if (ReadChar('!'))
            {
                int c;

                while ((c = ReadChar()) != '\n' && c != '\r')
                {
                    if (c < 0)
                    {
                        return new Script(nodes);
                    }
                }
            } else {
                UnreadChar('#');
            }
        }
        for (;;)
        {
            bool should_continue = false;

            if (!parse_text_block(interpreter, this, nodes, should_continue))
            {
                return Handle<Script>();
            }
            else if (should_continue)
            {
                if (!parse_script_block(interpreter, this, nodes, should_continue))
                {
                    return Handle<Script>();
                }
                else if (!should_continue)
                {
                    break;
                }
            } else {
                break;
            }
        }

        return new Script(nodes);
    }

    static bool expect_token(const Handle<Interpreter>& interpreter,
                             Parser* parser,
                             Token::Kind expected)
    {
        Parser::TokenDescriptor token = parser->ReadToken();

        if (token.kind == Token::ERROR)
        {
            interpreter->Throw(interpreter->eSyntaxError, token.text);
        }
        else if (token.kind == expected)
        {
            return true;
        } else {
            interpreter->Throw(
                interpreter->eSyntaxError,
                String("Unexpected ")
                + Token::What(token.kind)
                + "; Missing "
                + Token::What(expected)
            );
        }

        return false;
    }

    static bool parse_text_block(const Handle<Interpreter>& interpreter,
                                 Parser* parser,
                                 std::vector<Handle<Node> >& nodes,
                                 bool& should_continue)
    {
        String text;
        int c = parser->ReadChar();

        while (c > 0)
        {
            if (c == '<')
            {
                if (parser->ReadChar('%'))
                {
                    if (!text.empty())
                    {
                        nodes.push_back(new TextNode(text));
                        text.clear();
                    }
                    should_continue = true;

                    return true;
                } else {
                    text.append(1, '<');
                    c = parser->ReadChar();
                }
            }
            else if (c == '$')
            {
                if ((c = parser->ReadChar()) == '{' || c == '!')
                {
                    const bool escape = c == '!';
                    Handle<Node> expr;

                    if (!text.empty())
                    {
                        nodes.push_back(new TextNode(text));
                        text.clear();
                    }
                    if (!(expr = parse_expr(interpreter, parser)))
                    {
                        return false;
                    }
                    if (parser->PeekToken(escape ? Token::NOT : Token::RBRACE))
                    {
                        parser->SkipToken();
                    }
                    else if (parser->ReadChar() != escape ? '!' : '}')
                    {
                        interpreter->Throw(
                            interpreter->eSyntaxError,
                            "Unterminated expression: Missing `}'"
                        );

                        return false;
                    }
                    nodes.push_back(new ExpressionNode(expr, escape));
                    c = parser->ReadChar();
                } else {
                    text.append(1, '$');
                }
            }
            else if (c == '\\')
            {
                if ((c = parser->ReadChar()) == '\r')
                {
                    if ((c = parser->ReadChar()) != '\n')
                    {
                        text.append(1, static_cast<char>(c));
                    }
                }
                else if (c == '\n')
                {
                    c = parser->ReadChar();
                }
                else if (c == '<')
                {
                    if ((c = parser->ReadChar()) == '%')
                    {
                        text.append("<%");
                        c = parser->ReadChar();
                    } else {
                        text.append("\\<");
                    }
                }
                else if (c == '$')
                {
                    if ((c = parser->ReadChar()) == '{' || c == '!')
                    {
                        text.append(1, '$');
                        text.append(1, static_cast<char>(c));
                        c = parser->ReadChar();
                    } else {
                        text.append("\\$");
                    }
                } else {
                    text.append(1, '\\');
                }
            } else {
                text.append(1, static_cast<char>(c));
                c = parser->ReadChar();
            }
        }
        if (!text.empty())
        {
            nodes.push_back(new TextNode(text));
        }
        should_continue = false;

        return true;
    }

    static bool parse_script_block(const Handle<Interpreter>& interpreter,
                                   Parser* parser,
                                   std::vector<Handle<Node> >& nodes,
                                   bool& should_continue)
    {
        for (;;)
        {
            const Parser::TokenDescriptor& token = parser->PeekToken();

            if (token.kind == Token::END_OF_INPUT)
            {
                should_continue = false;
                break;
            }
            else if (token.kind == Token::CLOSE_TAG)
            {
                parser->SkipToken();
                should_continue = true;
                break;
            }
            else if (token.kind == Token::SEMICOLON)
            {
                parser->SkipToken();
            } else {
                Handle<Node> stmt = parse_stmt(interpreter, parser);

                if (!stmt)
                {
                    return false;
                }
                nodes.push_back(stmt);
                should_continue = true;
            }
        }

        return true;
    }

    static Handle<Node> parse_stmt(const Handle<Interpreter>& interpreter,
                                   Parser* parser)
    {
        const Parser::TokenDescriptor& token = parser->PeekToken();
        Handle<Node> node;

        switch (token.kind)
        {
            case Token::ERROR:
                interpreter->Throw(interpreter->eSyntaxError, token.text);
                break;

            case Token::END_OF_INPUT:
                interpreter->Throw(
                    interpreter->eSyntaxError,
                    "Unexpected end of input; Missing statement"
                );
                break;

            case Token::SEMICOLON:
                parser->SkipToken();
                node = new EmptyNode();
                break;

            case Token::KW_IF: // TODO
            case Token::KW_WHILE: // TODO
            case Token::KW_FOR: // TODO

            case Token::KW_BREAK: // TODO
            case Token::KW_CONTINUE: // TODO
            case Token::KW_RETURN: // TODO
            case Token::KW_THROW: // TODO

            default:
                node = parse_expr(interpreter, parser);
                // TODO: read semicolon
        }

        return node;
    }

    static Handle<Node> parse_primary(const Handle<Interpreter>& interpreter,
                                      Parser* parser)
    {
        Parser::TokenDescriptor token = parser->ReadToken();
        Handle<Node> node;

        switch (token.kind)
        {
            case Token::ERROR:
                interpreter->Throw(interpreter->eSyntaxError, token.text);
                break;

            case Token::END_OF_INPUT:
                interpreter->Throw(
                    interpreter->eSyntaxError,
                    "Unexpected end of input; Missing expression"
                );
                break;

            case Token::KW_TRUE:
                node = new ValueNode(Value::NewBool(true));
                break;

            case Token::KW_FALSE:
                node = new ValueNode(Value::NewBool(false));
                break;

            case Token::KW_NULL:
                node = new ValueNode(Value::NullValue());
                break;

            case Token::STRING:
                node = new ValueNode(Value::NewString(token.text));
                break;

            case Token::INT:
            {
                i64 value;

                if (!Utils::ParseInt(token.text, value))
                {
                    interpreter->Throw(interpreter->eSyntaxError,
                                       "Integer overflow");

                    return Handle<Node>();
                }
                node = new ValueNode(Value::NewInt(value));
                break;
            }

            case Token::FLOAT:
            {
                double value;

                if (!Utils::ParseFloat(token.text, value))
                {
                    interpreter->Throw(interpreter->eSyntaxError,
                                       "Float overflow");

                    return Handle<Node>();
                }
                node = new ValueNode(Value::NewFloat(value));
                break;
            }

            case Token::LPAREN:
                if (!(node = parse_expr(interpreter, parser))
                    || !expect_token(interpreter, parser, Token::RPAREN))
                {
                    return Handle<Node>();
                }
                break;

            //TODO:case Token::LBRACK:
            //TODO:case Token::LBRACE:
            //TODO:case Token::IDENTIFIER:
            
            default:
                interpreter->Throw(
                    interpreter->eSyntaxError,
                    String("Unexpected ")
                    + Token::What(token.kind)
                    + "; Missing expression"
                );
                break;
        }

        return node;
    }

    static bool parse_args(const Handle<Interpreter>& interpreter,
                           Parser* parser,
                           std::vector<Handle<Node> >& args)
    {
        if (!expect_token(interpreter, parser, Token::LPAREN))
        {
            return false;
        }
        else if (parser->ReadToken(Token::RPAREN))
        {
            return true;
        }
        for (;;)
        {
            Handle<Node> node = parse_expr(interpreter, parser);

            if (!node)
            {
                return false;
            }
            args.push_back(node);
            if (parser->ReadToken(Token::COMMA))
            {
                continue;
            }
            else if (parser->ReadToken(Token::RPAREN))
            {
                return true;
            }
            interpreter->Throw(
                interpreter->eSyntaxError,
                "Unterminated argument list"
            );

            return false;
        }
    }

    static Handle<Node> parse_selection(const Handle<Interpreter>& interpreter,
                                        Parser* parser,
                                        const Handle<Node>& node,
                                        bool safe)
    {
        const Parser::TokenDescriptor token = parser->ReadToken();

        if (token.kind != Token::IDENTIFIER)
        {
            std::stringstream ss;

            ss << "Unexpected "
               << Token::What(token.kind)
               << "; Missing identifier";
            interpreter->Throw(interpreter->eSyntaxError, ss.str());

            return Handle<Node>();
        }
        else if (parser->PeekToken(Token::LPAREN))
        {
            std::vector<Handle<Node> > args;

            if (parse_args(interpreter, parser, args))
            {
                return new CallNode(node, token.text, args, safe);
            } else {
                return Handle<Node>();
            }
        } else {
            return new AttributeNode(node, token.text, safe);
        }
    }

    /**
     * primary-expression
     * postfix-expression "(" expression... ")"
     * postfix-expression "[" expression "]"
     * postfix-expression "." selection-expression
     * postfix-expression "?." selection-expression
     * postfix-expression "++"
     * postfix-expression "--"
     */
    static Handle<Node> parse_postfix(const Handle<Interpreter>& interpreter,
                                      Parser* parser)
    {
        Handle<Node> node = parse_primary(interpreter, parser);

        if (!node)
        {
            return Handle<Node>();
        }
        for (;;)
        {
            const Parser::TokenDescriptor& token = parser->PeekToken();

            if (token.kind == Token::LPAREN)
            {
                std::vector<Handle<Node> > args;

                if (!parse_args(interpreter, parser, args))
                {
                    return Handle<Node>();
                }
                node = new CallNode(node, "__call__", args);
            }
            else if (token.kind == Token::LBRACK)
            {
                Handle<Node> index;

                parser->SkipToken();
                if (!(index = parse_expr(interpreter, parser))
                    || !expect_token(interpreter, parser, Token::RBRACK))
                {
                    return Handle<Node>();
                }
                node = new SubscriptNode(node, index);
            }
            else if (token.kind == Token::DOT
                    || token.kind == Token::DOT_CONDITIONAL)
            {
                const bool safe = token.kind == Token::DOT_CONDITIONAL;

                parser->SkipToken();
                if (!(node = parse_selection(interpreter, parser, node, safe)))
                {
                    return Handle<Node>();
                }
            }
            else if (token.kind == Token::INCREMENT
                    || token.kind == Token::DECREMENT)
            {
                const PostfixNode::Kind kind = token.kind == Token::INCREMENT ?
                    PostfixNode::INCREMENT : PostfixNode::DECREMENT;

                parser->SkipToken();
                if (!node->IsVariable())
                {
                    interpreter->Throw(interpreter->eSyntaxError,
                                       "Node is not assignable");

                    return Handle<Node>();
                }
                node = new PostfixNode(node, kind);
            } else {
                return node;
            }
        }
    }

    /**
     * postfix-expression
     * "+" unary-expression
     * "-" unary-expression
     * "!" unary-expression
     * "~" unary-expression
     * "++" unary-expression
     * "--" unary-expression
     */
    static Handle<Node> parse_unary(const Handle<Interpreter>& interpreter,
                                    Parser* parser)
    {
        const Parser::TokenDescriptor& token = parser->PeekToken();
        Handle<Node> node;

        switch (token.kind)
        {
            case Token::ADD:
            case Token::SUB:
            case Token::BIT_NOT:
            {
                const Token::Kind kind = token.kind;
                Handle<Node> receiver;

                parser->SkipToken();
                if (!(receiver = parse_unary(interpreter, parser)))
                {
                    return Handle<Node>();
                }
                node = new CallNode(
                    receiver,
                    kind == Token::ADD ? "__pos__" :
                    kind == Token::SUB ? "__neg__" : "__invert__"
                );
                break;
            }

            case Token::NOT:
            {
                Handle<Node> condition;

                parser->SkipToken();
                if (!(condition = parse_unary(interpreter, parser)))
                {
                    return Handle<Node>();
                }
                node = new NotNode(condition);
                break;
            }

            case Token::INCREMENT:
            case Token::DECREMENT:
            {
                const PrefixNode::Kind kind = token.kind == Token::INCREMENT ?
                    PrefixNode::INCREMENT : PrefixNode::DECREMENT;

                parser->SkipToken();
                if (!(node = parse_unary(interpreter, parser)))
                {
                    return Handle<Node>();
                }
                else if (!node->IsVariable())
                {
                    interpreter->Throw(interpreter->eSyntaxError,
                                       "Node is not assignable");

                    return Handle<Node>();
                }
                node = new PrefixNode(node, kind);
                break;
            }

            default:
                node = parse_postfix(interpreter, parser);
        }

        return node;
    }

    /**
     * unary-expression
     * multiplicative-expression "*" unary-expression
     * multiplicative-expression "/" unary-expression
     * multiplicative-expression "%" unary-expression
     */
    static Handle<Node> parse_multiplicative(const Handle<Interpreter>& interpreter,
                                             Parser* parser)
    {
        Handle<Node> node = parse_unary(interpreter, parser);

        if (!node)
        {
            return Handle<Node>();
        }
        for (;;)
        {
            const Parser::TokenDescriptor& token = parser->PeekToken();

            if (token.kind == Token::MUL
                || token.kind == Token::DIV
                || token.kind == Token::MOD)
            {
                const Token::Kind kind = token.kind;
                Handle<Node> operand;

                parser->SkipToken();
                if (!(operand = parse_unary(interpreter, parser)))
                {
                    return Handle<Node>();
                }
                node = new CallNode(
                    node,
                    kind == Token::MUL ? "__mul__" :
                    kind == Token::DIV ? "__div__" : "__mod__",
                    std::vector<Handle<Node> >(1, operand)
                );
            } else {
                return node;
            }
        }
    }

    /**
     * multiplicative-expression
     * additive-expression "+" multiplicative-expression
     * additive-expression "-" multiplicative-expression
     */
    static Handle<Node> parse_additive(const Handle<Interpreter>& interpreter,
                                       Parser* parser)
    {
        Handle<Node> node = parse_multiplicative(interpreter, parser);

        if (!node)
        {
            return Handle<Node>();
        }
        for (;;)
        {
            const Parser::TokenDescriptor& token = parser->PeekToken();

            if (token.kind == Token::ADD || token.kind == Token::SUB)
            {
                const Token::Kind kind = token.kind;
                Handle<Node> operand;

                parser->SkipToken();
                if (!(operand = parse_multiplicative(interpreter, parser)))
                {
                    return Handle<Node>();
                }
                node = new CallNode(
                    node,
                    kind == Token::ADD ? "__add_" : "__sub__",
                    std::vector<Handle<Node> >(1, operand)
                );
            } else {
                return node;
            }
        }
    }

    /**
     * additive-expression
     * shift-expression "<<" additive-expression
     * shift-expression ">>" additive-expression
     */
    static Handle<Node> parse_shift(const Handle<Interpreter>& interpreter,
                                    Parser* parser)
    {
        Handle<Node> node = parse_additive(interpreter, parser);

        if (!node)
        {
            return Handle<Node>();
        }
        for (;;)
        {
            const Parser::TokenDescriptor& token = parser->PeekToken();

            if (token.kind == Token::LSH || token.kind == Token::RSH)
            {
                const Token::Kind kind = token.kind;
                Handle<Node> operand;

                parser->SkipToken();
                if (!(operand = parse_additive(interpreter, parser)))
                {
                    return Handle<Node>();
                }
                node = new CallNode(
                    node,
                    kind == Token::LSH ? "__lsh__" : "__rsh__",
                    std::vector<Handle<Node> >(1, operand)
                );
            } else {
                return node;
            }
        }
    }

    /**
     * shift-expression
     * bit-and-expression "&" shift-expression
     */
    static Handle<Node> parse_bit_and(const Handle<Interpreter>& interpreter,
                                      Parser* parser)
    {
        Handle<Node> node = parse_shift(interpreter, parser);

        if (!node)
        {
            return Handle<Node>();
        }
        while (parser->ReadToken(Token::BIT_AND))
        {
            Handle<Node> operand = parse_shift(interpreter, parser);

            if (!operand)
            {
                return Handle<Node>();
            }
            node = new CallNode(
                node,
                "__and__",
                std::vector<Handle<Node> >(1, operand)
            );
        }

        return node;
    }

    /**
     * bit-and-expression
     * bit-xor-expression "^" bit-and-expression
     */
    static Handle<Node> parse_bit_xor(const Handle<Interpreter>& interpreter,
                                      Parser* parser)
    {
        Handle<Node> node = parse_bit_and(interpreter, parser);

        if (!node)
        {
            return Handle<Node>();
        }
        while (parser->ReadToken(Token::BIT_XOR))
        {
            Handle<Node> operand = parse_bit_and(interpreter, parser);

            if (!operand)
            {
                return Handle<Node>();
            }
            node = new CallNode(
                node,
                "__xor__",
                std::vector<Handle<Node> >(1, operand)
            );
        }

        return node;
    }

    /**
     * bit-xor-expression
     * bit-or-expression "|" bit-xor-expression
     */
    static Handle<Node> parse_bit_or(const Handle<Interpreter>& interpreter,
                                     Parser* parser)
    {
        Handle<Node> node = parse_bit_xor(interpreter, parser);

        if (!node)
        {
            return Handle<Node>();
        }
        while (parser->ReadToken(Token::BIT_OR))
        {
            Handle<Node> operand = parse_bit_xor(interpreter, parser);

            if (!operand)
            {
                return Handle<Node>();
            }
            node = new CallNode(
                node,
                "__or__",
                std::vector<Handle<Node> >(1, operand)
            );
        }

        return node;
    }

    /**
     * bit-or-expression
     * relational-expression "<" bit-or-expression
     * relational-expression ">" bit-or-expression
     * relational-expression "<=" bit-or-expression
     * relational-expression ">=" bit-or-expression
     */
    static Handle<Node> parse_rational(const Handle<Interpreter>& interpreter,
                                       Parser* parser)
    {
        Handle<Node> node = parse_bit_or(interpreter, parser);

        if (!node)
        {
            return Handle<Node>();
        }
        // TODO

        return node;
    }

    /**
     * relational-expression
     * equality-expression "==" relational-expression
     * equality-expression "!=" relational-expression
     * equality-expression "=~" relational-expression
     * equality-expression "!~" relational-expression
     * equality-expression "<=>" relational-expression
     */
    static Handle<Node> parse_equality(const Handle<Interpreter>& interpreter,
                                       Parser* parser)
    {
        Handle<Node> node = parse_rational(interpreter, parser);

        if (!node)
        {
            return Handle<Node>();
        }
        // TODO

        return node;
    }

    /**
     * equality-expression
     * logical-and-expression "&&" equality-expression
     */
    static Handle<Node> parse_logical_and(const Handle<Interpreter>& interpreter,
                                          Parser* parser)
    {
        Handle<Node> node = parse_equality(interpreter, parser);

        if (!node)
        {
            return Handle<Node>();
        }
        while (parser->ReadToken(Token::AND))
        {
            Handle<Node> operand = parse_equality(interpreter, parser);

            if (operand)
            {
                node = new AndNode(node, operand);
            } else {
                return Handle<Node>();
            }
        }

        return node;
    }

    /**
     * logical-and-expression
     * logical-or-expression "||" logical-and-expression
     */
    static Handle<Node> parse_logical_or(const Handle<Interpreter>& interpreter,
                                         Parser* parser)
    {
        Handle<Node> node = parse_logical_and(interpreter, parser);

        if (!node)
        {
            return Handle<Node>();
        }
        while (parser->ReadToken(Token::OR))
        {
            Handle<Node> operand = parse_logical_and(interpreter, parser);

            if (operand)
            {
                node = new OrNode(node, operand);
            } else {
                return Handle<Node>();
            }
        }

        return node;
    }

    /**
     * logical-or-expression
     * range-expression ".." logical-or-expression
     * range-expression "..." logical-or-expression
     */
    static Handle<Node> parse_range(const Handle<Interpreter>& interpreter,
                                    Parser* parser)
    {
        Handle<Node> node = parse_logical_or(interpreter, parser);

        if (!node)
        {
            return Handle<Node>();
        }
        // TODO

        return node;
    }

    /**
     * range-expression
     * range-expression "?" expression ":" expression
     */
    static Handle<Node> parse_ternary(const Handle<Interpreter>& interpreter,
                                      Parser* parser)
    {
        Handle<Node> node = parse_range(interpreter, parser);

        if (!node)
        {
            return Handle<Node>();
        }
        // TODO

        return node;
    }

    /**
     * ternary-expression
     * assignment-expression "=" expression
     * assignment-expression "&&=" expression
     * assignment-expression "||=" expression
     * assignment-expression "&=" expression
     * assignment-expression "|=" expression
     * assignment-expression "^=" expression
     * assignment-expression "<<=" expression
     * assignment-expression ">>=" expression
     * assignment-expression "+=" expression
     * assignment-expression "-=" expression
     * assignment-expression "*=" expression
     * assignment-expression "/=" expression
     * assignment-expression "%=" expression
     * assignment-expression "**=" expression
     */
    static Handle<Node> parse_expr(const Handle<Interpreter>& interpreter,
                                   Parser* parser)
    {
        Handle<Node> node = parse_ternary(interpreter, parser);

        if (!node)
        {
            return Handle<Node>();
        }
        // TODO

        return node;
    }
}
