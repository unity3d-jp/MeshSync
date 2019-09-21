#include "pch.h"
#include "mshouROPSend.h"

void newDriverOperator(OP_OperatorTable *table)
{
    {
        auto op = new OP_Operator(
            "meshsync_send",
            "MeshSync Send",
            mshouROPSend::myConstructor,
            mshouROPSend::getTemplatePair(),
            0,
            0,
            mshouROPSend::getVariablePair(),
            OP_FLAG_GENERATOR);
        op->setOpTabSubMenuPath("Scene");
        table->addOperator(op);
    }

    {
        auto sop_table = OP_Network::getOperatorTable(SOP_TABLE_NAME, SOP_SCRIPT_NAME);
        auto op = new OP_Operator(
            "rop_meshsync_send",
            "ROP MeshSync Send",
            mshouROPSend::myConstructor,
            mshouROPSend::getTemplatePair(),
            0,
            0,
            mshouROPSend::getVariablePair(),
            OP_FLAG_GENERATOR | OP_FLAG_MANAGER);
        op->setOpTabSubMenuPath("Export");
        sop_table->addOperator(op);
    }
}

static PRM_Name sopPathName("soppath", "SOP Path");

static PRM_Template myTemplateList[] = {
    PRM_Template(
        PRM_STRING_OPREF,
        PRM_TYPE_DYNAMIC_PATH,
        1,
        &sopPathName,
        0, // default
        0, // choice
        0, // range
        0, // callback
        &PRM_SpareData::sopPath,
        0, // paramgroup (leave default)
        "SOP to export", // help string
        0), // disable rules

    theRopTemplates[ROP_TPRERENDER_TPLATE],
    theRopTemplates[ROP_PRERENDER_TPLATE],
    theRopTemplates[ROP_LPRERENDER_TPLATE],
    theRopTemplates[ROP_TPREFRAME_TPLATE],
    theRopTemplates[ROP_PREFRAME_TPLATE],
    theRopTemplates[ROP_LPREFRAME_TPLATE],
    theRopTemplates[ROP_TPOSTFRAME_TPLATE],
    theRopTemplates[ROP_POSTFRAME_TPLATE],
    theRopTemplates[ROP_LPOSTFRAME_TPLATE],
    theRopTemplates[ROP_TPOSTRENDER_TPLATE],
    theRopTemplates[ROP_POSTRENDER_TPLATE],
    theRopTemplates[ROP_LPOSTRENDER_TPLATE],

    PRM_Template(),
};

OP_TemplatePair* mshouROPSend::getTemplatePair()
{
    static OP_TemplatePair *pair = nullptr;
    if (!pair)
    {
        auto base = new OP_TemplatePair(myTemplateList);
        pair = new OP_TemplatePair(ROP_Node::getROPbaseTemplate(), base);
    }
    return pair;
}

OP_VariablePair* mshouROPSend::getVariablePair()
{
    static OP_VariablePair *pair = nullptr;
    if (!pair) {
        pair = new OP_VariablePair(ROP_Node::myVariableList);
    }
    return pair;
}

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
    //OBJ_Node::getDisplaySopPtr() and OBJ_Node::getRenderSopPtr().

    int num_children = n->getNchildren();
    for (int i = 0; i < num_children; ++i)
        exportNode(n->getChild(i));
    return false;
}
