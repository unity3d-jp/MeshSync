#include "pch.h"
#include "mshouSOPSend.h"

void newSopOperator(OP_OperatorTable *table)
{
    table->addOperator(new OP_Operator(
        "meshsync_send",
        "MeshSync_Send",
        mshouSOPSend::myConstructor,
        mshouSOPSend::myTemplateList,
        1,
        1,
        nullptr));
}

static PRM_Name names[] = {
    PRM_Name("usedir",  "Use Direction Vector"),
    PRM_Name("dist",    "Distance"),
};

PRM_Template mshouSOPSend::myTemplateList[] = {
    PRM_Template(PRM_STRING,    1, &PRMgroupName, 0, &SOP_Node::pointGroupMenu, 0, 0, SOP_Node::getGroupSelectButton(GA_GROUP_POINT)),
    PRM_Template(PRM_FLT_J,     1, &names[1], PRMzeroDefaults, 0, &PRMscaleRange),
    PRM_Template(PRM_TOGGLE,    1, &names[0]),
    PRM_Template(PRM_ORD,       1, &PRMorientName, 0, &PRMplaneMenu),
    PRM_Template(PRM_DIRECTION, 3, &PRMdirectionName, PRMzaxisDefaults),
    PRM_Template(),
};


OP_Node* mshouSOPSend::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new mshouSOPSend(net, name, op);
}

mshouSOPSend::mshouSOPSend(OP_Network *net, const char *name, OP_Operator *op)
    : super(net, name, op)
{
}

mshouSOPSend::~mshouSOPSend()
{
}

OP_ERROR mshouSOPSend::cookMySop(OP_Context& context)
{
    OP_AutoLockInputs inputs(this);
    if (inputs.lock(context) >= UT_ERROR_ABORT)
        return error();

    fpreal now = context.getTime();

    // todo

    return error();
}
