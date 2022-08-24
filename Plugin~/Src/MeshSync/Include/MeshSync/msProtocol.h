#pragma once

#include <atomic>

#include "MeshUtils/muMisc.h" //mu::nanosec

#include "MeshSync/MeshSync.h"
#include "MeshSync/msConfig.h" //msProtocolVersion
#include "MeshSync/NetworkData/msGetFlags.h"
#include "MeshSync/SceneGraph/msIdentifier.h" //InvalidID
#include "MeshSync/SceneGraph/msMeshRefineSettings.h"
#include "MeshSync/SceneGraph/msSceneSettings.h"
#include "MeshSync/SceneGraph/msPropertyInfo.h"
#include "MeshSync/SceneGraph/msEntity.h"

msDeclClassPtr(ResponseMessage)
msDeclClassPtr(Scene)
msDeclClassPtr(Entity)

namespace ms {

    const std::string REQUEST_SYNC = "sync";

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
        RequestServerLiveEdit,
        EditorCommand
    };
    int protocol_version = msProtocolVersion;
    int session_id = InvalidID;
    int message_id = 0;
    mu::nanosec timestamp_send = 0;

    // non-serializable fields
    mu::nanosec timestamp_recv = 0;

    virtual ~Message();
    virtual void serialize(std::ostream& os) const;
    virtual void deserialize(std::istream& is); // throw
};
msSerializable(Message);

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


class SetMessage : public Message
{
using super = Message;
public:
    ScenePtr scene;

public:
    SetMessage();
    explicit SetMessage(ScenePtr scene);
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};
msSerializable(SetMessage);


class DeleteMessage : public Message
{
using super = Message;
public:
    std::vector<Identifier> entities;
    std::vector<Identifier> materials;
    std::vector<Identifier> instances;

    DeleteMessage();
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};
msSerializable(DeleteMessage);


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

/// <summary>
/// A mechanism to initiate the sending of data from server to client.
/// Message that does not time out and is only replied to by the server
/// to send something back to the client.
/// </summary>
class ServerLiveEditRequest : public Message
{
    using super = Message;
public:
    SceneSettings scene_settings;

    // non-serializable fields
    std::atomic_bool ready{ false };
    std::atomic_bool cancelled { false };

public:
    ServerLiveEditRequest();
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};
msSerializable(ServerLiveEditRequest);

class ServerLiveEditResponse {
public:
    std::vector<PropertyInfo> properties;
    std::vector<EntityPtr> entities;
    std::string message;

    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};
msSerializable(ServerLiveEditResponse);

/// <summary>
/// Commands to be executed on the Unity Editor.
/// </summary>
class EditorCommandMessage : public Message
{
    using super = Message;
public:
    enum class CommandType {
        Unknown,
        AddServerToScene,
        GetProjectPath
    };
    CommandType command_type = CommandType::Unknown;

    std::atomic_bool ready{ false };
    const char* reply;

    EditorCommandMessage();
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
};
msSerializable(EditorCommandMessage);

} // namespace ms
