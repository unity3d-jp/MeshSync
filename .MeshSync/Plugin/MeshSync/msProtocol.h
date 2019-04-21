#pragma once

#include <atomic>
#include "SceneGraph/msSceneGraph.h"

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
        Query,
        Response,
    };
    int protocol_version = msProtocolVersion;
    int session_id = InvalidID;
    int message_id = 0;
    nanosec timestamp_send = 0;

    // non-serializable fields
    nanosec timestamp_recv = 0;

    virtual ~Message();
    virtual void serialize(std::ostream& os) const;
    virtual void deserialize(std::istream& is); // throw
};
msSerializable(Message);
msDeclPtr(Message);


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
    uint32_t get_blendshapes : 1; // 10
    uint32_t apply_culling : 1;

    void setAllGetFlags();
};

class GetMessage : public Message
{
using super = Message;
public:
    GetFlags flags = {0};
    SceneSettings scene_settings;
    MeshRefineSettings refine_settings;

    // non-serializable fields
    std::atomic_bool ready{ false };

public:
    GetMessage();
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};
msSerializable(GetMessage);
msDeclPtr(GetMessage);


class SetMessage : public Message
{
using super = Message;
public:
    Scene scene;

public:
    SetMessage();
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};
msSerializable(SetMessage);
msDeclPtr(SetMessage);


class DeleteMessage : public Message
{
using super = Message;
public:
    std::vector<Identifier> entities;
    std::vector<Identifier> materials;

    DeleteMessage();
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};
msSerializable(DeleteMessage);
msDeclPtr(DeleteMessage);


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
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};
msSerializable(FenceMessage);
msDeclPtr(FenceMessage);


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
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;

public:
    std::string text;
    Type type = Type::Normal;
};
msSerializable(TextMessage);
msDeclPtr(TextMessage);


class ScreenshotMessage : public Message
{
using super = Message;
public:

    // non-serializable fields
    std::atomic_bool ready{ false };

public:
    ScreenshotMessage();
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};
msSerializable(ScreenshotMessage);
msDeclPtr(ScreenshotMessage);


class ResponseMessage : public Message
{
    using super = Message;
public:
    std::vector<std::string> text;

    ResponseMessage();
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};
msSerializable(ResponseMessage);
msDeclPtr(ResponseMessage);


class QueryMessage : public Message
{
using super = Message;
public:
    enum class QueryType
    {
        Unknown,
        PluginVersion,
        ProtocolVersion,
        HostName,
        RootNodes,
        AllNodes,
    };

public:
    QueryType query_type = QueryType::Unknown;

    // non-serializable fields
    std::atomic_bool ready{ false };
    ResponseMessagePtr response;

    QueryMessage();
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};
msSerializable(QueryMessage);
msDeclPtr(QueryMessage);


class PollMessage : public Message
{
using super = Message;
public:
    enum class PollType
    {
        Unknown,
        SceneUpdate,
    };
    PollType poll_type = PollType::Unknown;

    // non-serializable fields
    std::atomic_bool ready{ false };

    PollMessage();
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};
msSerializable(PollMessage);
msDeclPtr(PollMessage);

} // namespace ms
