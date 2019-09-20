#include "pch.h"
#include "mshouROPSend.h"

void newSopOperator(OP_OperatorTable *table)
{
    table->addOperator(new OP_Operator(
        "meshsync_send",
        "MeshSync Send",
        mshouROPSend::myConstructor,
        mshouROPSend::myTemplateList,
        1,
        1,
        nullptr));
}

static PRM_Name names[] = {
    PRM_Name("usedir",  "Use Direction Vector"),
    PRM_Name("dist",    "Distance"),
};

PRM_Template mshouROPSend::myTemplateList[] = {
    PRM_Template(PRM_STRING,    1, &PRMgroupName, 0, &SOP_Node::pointGroupMenu, 0, 0, SOP_Node::getGroupSelectButton(GA_GROUP_POINT)),
    PRM_Template(PRM_FLT_J,     1, &names[1], PRMzeroDefaults, 0, &PRMscaleRange),
    PRM_Template(PRM_TOGGLE,    1, &names[0]),
    PRM_Template(PRM_ORD,       1, &PRMorientName, 0, &PRMplaneMenu),
    PRM_Template(PRM_DIRECTION, 3, &PRMdirectionName, PRMzaxisDefaults),
    PRM_Template(),
};


OP_Node* mshouROPSend::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new mshouROPSend(net, name, op);
}

mshouROPSend::mshouROPSend(OP_Network *net, const char *name, OP_Operator *op)
    : super(net, name, op)
{
}

mshouROPSend::~mshouROPSend()
{
}

int mshouROPSend::startRender(int nframes, fpreal s, fpreal e)
{
    m_time_start = s;
    m_time_end = e;
    return 0;
}

ROP_RENDER_CODE mshouROPSend::renderFrame(fpreal time, UT_Interrupt *boss)
{
    executePreFrameScript(time);

    exportNode(OPgetDirector());

    if (error() < UT_ERROR_ABORT)
        executePostFrameScript(time);
    return ROP_CONTINUE_RENDER;
}

ROP_RENDER_CODE mshouROPSend::endRender()
{
    if (error() < UT_ERROR_ABORT)
        executePostRenderScript(m_time_end);
    return ROP_CONTINUE_RENDER;
}


bool mshouROPSend::exportNode(OP_Node *n)
{
    int num_children = n->getNchildren();
    for (int i = 0; i < num_children; ++i)
        exportNode(n->getChild(i));
    return false;
}
