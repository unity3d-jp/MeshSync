#pragma once

class mshouROPSend : public ROP_Node
{
using super = ROP_Node;
public:
    static OP_TemplatePair *getTemplatePair();
    static OP_VariablePair* getVariablePair();
    static OP_Node* myConstructor(OP_Network *net, const char *name, OP_Operator *op);

    mshouROPSend(OP_Network *net, const char *name, OP_Operator *op);
    ~mshouROPSend() override;

    int startRender(int nframes, fpreal s, fpreal e) override;
    ROP_RENDER_CODE renderFrame(fpreal time, UT_Interrupt *boss) override;
    ROP_RENDER_CODE endRender() override;

private:
    bool exportNode(OP_Node *node);

    fpreal m_time_start, m_time_end;
};
