#ifndef TEMPEARLY_SCRIPT_H_GUARD
#define TEMPEARLY_SCRIPT_H_GUARD

#include "node.h"

namespace tempearly
{
    /**
     * Script encapsulates multiple nodes into single executable object.
     */
    class Script : public CountedObject
    {
    public:
        explicit Script(const Vector<Handle<Node> >& nodes);

        bool Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        const Vector<Node*> m_nodes;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Script);
    };
}

#endif /* !TEMPEARLY_SCRIPT_H_GUARD */
