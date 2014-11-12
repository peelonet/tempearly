#include <cctype>

#include "parameter.h"
#include "parser.h"
#include "utils.h"
#include "io/stream.h"

namespace tempearly
{
    static bool parse_text_block(const Handle<Parser>&, Vector<Handle<Node> >&, bool&);
    static bool parse_script_block(const Handle<Parser>&, Vector<Handle<Node> >&, bool&);
    static Handle<Node> parse_stmt(const Handle<Parser>&);
    static Handle<Node> parse_expr(const Handle<Parser>&);
    static Handle<Node> parse_postfix(const Handle<Parser>&);

    Parser::Parser(const Handle<Stream>& stream)
        : m_stream(stream.Get())
        , m_seen_cr(false)
    {
        m_keywords.Insert("break", Token::KW_BREAK);
        m_keywords.Insert("continue", Token::KW_CONTINUE);
        m_keywords.Insert("do", Token::KW_DO);
        m_keywords.Insert("else", Token::KW_ELSE);
        m_keywords.Insert("end", Token::KW_END);
        m_keywords.Insert("false", Token::KW_FALSE);
        m_keywords.Insert("for", Token::KW_FOR);
        m_keywords.Insert("function", Token::KW_FUNCTION);
        m_keywords.Insert("if", Token::KW_IF);
        m_keywords.Insert("null", Token::KW_NULL);
        m_keywords.Insert("return", Token::KW_RETURN);
        m_keywords.Insert("throw", Token::KW_THROW);
        m_keywords.Insert("true", Token::KW_TRUE);
        m_keywords.Insert("while", Token::KW_WHILE);
    }

    Parser::~Parser()
    {
        if (m_stream)
        {
            m_stream->Close();
        }
    }

