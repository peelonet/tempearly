#include "interpreter.h"
#include "script.h"

namespace tempearly
{
    Script::Script(const Vector<Handle<Node> >& nodes)
        : m_nodes(nodes) {}

    bool Script::Execute(const Handle<Interpreter>& interpreter) const
    {
        for (std::size_t i = 0; i < m_nodes.GetSize(); ++i)
        {
            Result result = m_nodes[i]->Execute(interpreter);

            if (result.Is(Result::KIND_SUCCESS))
            {
                continue;
            }
            switch (result.GetKind())
            {
                case Result::KIND_BREAK:
                    interpreter->Throw(interpreter->eSyntaxError,
                                       "Unexpected `break'");
                    break;

                case Result::KIND_CONTINUE:
                    interpreter->Throw(interpreter->eSyntaxError,
                                       "Unexpected `continue'");
                    break;

                case Result::KIND_RETURN:
                    return true;

                default:
                    break;
            }

            return false;
        }

        return true;
    }

    void Script::Mark()
    {
        CountedObject::Mark();
        for (std::size_t i = 0; i < m_nodes.GetSize(); ++i)
        {
            Node* node = m_nodes[i];

            if (!node->IsMarked())
            {
                node->Mark();
            }
        }
    }
}
