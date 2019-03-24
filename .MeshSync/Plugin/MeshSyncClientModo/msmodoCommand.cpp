#include "pch.h"
#include "MeshSyncClientModo.h"
#include "msmodoUtils.h"
#include "msmodoCommand.h"

#define CommandName "unity.command.meshsync"


void msmodoCommand::execute()
{
    CLxUser_LogService lS;
    lS.DebugOut(LXi_DBLOG_NORMAL, CommandName " executed%f\n");
}


static CLxMeta_Command<msmodoCommand> cmd_meta(CommandName);

static class CRoot : public CLxMetaRoot
{
    bool pre_init() override
    {
        cmd_meta.set_type_UI();
        cmd_meta.add_notifier(LXsNOTIFIER_SELECT, "item +d");

        add(&cmd_meta);
        return false;
    }
} root_meta;
