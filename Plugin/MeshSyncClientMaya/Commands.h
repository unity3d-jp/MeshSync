#pragma once


class CmdSettings : public MPxCommand
{
public:
    static void* create();
    static const char* name();
    static MSyntax syntax();
    virtual MStatus doIt(const MArgList& args);
};

class CmdSync : public MPxCommand
{
public:
    static void* create();
    static const char* name();
    static MSyntax syntax();
    virtual MStatus doIt(const MArgList&);
};

class CmdImport : public MPxCommand
{
public:
    static void* create();
    static const char* name();
    static MSyntax syntax();
    virtual MStatus doIt(const MArgList&);
};

#define EachCommand(Body)\
    Body(CmdSettings)\
    Body(CmdSync)\
    Body(CmdImport)
