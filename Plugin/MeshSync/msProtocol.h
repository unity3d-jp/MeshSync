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
        Query,
        Response,
    };

    virtual ~Message();
    virtual uint32_t getSerializeSize() const;
    virtual void serialize(std::ostream& os) const;
    virtual bool deserialize(std::istream& is);
};
msHasSerializer(Message);
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
msHasSerializer(GetMessage);
using GetMessagePtr = std::shared_ptr<GetMessage>;


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
msHasSerializer(SetMessage);
using SetMessagePtr = std::shared_ptr<SetMessage>;


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
msHasSerializer(DeleteMessage::Identifier);
msHasSerializer(DeleteMessage);
using DeleteMessagePtr = std::shared_ptr<DeleteMessage>;


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
msHasSerializer(FenceMessage);
using FenceMessagePtr = std::shared_ptr<FenceMessage>;


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
msHasSerializer(TextMessage);
using TextMessagePtr = std::shared_ptr<TextMessage>;


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
msHasSerializer(ScreenshotMessage);
using ScreenshotMessagePtr = std::shared_ptr<ScreenshotMessage>;


class QueryMessage : public Message
{
using super = Message;
public:
    enum class QueryType
    {
        Unknown,
        ClientName,
        RootNodes,
        AllNodes,
    };

public:
    QueryType type = QueryType::Unknown;

    std::shared_ptr<std::atomic_int> wait_flag; // non-serializable
    MessagePtr response;                        // 

    QueryMessage();
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    bool deserialize(std::istream& is) override;
};
msHasSerializer(QueryMessage);
using QueryMessagePtr = std::shared_ptr<QueryMessage>;


class ResponseMessage : public Message
{
using super = Message;
public:
    std::vector<std::string> text;

    ResponseMessage();
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    bool deserialize(std::istream& is) override;
};
msHasSerializer(ResponseMessage);
using ResponseMessagePtr = std::shared_ptr<ResponseMessage>;

} // namespace ms