    Handle<Script> Parser::Compile()
    {
        Handle<Parser> handle = this;
        Vector<Handle<Node> > nodes;

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

            if (!parse_text_block(handle, nodes, should_continue))
            {
                return Handle<Script>();
            }
            else if (should_continue)
            {
                if (!parse_script_block(handle, nodes, should_continue))
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

    void Parser::Close()
    {
        if (m_stream)
        {
            m_stream->Close();
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
            rune slot;

            if (!m_stream->ReadRune(slot))
            {
                m_stream->Close();
                m_stream = 0;

                return -1;
            }
            switch (slot)
            {
                case '\r':
                    ++m_position.line;
                    m_position.column = 0;
                    m_seen_cr = true;
                    break;

                case '\n':
                    if (m_seen_cr)
                    {
                        m_seen_cr = false;
                    } else {
                        ++m_position.line;
                        m_position.column = 0;
                    }
                    break;

                default:
                    ++m_position.column;
                    if (m_seen_cr)
                    {
                        m_seen_cr = false;
                    }
            }

            return slot;
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
        TokenDescriptor token;
        int c;

        if (!m_pushback_tokens.empty())
        {
            token = m_pushback_tokens.front();
            m_pushback_tokens.pop();

            return token;
        }
READ_NEXT_CHAR:
        c = ReadChar();
        token.position.line = m_position.line;
        token.position.column = m_position.column;
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

            // Invalid Unicode code point.
            case 0xfffd:
                m_error_message = "Malformed UTF-8 input";
                token.kind = Token::ERROR;
                break;

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
                else if (ReadChar('}'))
                {
                    token.kind = Token::CLOSE_TAG;
                    // Eat possible new line following the close tag.
                    ReadChar('\r');
                    ReadChar('\n');
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
                            StringBuilder sb;

                            sb << "Unterminated multi-line comment at "
                               << Utils::ToString(static_cast<i64>(token.position.line))
                               << "; Missing `*/'";
                            m_error_message = sb.ToString();
                            token.kind = Token::ERROR;

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
                    token.kind = Token::LTE;
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
                }
                else if (ReadChar('>'))
                {
                    token.kind = Token::ARROW;
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

                m_buffer.Clear();
                for (;;)
                {
                    if ((c = ReadChar()) < 0)
                    {
                        StringBuilder sb;

                        sb << "Unterminated string literal at "
                           << Utils::ToString(static_cast<i64>(token.position.line))
                           << "; missing `"
                           << separator
                           << "'";
                        m_error_message = sb.ToString();
                        token.kind = Token::ERROR;

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
                        m_buffer << c;
                    }
                }
                token.kind = Token::STRING;
                token.text = m_buffer.ToString();
                break;
            }

            case '0':
                m_buffer.Assign(1, '0');
                switch (c = ReadChar())
                {
                    // Binary integer literal.
                    case 'b': case 'B':
                        m_buffer << 'b';
                        while (PeekChar('_') || std::isdigit(PeekChar()))
                        {
                            if ((c = ReadChar()) == '_')
                            {
                                continue;
                            }
                            else if (c != '0' && c != '1')
                            {
                                StringBuilder sb;

                                sb << "Invalid binary digit: " << c;
                                m_error_message = sb.ToString();
                                token.kind = Token::ERROR;

                                return token;
                            } else {
                                m_buffer << c;
                            }
                        }
                        token.kind = Token::INT;
                        token.text = m_buffer.ToString();
                        break;

                    // Hex integer literal.
                    case 'x': case 'X':
                        m_buffer << 'x';
                        while (PeekChar('_') || std::isxdigit(PeekChar()))
                        {
                            if ((c = ReadChar()) != '_')
                            {
                                m_buffer << c;
                            }
                        }
                        token.kind = Token::INT;
                        token.text = m_buffer.ToString();
                        break;

                    // Octal integer literal.
                    case 'o': case 'O':
                    case '0': case '1':
                    case '2': case '3':
                    case '4': case '5':
                    case '6': case '7':
                        m_buffer << c;
                        while (PeekChar('_') || std::isdigit(PeekChar()))
                        {
                            if ((c = ReadChar()) == '_')
                            {
                                continue;
                            }
                            else if (c > '7')
                            {
                                StringBuilder sb;

                                sb << "Invalid octal digit: " << c;
                                m_error_message = sb.ToString();
                                token.kind = Token::ERROR;

                                return token;
                            } else {
                                m_buffer << c;
                            }
                        }
                        token.kind = Token::INT;
                        token.text = m_buffer.ToString();
                        break;

                    case '8': case '9':
                    {
                        StringBuilder sb;

                        sb << "Invalid octal digit: " << c;
                        m_error_message = sb.ToString();
                        token.kind = Token::ERROR;

                        return token;
                    }

                    case 'e': case 'E':
                        goto SCAN_EXPONENT;

                    case '.':
                        goto SCAN_FLOAT;

                    case 'f': case 'F':
                        token.kind = Token::FLOAT;
                        token.text = m_buffer.ToString();
                        break;

                    default:
                        UnreadChar(c);
                        token.kind = Token::INT;
                        token.text = m_buffer.ToString();
                }
                break;

            case '1': case '2': case '3':
            case '4': case '5': case '6':
            case '7': case '8': case '9':
                m_buffer.Assign(1, c);
                while (PeekChar('_') || std::isdigit(PeekChar()))
                {
                    if ((c = ReadChar()) != '_')
                    {
                        m_buffer << c;
                    }
                }
                if (PeekChar('.'))
                {
                    SkipChar();
SCAN_FLOAT:
                    if (std::isdigit(PeekChar()))
                    {
                        m_buffer << '.' << ReadChar();
                        while (PeekChar('_') || std::isdigit(PeekChar()))
                        {
                            if ((c = ReadChar()) != '_')
                            {
                                m_buffer << c;
                            }
                        }
                        if (ReadChar('e') || ReadChar('E'))
                        {
SCAN_EXPONENT:
                            m_buffer << 'e';
                            if (PeekChar('+') || PeekChar('-'))
                            {
                                m_buffer << ReadChar();
                                c = ReadChar(); // isdigit() might be a macro
                                if (!std::isdigit(c))
                                {
                                    m_error_message = "Invalid exponent";
                                    token.kind = Token::ERROR;

                                    return token;
                                }
                                m_buffer << c;
                            }
                            else if (std::isdigit(PeekChar()))
                            {
                                m_buffer << ReadChar();
                            } else {
                                m_error_message = "Invalid exponent";
                                token.kind = Token::ERROR;

                                return token;
                            }
                            while (std::isdigit(PeekChar()))
                            {
                                m_buffer << ReadChar();
                            }
                        }
                        token.kind = Token::FLOAT;
                        token.text = m_buffer.ToString();
                    } else {
                        UnreadChar('.');
                        token.kind = Token::INT;
                        token.text = m_buffer.ToString();
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
                    token.text = m_buffer.ToString();
                }
                break;

            default:
                if (c == '_' || std::isalpha(c))
                {
                    const Dictionary<Token::Kind>::Entry* entry;
                    String string;

                    m_buffer.Assign(1, c);
                    while ((c = ReadChar()) == '_' || std::isalnum(c))
                    {
                        m_buffer << c;
                    }
                    UnreadChar(c);
                    string = m_buffer.ToString();
                    if ((entry = m_keywords.Find(string)))
                    {
                        token.kind = entry->GetValue();
                    } else {
                        token.kind = Token::IDENTIFIER;
                        token.text = string;
                    }
                } else {
                    m_error_message = "Unexpected input";
                    token.kind = Token::ERROR;
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

    void Parser::Mark()
    {
        CountedObject::Mark();
        if (m_stream && !m_stream->IsMarked())
        {
            m_stream->Mark();
        }
    }

    static bool expect_token(const Handle<Parser>& parser, Token::Kind expected)
    {
        Parser::TokenDescriptor token = parser->ReadToken();

        if (token.kind == expected)
        {
            return true;
        }
        else if (token.kind != Token::ERROR)
        {
            parser->SetErrorMessage(String("Unexpected ")
                                    + Token::What(token.kind)
                                    + "; Missing "
                                    + Token::What(expected));
        }

        return false;
    }

    static bool parse_text_block(const Handle<Parser>& parser, Vector<Handle<Node> >& nodes, bool& should_continue)
    {
        StringBuilder text;
        int c = parser->ReadChar();

        while (c > 0)
        {
            if (c == '{')
            {
                if ((c = parser->ReadChar()) == '%')
                {
                    if (!text.IsEmpty())
                    {
                        nodes.PushBack(new TextNode(text.ToString()));
                        text.Clear();
                    }
                    should_continue = true;

                    return true;
                }
                else if (c == '{' || c == '!')
                {
                    const bool escape = c != '!';
                    Handle<Node> expr;

                    if (!text.IsEmpty())
                    {
                        nodes.PushBack(new TextNode(text.ToString()));
                        text.Clear();
                    }
                    if (!(expr = parse_expr(parser)))
                    {
                        return false;
                    }
                    if (parser->PeekToken(escape ? Token::RBRACE : Token::NOT))
                    {
                        parser->SkipToken();
                        if (parser->ReadChar() != '}')
                        {
                            if (escape)
                            {
                                parser->SetErrorMessage("Unterminated expression: Missing '}}'");
                            } else {
                                parser->SetErrorMessage("Unterminated expression: Missing '!}'");
                            }

                            return false;
                        }
                    }
                    else if (parser->ReadChar() == (escape ? '}' : '!'))
                    {
                        if (escape)
                        {
                            parser->SetErrorMessage("Unterminated expression: Missing '}}'");
                        } else {
                            parser->SetErrorMessage("Unterminated expression: Missing '!}'");
                        }

                        return false;
                    }
                    nodes.PushBack(new ExpressionNode(expr, escape));
                    c = parser->ReadChar();
                }
                else if (c == '#')
                {
                    c = parser->ReadChar();
                    for (;;)
                    {
                        if (c < 0)
                        {
                            parser->SetErrorMessage("Unterminated comment: Missing '#}'");

                            return false;
                        }
                        else if (c == '#')
                        {
                            if ((c = parser->ReadChar()) == '}')
                            {
                                c = parser->ReadChar();
                                break;
                            }
                        } else {
                            c = parser->ReadChar();
                        }
                    }
                } else {
                    text << '{';
                }
            }
            else if (c == '\\')
            {
                if ((c = parser->ReadChar()) == '\r')
                {
                    if ((c = parser->ReadChar()) != '\n')
                    {
                        text << c;
                    }
                }
                else if (c == '\n')
                {
                    c = parser->ReadChar();
                }
                else if (c == '{')
                {
                    text << '{';
                    c = parser->ReadChar();
                } else {
                    text << '\\';
                }
            } else {
                text << c;
                c = parser->ReadChar();
            }
        }
        if (!text.IsEmpty())
        {
            nodes.PushBack(new TextNode(text.ToString()));
        }
        should_continue = false;

        return true;
    }

    static bool parse_script_block(const Handle<Parser>& parser, Vector<Handle<Node> >& nodes, bool& should_continue)
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
                Handle<Node> stmt = parse_stmt(parser);

                if (!stmt)
                {
                    return false;
                }
                nodes.PushBack(stmt);
                should_continue = true;
            }
        }

        return true;
    }

    static Handle<Node> parse_block(const Handle<Parser>& parser)
    {
        Vector<Handle<Node> > nodes;

        if (parser->ReadToken(Token::CLOSE_TAG))
        {
            for (;;)
            {
                bool should_continue = false;

                if (!parse_text_block(parser, nodes, should_continue))
                {
                    return Handle<Node>();
                }
                else if (should_continue)
                {
                    if (parser->PeekToken(Token::KW_END) || parser->PeekToken(Token::KW_ELSE))
                    {
                        break;
                    }
                    else if (!parse_script_block(parser, nodes, should_continue))
                    {
                        return Handle<Node>();
                    }
                    else if (!should_continue)
                    {
                        break;
                    }
                } else {
                    break;
                }
            }
        } else {
            while (!parser->PeekToken(Token::KW_END) && !parser->PeekToken(Token::KW_ELSE))
            {
                Handle<Node> statement = parse_stmt(parser);

                if (!statement)
                {
                    return Handle<Node>();
                }
                nodes.PushBack(statement);
            }
        }
        switch (nodes.GetSize())
        {
            case 0:
                return new EmptyNode();

            case 1:
                return nodes[0];

            default:
                return new BlockNode(nodes);
        }
    }

    static Handle<Node> parse_if(const Handle<Parser>& parser)
    {
        Handle<Node> condition;
        Handle<Node> then_statement;
        Handle<Node> else_statement;

        if (!expect_token(parser, Token::KW_IF)
            || !(condition = parse_expr(parser))
            || !expect_token(parser, Token::COLON)
            || !(then_statement = parse_block(parser)))
        {
            return Handle<Node>();
        }
        if (parser->ReadToken(Token::KW_ELSE))
        {
            if (parser->PeekToken(Token::KW_IF))
            {
                else_statement = parse_if(parser);
            }
            else if (!expect_token(parser, Token::COLON)
                    || !(else_statement = parse_block(parser))
                    || !expect_token(parser, Token::KW_END)
                    || !expect_token(parser, Token::KW_IF))
            {
                return Handle<Node>();
            }
        }
        else if (!expect_token(parser, Token::KW_END) || !expect_token(parser, Token::KW_IF))
        {
            return Handle<Node>();
        }
        parser->ReadToken(Token::SEMICOLON); // Eat optional semicolon

        return new IfNode(condition, then_statement, else_statement);
    }

    static Handle<Node> parse_while(const Handle<Parser>& parser)
    {
        Handle<Node> condition;
        Handle<Node> statement;

        if (!expect_token(parser, Token::KW_IF)
            || !(condition = parse_expr(parser))
            || !expect_token(parser, Token::COLON)
            || !(statement = parse_block(parser))
            || !expect_token(parser, Token::KW_END)
            || !expect_token(parser, Token::KW_WHILE))
        {
            return Handle<Node>();
        }
        parser->ReadToken(Token::SEMICOLON); // Eat optional semicolon

        return new WhileNode(condition, statement);
    }

    static Handle<Node> parse_for(const Handle<Parser>& parser)
    {
        Handle<Node> variable;
        Handle<Node> collection;
        Handle<Node> statement;

        if (!expect_token(parser, Token::KW_FOR) || !(variable = parse_expr(parser)))
        {
            return Handle<Node>();
        }
        if (!variable->IsVariable())
        {
            parser->SetErrorMessage("'for' loop requires variable");

            return Handle<Node>();
        }
        if (!expect_token(parser, Token::COLON)
            || !(collection = parse_expr(parser))
            || !expect_token(parser, Token::COLON)
            || !(statement = parse_block(parser))
            || !expect_token(parser, Token::KW_END)
            || !expect_token(parser, Token::KW_FOR))
        {
            return Handle<Node>();
        }
        parser->ReadToken(Token::SEMICOLON); // Eat optional semicolon

        return new ForNode(variable, collection, statement);
    }

    static Handle<Node> parse_stmt(const Handle<Parser>& parser)
    {
        const Parser::TokenDescriptor& token = parser->PeekToken();
        Handle<Node> node;

        switch (token.kind)
        {
            case Token::ERROR:
                return Handle<Node>();

            case Token::END_OF_INPUT:
                parser->SetErrorMessage("Unexpected end of input; Missing statement");
                return Handle<Node>();

            case Token::SEMICOLON:
                parser->SkipToken();
                return new EmptyNode();

            case Token::KW_IF:
                return parse_if(parser);

            case Token::KW_WHILE:
                return parse_while(parser);

            case Token::KW_FOR:
                return parse_for(parser);

            case Token::KW_BREAK:
                node = new BreakNode();
                break;

            case Token::KW_CONTINUE:
                node = new ContinueNode();
                break;

            case Token::KW_RETURN:
            {
                Handle<Node> value;

                parser->SkipToken();
                if (!parser->PeekToken(Token::SEMICOLON))
                {
                    if (!(value = parse_expr(parser)))
                    {
                        return Handle<Node>();
                    }
                }
                node = new ReturnNode(value);
                break;
            }

            //TODO:case Token::KW_THROW:

            default:
                node = parse_expr(parser);
        }
        if (expect_token(parser, Token::SEMICOLON))
        {
            return node;
        } else {
            return Handle<Node>();
        }
    }

    static Handle<Node> parse_list(const Handle<Parser>& parser)
    {
        Vector<Handle<Node> > elements;

        if (!parser->ReadToken(Token::RBRACK))
        {
            for (;;)
            {
                Handle<Node> node = parse_expr(parser);

                if (!node)
                {
                    return Handle<Node>();
                }
                elements.PushBack(node);
                if (parser->ReadToken(Token::COMMA))
                {
                    continue;
                }
                else if (parser->ReadToken(Token::RBRACK))
                {
                    break;
                }
                parser->SetErrorMessage("Unterminated list literal");

                return Handle<Node>();
            }
        }

        return new ListNode(elements);
    }

    static Handle<Node> parse_map(const Handle<Parser>& parser)
    {
        Vector<Pair<Handle<Node> > > entries;
        Handle<Node> key;
        Handle<Node> value;

        if (!parser->ReadToken(Token::RBRACE))
        {
            for (;;)
            {
                if (!(key = parse_expr(parser))
                    || !expect_token(parser, Token::COLON)
                    || !(value = parse_expr(parser)))
                {
                    return Handle<Node>();
                }
                entries.PushBack(Pair<Handle<Node> >(key, value));
                if (parser->ReadToken(Token::COMMA))
                {
                    continue;
                }
                else if (parser->ReadToken(Token::RBRACE))
                {
                    break;
                }
                parser->SetErrorMessage("Unterminated map literal");

                return Handle<Node>();
            }
        }

        return new MapNode(entries);
    }

    static Handle<TypeHint> parse_typehint(const Handle<Parser>& parser)
    {
        Handle<Node> node = parse_postfix(parser);
        Handle<TypeHint> hint;

        if (!node)
        {
            return Handle<TypeHint>();
        }
        hint = TypeHint::FromExpression(node);
        if (parser->ReadToken(Token::CONDITIONAL))
        {
            hint = hint->MakeNullable();
        }
        if (parser->ReadToken(Token::BIT_AND))
        {
            Handle<TypeHint> other = parse_typehint(parser);

            if (!other)
            {
                return Handle<TypeHint>();
            }
            hint = hint->MakeAnd(other);
        }
        else if (parser->ReadToken(Token::BIT_OR))
        {
            Handle<TypeHint> other = parse_typehint(parser);

            if (!other)
            {
                return Handle<TypeHint>();
            }
            hint = hint->MakeOr(other);
        }

        return hint;
    }

    static bool parse_parameters(const Handle<Parser>& parser, Vector<Handle<Parameter> >& parameters)
    {
        if (!expect_token(parser, Token::LPAREN))
        {
            return false;
        }
        else if (parser->ReadToken(Token::RPAREN))
        {
            return true;
        }
        for (;;)
        {
            const bool rest = parser->ReadToken(Token::DOT_DOT_DOT);
            const Parser::TokenDescriptor token = parser->ReadToken();
            String name;
            Handle<TypeHint> type;
            Handle<Node> default_value;

            if (token.kind != Token::IDENTIFIER)
            {
                parser->SetErrorMessage(String("Unexpected ")
                                        + Token::What(token.kind)
                                        + "; Missing identifier");

                return false;
            }
            name = token.text;
            if (parser->ReadToken(Token::COLON))
            {
                if (!(type = parse_typehint(parser)))
                {
                    return false;
                }
            }
            if (parser->ReadToken(Token::ASSIGN))
            {
                if (!(default_value = parse_expr(parser)))
                {
                    return false;
                }
            }
            parameters.PushBack(new Parameter(name, type, default_value, rest));
            if (!rest && parser->ReadToken(Token::COMMA))
            {
                continue;
            }
            else if (parser->ReadToken(Token::RPAREN))
            {
                return true;
            }
            parser->SetErrorMessage("Unterminated parameter list");

            return false;
        }
    }

    static Handle<Node> parse_function(const Handle<Parser>& parser)
    {
        Vector<Handle<Parameter> > parameters;
        Vector<Handle<Node> > nodes;

        if (parser->PeekToken(Token::LPAREN) && !parse_parameters(parser, parameters))
        {
            return Handle<Node>();
        }
        if (parser->ReadToken(Token::ARROW))
        {
            Handle<Node> node = parse_expr(parser);

            if (!node)
            {
                return Handle<Node>();
            }
            nodes.PushBack(new ReturnNode(node));
        } else {
            if (!expect_token(parser, Token::COLON))
            {
                return Handle<Node>();
            }
            if (parser->ReadToken(Token::CLOSE_TAG))
            {
                for (;;)
                {
                    bool should_continue = false;

                    if (!parse_text_block(parser, nodes, should_continue))
                    {
                        return Handle<Node>();
                    }
                    else if (should_continue)
                    {
                        if (parser->PeekToken(Token::KW_END) || parser->PeekToken(Token::KW_ELSE))
                        {
                            break;
                        }
                        else if (!parse_script_block(parser, nodes, should_continue))
                        {
                            return Handle<Node>();
                        }
                        else if (!should_continue)
                        {
                            break;
                        }
                    } else {
                        break;
                    }
                }
            } else {
                while (!parser->PeekToken(Token::KW_END))
                {
                    Handle<Node> statement = parse_stmt(parser);

                    if (!statement)
                    {
                        return Handle<Node>();
                    }
                    nodes.PushBack(statement);
                }
            }
            if (!expect_token(parser, Token::KW_END) || !expect_token(parser, Token::KW_FUNCTION))
            {
                return Handle<Node>();
            }
        }

        return new FunctionNode(parameters, nodes);
    }

    static Handle<Node> parse_primary(const Handle<Parser>& parser)
    {
        Parser::TokenDescriptor token = parser->ReadToken();
        Handle<Node> node;

        switch (token.kind)
        {
            case Token::ERROR:
                break;

            case Token::END_OF_INPUT:
                parser->SetErrorMessage("Unexpected end of input; Missing expression");
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
                    parser->SetErrorMessage("Integer overflow");

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
                    parser->SetErrorMessage("Float overflow");

                    return Handle<Node>();
                }
                node = new ValueNode(Value::NewFloat(value));
                break;
            }

            case Token::LPAREN:
                if (!(node = parse_expr(parser)) || !expect_token(parser, Token::RPAREN))
                {
                    return Handle<Node>();
                }
                break;

            case Token::LBRACK:
                node = parse_list(parser);
                break;

            case Token::LBRACE:
                node = parse_map(parser);
                break;

            case Token::IDENTIFIER:
                node = new IdentifierNode(token.text);
                break;

            case Token::KW_FUNCTION:
                node = parse_function(parser);
                break;
            
            default:
                parser->SetErrorMessage(String("Unexpected ")
                                        + Token::What(token.kind)
                                        + "; Missing expression");
                break;
        }

        return node;
    }

    static bool parse_args(const Handle<Parser>& parser, Vector<Handle<Node> >& args)
    {
        if (!expect_token(parser, Token::LPAREN))
        {
            return false;
        }
        else if (parser->ReadToken(Token::RPAREN))
        {
            return true;
        }
        for (;;)
        {
            Handle<Node> node = parse_expr(parser);

            if (!node)
            {
                return false;
            }
            args.PushBack(node);
            if (parser->ReadToken(Token::COMMA))
            {
                continue;
            }
            else if (parser->ReadToken(Token::RPAREN))
            {
                return true;
            }
            parser->SetErrorMessage("Unterminated argument list");

            return false;
        }
    }

    static Handle<Node> parse_selection(const Handle<Parser>& parser, const Handle<Node>& node, bool safe)
    {
        const Parser::TokenDescriptor token = parser->ReadToken();

        if (token.kind != Token::IDENTIFIER)
        {
            StringBuilder sb;

            sb << "Unexpected " << Token::What(token.kind) << "; Missing identifier";
            parser->SetErrorMessage(sb.ToString());

            return Handle<Node>();
        }
        else if (parser->PeekToken(Token::LPAREN))
        {
            Vector<Handle<Node> > args;

            if (parse_args(parser, args))
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
    static Handle<Node> parse_postfix(const Handle<Parser>& parser)
    {
        Handle<Node> node = parse_primary(parser);

        if (!node)
        {
            return Handle<Node>();
        }
        for (;;)
        {
            const Parser::TokenDescriptor& token = parser->PeekToken();

            if (token.kind == Token::LPAREN)
            {
                Vector<Handle<Node> > args;

                if (!parse_args(parser, args))
                {
                    return Handle<Node>();
                }
                node = new CallNode(node, "__call__", args);
            }
            else if (token.kind == Token::LBRACK)
            {
                Handle<Node> index;

                parser->SkipToken();
                if (!(index = parse_expr(parser)) || !expect_token(parser, Token::RBRACK))
                {
                    return Handle<Node>();
                }
                node = new SubscriptNode(node, index);
            }
            else if (token.kind == Token::DOT || token.kind == Token::DOT_CONDITIONAL)
            {
                const bool safe = token.kind == Token::DOT_CONDITIONAL;

                parser->SkipToken();
                if (!(node = parse_selection(parser, node, safe)))
                {
                    return Handle<Node>();
                }
            }
            else if (token.kind == Token::INCREMENT || token.kind == Token::DECREMENT)
            {
                const PostfixNode::Kind kind = token.kind == Token::INCREMENT ?
                    PostfixNode::INCREMENT : PostfixNode::DECREMENT;

                parser->SkipToken();
                if (!node->IsVariable())
                {
                    parser->SetErrorMessage("Node is not assignable");

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
    static Handle<Node> parse_unary(const Handle<Parser>& parser)
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
                if (!(receiver = parse_unary(parser)))
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
                if (!(condition = parse_unary(parser)))
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
                if (!(node = parse_unary(parser)))
                {
                    return Handle<Node>();
                }
                else if (!node->IsVariable())
                {
                    parser->SetErrorMessage("Node is not assignable");

                    return Handle<Node>();
                }
                node = new PrefixNode(node, kind);
                break;
            }

            default:
                node = parse_postfix(parser);
        }

        return node;
    }

    /**
     * unary-expression
     * multiplicative-expression "*" unary-expression
     * multiplicative-expression "/" unary-expression
     * multiplicative-expression "%" unary-expression
     */
    static Handle<Node> parse_multiplicative(const Handle<Parser>& parser)
    {
        Handle<Node> node = parse_unary(parser);

        if (!node)
        {
            return Handle<Node>();
        }
        for (;;)
        {
            const Parser::TokenDescriptor& token = parser->PeekToken();

            if (token.kind == Token::MUL || token.kind == Token::DIV || token.kind == Token::MOD)
            {
                const Token::Kind kind = token.kind;
                Handle<Node> operand;

                parser->SkipToken();
                if (!(operand = parse_unary(parser)))
                {
                    return Handle<Node>();
                }
                node = new CallNode(
                    node,
                    kind == Token::MUL ? "__mul__" :
                    kind == Token::DIV ? "__div__" : "__mod__",
                    Vector<Handle<Node> >(1, operand)
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
    static Handle<Node> parse_additive(const Handle<Parser>& parser)
    {
        Handle<Node> node = parse_multiplicative(parser);

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
                if (!(operand = parse_multiplicative(parser)))
                {
                    return Handle<Node>();
                }
                node = new CallNode(
                    node,
                    kind == Token::ADD ? "__add__" : "__sub__",
                    Vector<Handle<Node> >(1, operand)
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
    static Handle<Node> parse_shift(const Handle<Parser>& parser)
    {
        Handle<Node> node = parse_additive(parser);

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
                if (!(operand = parse_additive(parser)))
                {
                    return Handle<Node>();
                }
                node = new CallNode(
                    node,
                    kind == Token::LSH ? "__lsh__" : "__rsh__",
                    Vector<Handle<Node> >(1, operand)
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
    static Handle<Node> parse_bit_and(const Handle<Parser>& parser)
    {
        Handle<Node> node = parse_shift(parser);

        if (!node)
        {
            return Handle<Node>();
        }
        while (parser->ReadToken(Token::BIT_AND))
        {
            Handle<Node> operand = parse_shift(parser);

            if (!operand)
            {
                return Handle<Node>();
            }
            node = new CallNode(
                node,
                "__and__",
                Vector<Handle<Node> >(1, operand)
            );
        }

        return node;
    }

    /**
     * bit-and-expression
     * bit-xor-expression "^" bit-and-expression
     */
    static Handle<Node> parse_bit_xor(const Handle<Parser>& parser)
    {
        Handle<Node> node = parse_bit_and(parser);

        if (!node)
        {
            return Handle<Node>();
        }
        while (parser->ReadToken(Token::BIT_XOR))
        {
            Handle<Node> operand = parse_bit_and(parser);

            if (!operand)
            {
                return Handle<Node>();
            }
            node = new CallNode(
                node,
                "__xor__",
                Vector<Handle<Node> >(1, operand)
            );
        }

        return node;
    }

    /**
     * bit-xor-expression
     * bit-or-expression "|" bit-xor-expression
     */
    static Handle<Node> parse_bit_or(const Handle<Parser>& parser)
    {
        Handle<Node> node = parse_bit_xor(parser);

        if (!node)
        {
            return Handle<Node>();
        }
        while (parser->ReadToken(Token::BIT_OR))
        {
            Handle<Node> operand = parse_bit_xor(parser);

            if (!operand)
            {
                return Handle<Node>();
            }
            node = new CallNode(
                node,
                "__or__",
                Vector<Handle<Node> >(1, operand)
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
    static Handle<Node> parse_relational(const Handle<Parser>& parser)
    {
        Handle<Node> node = parse_bit_or(parser);

        if (!node)
        {
            return Handle<Node>();
        }
        for (;;)
        {
            const Parser::TokenDescriptor& token = parser->PeekToken();

            switch (token.kind)
            {
                case Token::LT:
                case Token::GT:
                case Token::LTE:
                case Token::GTE:
                {
                    const Token::Kind kind = token.kind;
                    Handle<Node> operand;

                    parser->SkipToken();
                    if (!(operand = parse_bit_or(parser)))
                    {
                        return Handle<Node>();
                    }
                    node = new CallNode(
                        node,
                        kind == Token::LT  ? "__lt__"  :
                        kind == Token::GT  ? "__gt__"  :
                        kind == Token::LTE ? "__lte__" :
                                             "__gte__",
                        Vector<Handle<Node> >(1, operand)
                    );
                    break;
                }

                default:
                    return node;
            }
        }
    }

    /**
     * relational-expression
     * equality-expression "==" relational-expression
     * equality-expression "!=" relational-expression
     * equality-expression "=~" relational-expression
     * equality-expression "!~" relational-expression
     * equality-expression "<=>" relational-expression
     */
    static Handle<Node> parse_equality(const Handle<Parser>& parser)
    {
        Handle<Node> node = parse_relational(parser);

        if (!node)
        {
            return Handle<Node>();
        }
        for (;;)
        {
            const Parser::TokenDescriptor& token = parser->PeekToken();

            switch (token.kind)
            {
                case Token::EQ:
                case Token::MATCH:
                {
                    const Token::Kind kind = token.kind;
                    Handle<Node> operand;

                    parser->SkipToken();
                    if (!(operand = parse_relational(parser)))
                    {
                        return Handle<Node>();
                    }
                    node = new CallNode(
                        node,
                        kind == Token::EQ ? "__eq__" : "__match__",
                        Vector<Handle<Node> >(1, operand)
                    );
                    break;
                }

                case Token::NE:
                case Token::NO_MATCH:
                {
                    const Token::Kind kind = token.kind;
                    Handle<Node> operand;

                    parser->SkipToken();
                    if (!(operand = parse_relational(parser)))
                    {
                        return Handle<Node>();
                    }
                    node = new NotNode(
                        new CallNode(
                            node,
                            kind == Token::NE ? "__eq__" : "__match__",
                            Vector<Handle<Node> >(1, operand)
                        )
                    );
                    break;
                }

                default:
                    return node;
            }
        }
    }

    /**
     * equality-expression
     * logical-and-expression "&&" equality-expression
     */
    static Handle<Node> parse_logical_and(const Handle<Parser>& parser)
    {
        Handle<Node> node = parse_equality(parser);

        if (!node)
        {
            return Handle<Node>();
        }
        while (parser->ReadToken(Token::AND))
        {
            Handle<Node> operand = parse_equality(parser);

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
    static Handle<Node> parse_logical_or(const Handle<Parser>& parser)
    {
        Handle<Node> node = parse_logical_and(parser);

        if (!node)
        {
            return Handle<Node>();
        }
        while (parser->ReadToken(Token::OR))
        {
            Handle<Node> operand = parse_logical_and(parser);

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
    static Handle<Node> parse_range(const Handle<Parser>& parser)
    {
        Handle<Node> node = parse_logical_or(parser);

        if (!node)
        {
            return Handle<Node>();
        }
        if (parser->PeekToken(Token::DOT_DOT) || parser->PeekToken(Token::DOT_DOT_DOT))
        {
            const bool exclusive = parser->ReadToken().kind == Token::DOT_DOT_DOT;
            Handle<Node> operand = parse_logical_or(parser);

            if (operand)
            {
                return new RangeNode(node, operand, exclusive);
            } else {
                return Handle<Node>();
            }
        }

        return node;
    }

    /**
     * range-expression
     * range-expression "?" expression ":" expression
     */
    static Handle<Node> parse_ternary(const Handle<Parser>& parser)
    {
        Handle<Node> node = parse_range(parser);

        if (!node)
        {
            return Handle<Node>();
        }
        else if (parser->ReadToken(Token::CONDITIONAL))
        {
            Handle<Node> then_node;
            Handle<Node> else_node;

            if (!(then_node = parse_expr(parser))
                || !expect_token(parser, Token::COLON)
                || !(else_node = parse_expr(parser)))
            {
                return Handle<Node>();
            }
            node = new IfNode(node, then_node, else_node);
        }

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
    static Handle<Node> parse_expr(const Handle<Parser>& parser)
    {
        Handle<Node> node = parse_ternary(parser);

        if (!node)
        {
            return Handle<Node>();
        }
        switch (parser->PeekToken().kind)
        {
            case Token::ERROR:
                return Handle<Node>();

            case Token::ASSIGN:
            {
                Handle<Node> operand;

                parser->SkipToken();
                if (!(operand = parse_expr(parser)))
                {
                    return Handle<Node>();
                }
                else if (!node->IsVariable())
                {
                    parser->SetErrorMessage("Missing variable expression before '='");

                    return Handle<Node>();
                }

                return new AssignNode(node, operand);
            }

            case Token::ASSIGN_AND:
            case Token::ASSIGN_OR:
            {
                const Token::Kind kind = parser->ReadToken().kind;
                Handle<Node> operand = parse_expr(parser);

                if (!operand)
                {
                    return Handle<Node>();
                }
                else if (!node->IsVariable())
                {
                    parser->SetErrorMessage(String("Missing variable expression before ") + Token::What(kind));

                    return Handle<Node>();
                }
                else if (kind == Token::ASSIGN_AND)
                {
                    return new AssignNode(node, new AndNode(node, operand));
                } else {
                    return new AssignNode(node, new OrNode(node, operand));
                }
            }

            case Token::ASSIGN_BIT_AND:
            case Token::ASSIGN_BIT_OR:
            case Token::ASSIGN_BIT_XOR:
            case Token::ASSIGN_LSH:
            case Token::ASSIGN_RSH:
            case Token::ASSIGN_ADD:
            case Token::ASSIGN_SUB:
            case Token::ASSIGN_MUL:
            case Token::ASSIGN_DIV:
            case Token::ASSIGN_MOD:
            {
                const Token::Kind kind = parser->ReadToken().kind;
                Handle<Node> operand = parse_expr(parser);

                if (!operand)
                {
                    return Handle<Node>();
                }
                else if (!node->IsVariable())
                {
                    parser->SetErrorMessage(String("Missing variable expression before ") + Token::What(kind));

                    return Handle<Node>();
                }

                return new AssignNode(
                    node,
                    new CallNode(
                        node,
                        kind == Token::ASSIGN_BIT_AND ? "__and__" :
                        kind == Token::ASSIGN_BIT_OR  ? "__or__"  :
                        kind == Token::ASSIGN_BIT_XOR ? "__xor__" :
                        kind == Token::ASSIGN_LSH     ? "__lsh__" :
                        kind == Token::ASSIGN_RSH     ? "__rsh__" :
                        kind == Token::ASSIGN_ADD     ? "__add__" :
                        kind == Token::ASSIGN_SUB     ? "__sub__" :
                        kind == Token::ASSIGN_MUL     ? "__mul__" :
                        kind == Token::ASSIGN_DIV     ? "__div__" :
                                                        "__mod__"
                    )
                );
            }

            default:
                return node;
        }
    }
}
