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
msDeclClassPtr(Transform)

//Assets
msDeclClassPtr(Audio)
msDeclClassPtr(Texture)

//Protocol/Messages
msDeclClassPtr(TextMessage)

//Network
msDeclClassPtr(Server)


//
//class Entity;
//msDeclPtr(Entity);
//
//class Mesh;
//msDeclPtr(Mesh);
//
//class Points;
//msDeclPtr(Points);
//
//struct BlendShapeFrameData;
//msDeclPtr(BlendShapeFrameData);
//
//struct BlendShapeData;
//msDeclPtr(BlendShapeData);
//
//struct BoneData;
//msDeclPtr(BoneData);
//
//class Points;
//msDeclPtr(Points);
//
//class Mesh;
//msDeclPtr(Mesh);
//
//class Constraint;
//msDeclPtr(Constraint);
//
//class AnimationCurve;
//msDeclPtr(AnimationCurve);
//
//class Scene;
//msDeclPtr(Scene);
//
//class Animation;
//msDeclPtr(Animation);
//
//class TransformAnimation;
//msDeclPtr(TransformAnimation);
//
////Protocol/Messages
//class ResponseMessage;
//msDeclPtr(ResponseMessage);
//
//class QueryMessage;
//msDeclPtr(QueryMessage);
//
//class PollMessage;
//msDeclPtr(PollMessage);
//
//class SetMessage;
//msDeclPtr(SetMessage);
//
//class DeleteMessage;
//msDeclPtr(DeleteMessage);
//
//class FenceMessage;
//msDeclPtr(FenceMessage);
//
//

//
//class Message;
//msDeclPtr(Message);
//
////[Note-sin: 2020-9-4] Might be defined in Windows by Winsock
////#ifdef GetMessage 
////#undef GetMessage
////#endif
//
//class GetMessage;
//msDeclPtr(GetMessage);
//
//class ScreenshotMessage;
//msDeclPtr(ScreenshotMessage);
//
//


//
//class AnimationClip;
//msDeclPtr(AnimationClip);
//
//class FileAsset;
//msDeclPtr(FileAsset);
//
//class Material;
//msDeclPtr(Material);
//
//
////BlendShape
//struct BlendShapeFrameData;
//msDeclPtr(BlendShapeFrameData);
//
//
////Converter / Correctors
//class EntityConverter;
//msDeclPtr(EntityConverter);
//
//class ScaleConverter;
//msDeclPtr(ScaleConverter);
//
//class FlipX_HandednessCorrector;
//msDeclPtr(FlipX_HandednessCorrector);
//
//class FlipYZ_ZUpCorrector;
//msDeclPtr(FlipYZ_ZUpCorrector);
//
//class RotateX_ZUpCorrector;
//msDeclPtr(RotateX_ZUpCorrector);
