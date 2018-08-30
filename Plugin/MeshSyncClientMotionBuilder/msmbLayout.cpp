#include "pch.h"
#include "msmbDevice.h"
#include "msmbLayout.h"
#include "msmbUtils.h"


FBDeviceLayoutImplementation(msmbLayout);
FBRegisterDeviceLayout(msmbLayout, "UnityMeshSync", FB_DEFAULT_SDK_ICON);

bool msmbLayout::FBCreate()
{
    m_device = (msmbDevice*)(FBDevice*)Device;
    return true;
}

void msmbLayout::FBDestroy()
{
}
