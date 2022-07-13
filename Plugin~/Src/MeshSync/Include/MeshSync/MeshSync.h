#pragma once

#include "msConfig.h"

//Forward declarations
#define msDeclClassPtr(T) \
namespace ms { \
    class T; \
    typedef std::shared_ptr<T> T##Ptr; \
} 

#define msDeclStructPtr(T) \
namespace ms { \
    struct T; \
    typedef std::shared_ptr<T> T##Ptr; \
} 

//Entity
msDeclClassPtr(Camera)
msDeclClassPtr(Light)
msDeclClassPtr(Mesh)
msDeclClassPtr(Curve)
msDeclClassPtr(Transform)

//Assets
msDeclClassPtr(AnimationClip)
msDeclClassPtr(Audio)
msDeclClassPtr(Material)
msDeclClassPtr(Texture)

//Protocol/Messages
msDeclClassPtr(TextMessage)

//Network
msDeclClassPtr(Server)

//Others
msDeclClassPtr(TransformAnimation)

//Structs
msDeclStructPtr(BlendShapeData)

