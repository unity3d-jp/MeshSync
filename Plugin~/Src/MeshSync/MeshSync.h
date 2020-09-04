#pragma once

namespace ms {

#define msDeclPtr(T) using T##Ptr = std::shared_ptr<T>

class Transform;
msDeclPtr(Transform);

class Entity;
msDeclPtr(Entity);

class Mesh;
msDeclPtr(Mesh);

class Points;
msDeclPtr(Points);

struct BlendShapeFrameData;
msDeclPtr(BlendShapeFrameData);

struct BlendShapeData;
msDeclPtr(BlendShapeData);

struct BoneData;
msDeclPtr(BoneData);

class Points;
msDeclPtr(Points);

class Mesh;
msDeclPtr(Mesh);

class Constraint;
msDeclPtr(Constraint);

class AnimationCurve;
msDeclPtr(AnimationCurve);

class Scene;
msDeclPtr(Scene);

class Animation;
msDeclPtr(Animation);

class TransformAnimation;
msDeclPtr(TransformAnimation);

//Protocol/Messages
class ResponseMessage;
msDeclPtr(ResponseMessage);

class QueryMessage;
msDeclPtr(QueryMessage);

class PollMessage;
msDeclPtr(PollMessage);

class SetMessage;
msDeclPtr(SetMessage);

class DeleteMessage;
msDeclPtr(DeleteMessage);

class FenceMessage;
msDeclPtr(FenceMessage);

class TextMessage;
msDeclPtr(TextMessage);

class Message;
msDeclPtr(Message);

//[Note-sin: 2020-9-4] Might be defined in Windows by Winsock
//#ifdef GetMessage 
//#undef GetMessage
//#endif

class GetMessage;
msDeclPtr(GetMessage);

class ScreenshotMessage;
msDeclPtr(ScreenshotMessage);


//Entity
class Camera;
msDeclPtr(Camera);

class Light;
msDeclPtr(Light);

//Assets
class Asset;
msDeclPtr(Asset);

class Audio;
msDeclPtr(Audio);

class AnimationClip;
msDeclPtr(AnimationClip);

class FileAsset;
msDeclPtr(FileAsset);

class Material;
msDeclPtr(Material);

class Texture;
msDeclPtr(Texture);

//BlendShape
struct BlendShapeFrameData;
msDeclPtr(BlendShapeFrameData);


//Converter / Correctors
class EntityConverter;
msDeclPtr(EntityConverter);

class ScaleConverter;
msDeclPtr(ScaleConverter);

class FlipX_HandednessCorrector;
msDeclPtr(FlipX_HandednessCorrector);

class FlipYZ_ZUpCorrector;
msDeclPtr(FlipYZ_ZUpCorrector);

class RotateX_ZUpCorrector;
msDeclPtr(RotateX_ZUpCorrector);

#undef msDeclPtr

} // end namespace ms

//#ifdef mscDebug
//    #define mscTrace(...) ::mu::Print("MeshSync trace: " __VA_ARGS__)
//    #define mscTraceW(...) ::mu::Print(L"MeshSync trace: " __VA_ARGS__)
//#else
//    #define mscTrace(...)
//    #define mscTraceW(...)
//#endif
