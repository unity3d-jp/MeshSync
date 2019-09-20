#pragma once

class mshouSOPSend : public SOP_Node
{
using super = SOP_Node;
public:
    static PRM_Template myTemplateList[];
    static OP_Node* myConstructor(OP_Network*, const char*, OP_Operator*);

    mshouSOPSend(OP_Network *net, const char *name, OP_Operator *op);
    ~mshouSOPSend() override;

    OP_ERROR cookMySop(OP_Context &context) override;
};
