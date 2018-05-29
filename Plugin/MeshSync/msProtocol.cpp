#include "pch.h"
#include "msProtocol.h"
#include "msSceneGraphImpl.h"

namespace ms {

Message::~Message()
{
}
uint32_t Message::getSerializeSize() const
{
    return ssize(msProtocolVersion);
}
void Message::serialize(std::ostream& os) const
{
    write(os, msProtocolVersion);
}
bool Message::deserialize(std::istream& is)
{
    int protocol_version = 0;
    read(is, protocol_version);
    if (protocol_version != msProtocolVersion)
        return false;
    return true;
}

GetMessage::GetMessage()
{
}
uint32_t GetMessage::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    ret += ssize(flags);
    ret += ssize(scene_settings);
    ret += ssize(refine_settings);
    return ret;
}
void GetMessage::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, flags);
    write(os, scene_settings);
    write(os, refine_settings);
}
bool GetMessage::deserialize(std::istream& is)
{
    if (!super::deserialize(is)) { return false; }
    read(is, flags);
    read(is, scene_settings);
    read(is, refine_settings);
    return true;
}


SetMessage::SetMessage()
{
}
uint32_t SetMessage::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    ret += scene.getSerializeSize();
    return ret;
}
void SetMessage::serialize(std::ostream& os) const
{
    super::serialize(os);
    scene.serialize(os);
}
bool SetMessage::deserialize(std::istream& is)
{
    if (!super::deserialize(is)) { return false; }
    scene.deserialize(is);
    return true;
}


uint32_t DeleteMessage::Identifier::getSerializeSize() const
{
    return ssize(path) + ssize(id);
}
void DeleteMessage::Identifier::serialize(std::ostream& os) const
{
    write(os, path); write(os, id);
}
void DeleteMessage::Identifier::deserialize(std::istream& is)
{
    read(is, path); read(is, id);
}

DeleteMessage::DeleteMessage()
{
}
uint32_t DeleteMessage::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    ret += ssize(targets);
    return ret;
}
void DeleteMessage::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, targets);
}
bool DeleteMessage::deserialize(std::istream& is)
{
    if (!super::deserialize(is)) { return false; }
    read(is, targets);
    return true;
}


FenceMessage::~FenceMessage() {}
uint32_t FenceMessage::getSerializeSize() const
{
    return super::getSerializeSize()
        + ssize(type);
}
void FenceMessage::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, type);
}
bool FenceMessage::deserialize(std::istream& is)
{
    if (!super::deserialize(is)) { return false; }
    read(is, type);
    return true;
}

TextMessage::~TextMessage() {}
uint32_t TextMessage::getSerializeSize() const
{
    return super::getSerializeSize()
        + ssize(text)
        + ssize(type);
}
void TextMessage::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, text);
    write(os, type);
}
bool TextMessage::deserialize(std::istream& is)
{
    if (!super::deserialize(is)) { return false; }
    read(is, text);
    read(is, type);
    return true;
}


ScreenshotMessage::ScreenshotMessage() {}
uint32_t ScreenshotMessage::getSerializeSize() const { return super::getSerializeSize(); }
void ScreenshotMessage::serialize(std::ostream& os) const { super::serialize(os); }
bool ScreenshotMessage::deserialize(std::istream& is) { return super::deserialize(is); }

} // namespace ms
