#pragma once


class CmdSettings : public MPxCommand
{
public:
    static void* create();
    static const char* name();
    static MSyntax createSyntax();

    virtual MStatus doIt(const MArgList& args);
};

class CmdExport : public MPxCommand
{
public:
    static void* create();
    static const char* name();
    static MSyntax createSyntax();

    virtual MStatus doIt(const MArgList&);
};

class CmdImport : public MPxCommand
{
public:
    static void* create();
    static const char* name();
    static MSyntax createSyntax();

    virtual MStatus doIt(const MArgList&);
};

#define EachCommand(Body)\
    Body(CmdSettings)\
    Body(CmdExport)\
    Body(CmdImport)
