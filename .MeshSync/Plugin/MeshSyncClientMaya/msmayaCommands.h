#pragma once

#if MAYA_LT
    #define LT_PAD64 char pad[64] = {0}
#else
    #define LT_PAD64
#endif


class CmdSettings : public MPxCommand
{
public:
    static void* create();
    static const char* name();
    static MSyntax createSyntax();

    MStatus doIt(const MArgList& args) override;
    LT_PAD64;
};

class CmdExport : public MPxCommand
{
public:
    static void* create();
    static const char* name();
    static MSyntax createSyntax();

    MStatus doIt(const MArgList&) override;
    LT_PAD64;
};

class CmdImport : public MPxCommand
{
public:
    static void* create();
    static const char* name();
    static MSyntax createSyntax();

    MStatus doIt(const MArgList&) override;
    LT_PAD64;
};

#define EachCommand(Body)\
    Body(CmdSettings)\
    Body(CmdExport)\
    Body(CmdImport)
