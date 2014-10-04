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
        explicit Script(const std::vector<Handle<Node> >& nodes);

        bool Execute(const Handle<Interpreter>& interpreter) const;

        void Mark();

    private:
        const std::vector<Node*> m_nodes;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Script);
    };
}

#endif /* !TEMPEARLY_SCRIPT_H_GUARD */
