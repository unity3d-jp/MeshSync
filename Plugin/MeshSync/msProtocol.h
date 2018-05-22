#pragma once

#include <atomic>
#include "msSceneGraph.h"

namespace ms {

class Message
{
public:
    enum class Type
    {
        Unknown,
        Get,
        Set,
        Delete,
        Fence,
        Text,
        Screenshot,
    };

    virtual ~Message();
    virtual uint32_t getSerializeSize() const;
    virtual void serialize(std::ostream& os) const;
    virtual bool deserialize(std::istream& is);
};
HasSerializer(Message);
using MessagePtr = std::shared_ptr<Message>;


struct GetFlags
{
    uint32_t get_transform : 1;
    uint32_t get_points : 1;
    uint32_t get_normals : 1;
    uint32_t get_tangents : 1;
    uint32_t get_uv0 : 1;
    uint32_t get_uv1 : 1;
    uint32_t get_colors : 1;
    uint32_t get_indices : 1;
    uint32_t get_material_ids : 1;
    uint32_t get_bones : 1;
    uint32_t get_blendshapes : 1;
    uint32_t apply_culling : 1;
};


class GetMessage : public Message
{
using super = Message;
public:
    GetFlags flags = {0};
    SceneSettings scene_settings;
    MeshRefineSettings refine_settings;

    // non-serializable
    std::shared_ptr<std::atomic_int> wait_flag;

public:
    GetMessage();
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    bool deserialize(std::istream& is) override;
};
HasSerializer(GetMessage);


class SetMessage : public Message
{
using super = Message;
public:
    Scene scene;

public:
    SetMessage();
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    bool deserialize(std::istream& is) override;
};
HasSerializer(SetMessage);


class DeleteMessage : public Message
{
using super = Message;
public:
    struct Identifier
    {
        std::string path;
        int id;

        uint32_t getSerializeSize() const;
        void serialize(std::ostream& os) const;
        void deserialize(std::istream& is);
    };
    std::vector<Identifier> targets;

    DeleteMessage();
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    bool deserialize(std::istream& is) override;
};
HasSerializer(DeleteMessage::Identifier);
HasSerializer(DeleteMessage);


class FenceMessage : public Message
{
using super = Message;
public:
    enum class FenceType
    {
        Unknown,
        SceneBegin,
        SceneEnd,
    };

    FenceType type = FenceType::Unknown;

    ~FenceMessage() override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    bool deserialize(std::istream& is) override;
};
HasSerializer(FenceMessage);


class TextMessage : public Message
{
using super = Message;
public:
    enum class Type
    {
        Normal,
        Warning,
        Error,
    };

    ~TextMessage() override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    bool deserialize(std::istream& is) override;

public:
    std::string text;
    Type type = Type::Normal;
};
HasSerializer(TextMessage);


class ScreenshotMessage : public Message
{
using super = Message;
public:

    // non-serializable
    std::shared_ptr<std::atomic_int> wait_flag;

public:
    ScreenshotMessage();
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    bool deserialize(std::istream& is) override;
};
HasSerializer(ScreenshotMessage);

} // namespace ms
