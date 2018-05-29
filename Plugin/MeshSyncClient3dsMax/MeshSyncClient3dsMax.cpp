#include "pch.h"
#include "MeshSyncClient3dsMax.h"

#ifdef _WIN32
#endif

static std::unique_ptr<MeshSyncClient3dsMax> g_plugin;

MeshSyncClient3dsMax & MeshSyncClient3dsMax::getInstance()
{
    return *g_plugin;
}

MeshSyncClient3dsMax::MeshSyncClient3dsMax()
{
}

MeshSyncClient3dsMax::~MeshSyncClient3dsMax()
{
}

void MeshSyncClient3dsMax::update()
{
}

bool MeshSyncClient3dsMax::sendScene(SendScope scope)
{
    return false;
}

bool MeshSyncClient3dsMax::sendAnimations(SendScope scope)
{
    return false;
}

bool MeshSyncClient3dsMax::recvScene()
{
    return false;
}
